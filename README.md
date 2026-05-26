# Duino-coin_Esp32_Devkit1_miner
High-performance lightweight Duino-Coin miner optimized for ESP32 using mbedTLS SHA1 acceleration.Only runs on single core.

# Duino-coin_Esp32_Devkit1_miner

High-performance lightweight Duino-Coin miner optimized for ESP32 using mbedTLS SHA1 acceleration.

---

## What The Project Does

This project is an optimized Duino-Coin mining client designed for ESP32 microcontrollers. It connects to mining pool servers, receives jobs, performs SHA1 hashing, and submits valid shares using lightweight and efficient embedded code.

---

## Features

- Optimized SHA1 hashing using mbedTLS
- Lightweight memory usage
- Automatic mining pool discovery
- Persistent reconnect handling
- Wi-Fi support for ESP32
- Real-time hashrate reporting
- Binary digest comparison optimization
- Stable single-core mining implementation

---

## Performance

| Device | Hashrate |
|---|---|
| ESP32 DevKit V1 | ~40 kH/s |

Current implementation runs on a single ESP32 core with optimized SHA1 computation.

---

## Future Plans

- Dual-core mining support
- ESP32-S3 optimization
- OLED statistics display
- Web dashboard
- OTA firmware updates
- Arduino Nano implementation
- Hardware acceleration improvements

---

## Repository Structure

```text
ESP32/
 ├── ESP32HashCore_v1.ino
 ├── hardware.md
 ├── optimization_notes.md
 └── performance.md

README.md
ABOUT.md
LICENSE
CHANGELOG.md
.gitignore
```

---

## About The Developer

**Zincate74**

Student embedded systems developer interested in:
- ESP32 optimization
- IoT systems
- embedded networking
- hardware acceleration
- microcontroller performance tuning

---

## Disclaimer

This project is for educational and experimental purposes only.