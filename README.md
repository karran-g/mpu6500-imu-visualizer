# MPU-6500 orientation visualizer

3D orientation visualizer for the MPU-6500 IMU. ESP32-C3 reads raw
registers over I2C, runs a complementary filter on-device, and streams Euler
angles to a Python/OpenGL renderer. No sensor libraries, register-level access
only.

## Hardware
- ESP32-C3-DevKitM-1
- MPU-6500 (AD0 → GND, address 0x68)
- I2C on GPIO4 (SDA) / GPIO5 (SCL)

## Status
- [x] Register-level I2C primitives (read, burst read, write with status)
- [x] WHO_AM_I verified, device reset sequence
- [x] Sensor config: ±250 °/s, ±2g, 41 Hz DLPF both sensors, 100 Hz ODR
- [x] Config write-back verification per register
- [x] Noise/bias characterization harness (min/max/mean per axis)
- [ ] Gyro bias calibration at boot
- [ ] Complementary filter
- [ ] Serial output + Python renderer

## Configuration
| Register | Value | Effect |
|---|---|---|
| GYRO_CONFIG (0x1B) | 0x00 | ±250 °/s, DLPF enabled |
| ACCEL_CONFIG (0x1C) | 0x00 | ±2g |
| CONFIG (0x1A) | 0x03 | Gyro 41 Hz BW, 1 kHz internal rate |
| ACCEL_CONFIG2 (0x1D) | 0x03 | Accel 41 Hz BW |
| SMPLRT_DIV (0x19) | 0x09 | 100 Hz output rate |
