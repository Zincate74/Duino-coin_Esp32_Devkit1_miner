/*
 * ESP32HashCore v1.0
 * Optimized Duino-Coin Miner for ESP32
 * 
 * Features:
 * - mbedTLS SHA1 acceleration
 * - Automatic pool discovery
 * - Persistent reconnect handling
 * - Lightweight memory usage
 * 
 * Developer: Zincate74
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <mbedtls/md.h> // Updated header


// ==================== CONFIGURATION ====================
const char* SSID = "your wifi ssid";
const char* PASSWORD = "your wifi password";
const char* DUCO_USER = "your DUCO username";
const char* RIG_IDENTIFIER = "ESP32_Lightweight";
// =======================================================

WiFiClient client;
String pool_ip = "";
int pool_port = 0;

// Helper to convert hexadecimal character tokens into raw bytes
uint8_t hex_to_byte(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

// Converts a 40-character target hex string into a 20-byte binary array
void hex_to_bytes(const char* hex, uint8_t* bytes) {
    for (int i = 0; i < 20; i++) {
        bytes[i] = (hex_to_byte(hex[i * 2]) << 4) | hex_to_byte(hex[i * 2 + 1]);
    }
}

// Connects to the Duino-Coin API to fetch the absolute best current pool node
bool fetchPoolAddress() {
    HTTPClient http;
    http.begin("https://server.duinocoin.com/getPool");
    int httpCode = http.GET();
    
    if (httpCode == 200) {
        String payload = http.getString();
        int ipIdx = payload.indexOf("\"ip\":\"");
        int portIdx = payload.indexOf("\"port\":");
        
        if (ipIdx != -1 && portIdx != -1) {
            int ipStart = ipIdx + 6;
            int ipEnd = payload.indexOf("\"", ipStart);
            pool_ip = payload.substring(ipStart, ipEnd);
            
            int portStart = portIdx + 7;
            int portEnd = payload.indexOf(",", portStart);
            if (portEnd == -1) portEnd = payload.indexOf("}", portStart);
            pool_port = payload.substring(portStart, portEnd).toInt();
            
            Serial.print("Fetched Pool -> IP: ");
            Serial.print(pool_ip);
            Serial.print(", Port: ");
            Serial.println(pool_port);
            return true;
        }
    }
    
    // Fallback constants if the main API coordinator experiences downtime
    pool_ip = "server.duinocoin.com";
    pool_port = 2811;
    Serial.println("API unreached; deploying default pool fallback.");
    return false;
}

// Persistent loop attempting connection to the TCP Socket mining pool server
void connectToPool() {
    while (!client.connected()) {
        fetchPoolAddress();
        Serial.print("Connecting to Duino-Coin mining pool node...");
        if (client.connect(pool_ip.c_str(), pool_port)) {
            Serial.println(" Connected!");
            String serverVersion = client.readStringUntil('\n');
            Serial.println("Server version: " + serverVersion);
        } else {
            Serial.println(" Connection failed. Retrying in 5 seconds...");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    
    // Establish Wi-Fi Connection
    Serial.print("Connecting to Wi-Fi");
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi Connected successfully!");
    Serial.print("Local IP Address: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    // Assert ongoing system network health
    if (WiFi.status() != WL_CONNECTED) {
        WiFi.disconnect();
        WiFi.begin(SSID, PASSWORD);
        while (WiFi.status() != WL_CONNECTED) delay(500);
    }
    
    if (!client.connected()) {
        connectToPool();
    }

    // 1. Send Job Request via the server protocol template
    client.print("JOB," + String(DUCO_USER) + ",ESP32\n");
    
    // 2. Await and extract the raw job payload string string
    String job = client.readStringUntil('\n');
    job.trim();
    
    int firstComma = job.indexOf(',');
    int secondComma = job.indexOf(',', firstComma + 1);
    if (firstComma == -1 || secondComma == -1) {
        Serial.println("Error parsing pool job signature. Reconnecting...");
        client.stop();
        delay(2000);
        return;
    }
    
    String last_block_hash = job.substring(0, firstComma);
    String expected_hash = job.substring(firstComma + 1, secondComma);
    unsigned long difficulty = job.substring(secondComma + 1).toInt();
    
    Serial.printf("\nJob Assigned. Difficulty: %lu\n", difficulty);

    // 3. Set up match targets using raw stack buffers instead of slow dynamic Heap objects
    uint8_t target_binary[20];
    hex_to_bytes(expected_hash.c_str(), target_binary);
    
    char buffer[120];
    strcpy(buffer, last_block_hash.c_str());
    int base_len = strlen(buffer);
    
    uint8_t digest[20];
    unsigned long start_time = micros();
    unsigned long nonce = 0;
    bool found = false;
    
    // Set up mbedtls MD context for the newer ESP32 cores (v3.x / ESP-IDF v5)
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA1), 0);
    
    // 4. Main high-performance mining block
    unsigned long max_nonce = difficulty * 100;
    for (nonce = 0; nonce <= max_nonce; nonce++) {
        // Appends the current nonce directly to the base hash string array memory zone
        itoa(nonce, buffer + base_len, 10);
        int total_len = base_len + strlen(buffer + base_len);
        
        // Push payload to crypto hardware
        mbedtls_md_starts(&ctx);
        mbedtls_md_update(&ctx, (const unsigned char *)buffer, total_len);
        mbedtls_md_finish(&ctx, digest);
        
        // Fast hardware binary block check
        if (memcmp(digest, target_binary, 20) == 0) {
            found = true;
            break;
        }
    }
    
    mbedtls_md_free(&ctx); // Free memory to prevent leaks
    
    unsigned long elapsed_time = micros() - start_time;
    if (elapsed_time == 0) elapsed_time = 1; // Safeguard against division by zero
    float hashrate = (nonce / (elapsed_time / 1000000.0));

    // 5. Package results and dispatch back to Pool Node
    if (found) {
        Serial.printf("Share Found! Hashrate: %.2f kH/s\n", (hashrate / 1000.0));
        
        // Format layout rule: nonce,hashrate,miner_banner,rig_identifier
        client.print(String(nonce) + "," + String(hashrate, 2) + ",Lightweight ESP32 Miner," + RIG_IDENTIFIER + "\n");
        
        String feedback = client.readStringUntil('\n');
        feedback.trim();
        Serial.println("Pool Status: " + feedback);
    } else {
        Serial.println("Nonce searching signature range exhausted without match.");
    }
}