# Optimization Notes

## Current Optimizations

### SHA1 Hardware Acceleration
Uses mbedTLS SHA1 functions optimized for ESP32 architecture.

### Lightweight Memory Usage
Uses stack buffers instead of heavy dynamic heap allocation.

### Binary Digest Comparison
Performs direct binary hash comparison for improved efficiency.

### Persistent Pool Reconnect
Automatically reconnects to mining pools during connection loss.

### Reduced Object Creation
Minimizes String allocations inside the mining loop.

---

## Current Limitations

- Single-core mining
- Wi-Fi latency overhead
- ASCII nonce conversion bottleneck

---

## Planned Improvements

- Dual-core mining support
- ESP32-S3 optimization
- FreeRTOS task scheduling
- Improved nonce generation
- OLED statistics display
- Hardware acceleration research