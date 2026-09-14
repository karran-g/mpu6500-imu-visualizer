
//Function to to collect and store the min, max and sum(to compute avg) stats from a raw list for each axis over n iterations.
void collectStats(uint8_t address, AxisStats *sample, uint16_t n) {
    int16_t raw[6];
    int attempts = 0;  //For failure case when I2C does not respond within 400 attempts
    for (uint16_t i = 0; i < n; /*Iteration handled only on a successful burst read.*/) {
        if (readMotion(address,raw)) {
            for (int j = 0; j < 6; j++) {
                if (raw[j] < sample[j].min) sample[j].min = raw[j];
                if (raw[j] > sample[j].max) sample[j].max = raw[j];
                sample[j].sum += raw[j];
                sample[j].count += 1;
            }
            i++;
        }
        attempts++;
        if (attempts > n * 2) break;  //Dead I2c Failsafe.
        delay(10);                    // Sampling cannot be faster than ODR.
    }
}

bool readMotion(uint8_t address, int16_t *raw) {
    uint8_t buffer[14];
    if (!readRegBurst(address, 0x3B, buffer, sizeof(buffer))) return false;
    raw[0] = ((int16_t)buffer[0] << 8 | buffer[1]);
    raw[1] = ((int16_t)buffer[2] << 8 | buffer[3]);
    raw[2] = ((int16_t)buffer[4] << 8 | buffer[5]);
    raw[3] = ((int16_t)buffer[8] << 8 | buffer[9]);
    raw[4] = ((int16_t)buffer[10] << 8 | buffer[11]);
    raw[5] = ((int16_t)buffer[12] << 8 | buffer[13]);
    return true;
}

void config(uint8_t address, configReg *r, uint8_t num) {
    for (int i = 0; i < num; i++) {
        r[i].status = writeRegByte(address, r[i].reg, r[i].val);
    }
}
bool verifyConfig(uint8_t address, configReg *r, uint8_t num) {
    for (int i = 0; i < num; i++) {
        if (r[i].status == 0) r[i].valCheck = (readReg(address, r[i].reg) == r[i].val);
    }
    //Reason for doing it in second loop so that the entire configRegisters table is updated and can be checked to see where the issues lie.
    for (int i = 0; i < num; i++) {
        if (r[i].valCheck == false) return false;
    }
    return true;
}
