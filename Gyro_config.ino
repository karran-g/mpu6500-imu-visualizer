//Function to to collect and store the min, max and sum(to compute avg) stats from a raw list for each axis over n iterations.
bool collectStats(uint8_t address, AxisStats *sample, uint16_t n) {
    int16_t raw[6];
    uint16_t i = 0;
    int attempts = 0;  //For failure case when I2C does not respond within 2n attempts
    while (i < n) {
        if (readMotion(address, raw)) {
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
        delay(10);                    // Sampling cannot be faster than ODR = 100hz .
    }
    return (i == n);
}
//Function to fill a raw list from the Burst read buffer.
bool readMotion(uint8_t address, int16_t *raw) {
    uint8_t buffer[14];
    if (!readRegBurst(address, 0x3B, buffer, sizeof(buffer))) return false;
    raw[0] = ((int16_t)buffer[0] << 8 | buffer[1]);    //ax
    raw[1] = ((int16_t)buffer[2] << 8 | buffer[3]);    //ay
    raw[2] = ((int16_t)buffer[4] << 8 | buffer[5]);    //az
    raw[3] = ((int16_t)buffer[8] << 8 | buffer[9]);    //gx
    raw[4] = ((int16_t)buffer[10] << 8 | buffer[11]);  //gy
    raw[5] = ((int16_t)buffer[12] << 8 | buffer[13]);  //gz
    return true;
}
// Function to write the config values to their registers and store the ACK status for each reg
void config(uint8_t address, configReg *r, uint8_t num) {
    for (int i = 0; i < num; i++) {
        r[i].status = writeRegByte(address, r[i].reg, r[i].val);
    }
}
// Function to chekc if config values were written properly and updating their valCheck 
bool verifyConfig(uint8_t address, configReg *r, uint8_t num) {
    for (int i = 0; i < num; i++) {
        r[i].valCheck = false; //already initliased in configRegisters in global but I kept it incase I have to call this function later on.
        if (r[i].status == 0) {
            uint8_t v;
            bool checkVal = readReg(address, r[i].reg, &v);
            if (checkVal) r[i].valCheck = v == r[i].val;
        }
    }
    //Reason for doing it in second loop so that the entire configRegisters table is updated and can be printed to see where the issues lie.
    for (int i = 0; i < num; i++) {
        if (r[i].valCheck == false) return false;
    }
    return true;
}
// Computes 2 lists containing mean values of n samples, used for gyro bias calc.
bool computeMeans(uint8_t address, float *out1, float *out2, uint16_t n) {
    AxisStats sample1[6], sample2[6];
    bool sample1Check = collectStats(address, sample1, n);
    bool sample2Check = collectStats(address, sample2, n);
    if (sample1Check && sample2Check) {
        for (int i = 3; i < 6; i++) {
            out1[i - 3] = sample1[i].sum / (float)sample1[i].count / 131.0f;
            out2[i - 3] = sample2[i].sum / (float)sample2[i].count / 131.0f;
        }
        return true;
    }
    return false;
}
// Need to comment what it does!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
BiasResult checkBias(uint8_t address, float *bias, float *diff, uint16_t n) {
    float mean1[3], mean2[3];
    bool biasCheck = computeMeans(address, mean1, mean2, n);
    if (biasCheck) {
        for (int i = 0; i < 3; i++) {
            diff[i] = fabsf(mean1[i] - mean2[i]);
        }
        for (int i = 0; i < 3; i++) {
            if (!(diff[i] < 0.1f)) return BIAS_MOVED;  // Need to see if this can be better
        }
        for (int i = 0; i < 3; i++) {
            bias[i] = (mean1[i] + mean2[i]) / 2;
        }
        return BIAS_OK;
    }
    return BIAS_BUS_FAILED;
}
// Corrected raw values with their actual units
bool readCorrected(uint8_t address, float *out, float *bias) {
    int16_t raw[6];
    bool motion = readMotion(address, raw);
    if (motion) {
        out[0] = raw[0] / 16384.0f;          //ax
        out[1] = raw[1] / 16384.0f;          //ay
        out[2] = raw[2] / 16384.0f;          //az
        out[3] = raw[3] / 131.0f - bias[0];  //gx
        out[4] = raw[4] / 131.0f - bias[1];  //gy
        out[5] = raw[5] / 131.0f - bias[2];  //gz
        return true;
    }
    return false;
}
// Avg values after removing bias
bool avgSample(uint8_t address, float *bias, float *avg, uint16_t n) {
    float out[6];
    uint16_t i = 0;
    int attempts = 0;
    while (i < n) {
        if (readCorrected(address, out, bias)) {
            for (int j = 0; j < 6; j++) {
                avg[j] += (out[j] - avg[j]) / (i + 1);  //formula for avg with just one variable
            }
            i++;
        }
        attempts++;
        if (attempts > n * 2) break;  //Dead I2c Failsafe.
        delay(10);                    // Sampling cannot be faster than ODR.
    }

    return (i == n);
}
