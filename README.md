# MPU-6500 orientation visualizer

3D orientation visualizer for the MPU-6500 IMU. ESP32-C3 reads raw
registers over I2C, runs a complementary filter on-device, and streams Euler
angles to a Python/OpenGL renderer. No sensor libraries, register-level access
only.

## Hardware
- ESP32-C3-DevKitM-1
- MPU-6500 (AD0 → GND, address 0x68)
- I2C on GPIO4 (SDA) / GPIO5 (SCL), 100 kHz

## Status
- [x] Register-level I2C primitives (read, burst read, write with status)
- [x] Short-read rejection on every read; failures reported, never passed on as data
- [x] WHO_AM_I verified, distinguishing no response from wrong chip ID
- [x] Device reset sequence
- [x] Sensor config: ±250 °/s, ±2g, 41 Hz DLPF both sensors, 100 Hz ODR
- [x] Config write-back verification per register
- [x] Noise/bias characterization harness (min/max/mean per axis)
- [x] Gyro bias calibration at boot, with stillness check
- [x] I2C wiring diagnostics (bus scan, read-reliability counter)
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

## Gyro bias calibration
At boot, the board must be still for ~2 s. The gyro is sampled in two
back-to-back runs of 100 samples. If any axis's two means differ by more than
0.1 °/s, the board moved and the run is rejected; up to 5 retries, then halt.
Otherwise the bias is the mean of both halves.

Measured on this unit: bias ≈ 2.8 / 4.7 / 0.9 °/s (X/Y/Z). After subtraction,
the gyro averages 0.00 ± 0.01 °/s at rest.

Why 200 samples is enough: gyro noise at rest is ~0.04 °/s standard deviation,
so a 200-sample mean is accurate to ~0.003 °/s — far below the ~0.1 °/s the
bias drifts after calibration (warm-up, temperature). Longer calibration only
lengthens boot.

## Hardware notes
- **Intermittent I2C errors** (NACKs, bus errors, corrupted samples passing
  the length check) were traced to worn breadboard contacts on the 3V3/GND
  rows. Moving to fresh rows fixed it. `i2cHealthCheck()` reads WHO_AM_I in a
  loop and counts failures — wiggle one wire at a time to find a bad contact.
- **I2C bus lockup:** resetting the ESP32 mid-read can leave the MPU holding
  SDA low. Re-uploading doesn't clear it (the MPU stays powered); a full USB
  power cycle does.