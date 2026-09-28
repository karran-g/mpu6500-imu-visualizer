//Function to write data to a register. 
byte writeRegByte(uint8_t address, uint8_t reg, uint8_t val) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(val);
    return Wire.endTransmission();
}

//Function to read a register (Single byte read).
bool readReg(uint8_t address, uint8_t reg, uint8_t *out) {
    return readRegBurst(address,reg,out,1); // better since i only need to change readRegBurst in AVR devices and error cases.
}
//Function to read a burst of n = len  bytes from a register and store the data into a buffer list.
bool readRegBurst(uint8_t address, uint8_t reg, uint8_t *buffer, uint8_t len) {
    /*
    esp32 core : endTransmission(false) only queues the register byte and always retruns 0.
    The address r+w happen both in requestFrom(). so it's byte count is the only thing you can check.
    No way to find a difference between NACK and short reads (In AVR boards like arduino it is possible to do so, I think.)
    */
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.endTransmission(false);
    uint8_t lenCheck = Wire.requestFrom(address, len);
    // So you can reject short reads before Wire.read() [it returns 0xFF / -1 on missing byte].
    if (lenCheck != len) return false; 
    for (int i = 0; i < len; i++) {
        buffer[i] = Wire.read();
    }
    return true;
}
// i2c debugging functions for wire and address not found problems(written by Claude)
// Scan the bus and print every address that ACKs. Returns the number found.
uint8_t i2cScan() {
    uint8_t found = 0;
    for (uint8_t a = 0x08; a <= 0x77; a++) {
        Wire.beginTransmission(a);
        if (Wire.endTransmission() == 0) {   // with STOP: really transmits, returns ACK status
            Serial.printf("  ACK at 0x%02X\n", a);
            found++;
        }
    }
    Serial.printf("Scan done, %u device(s)\n", found);
    return found;
}

// Wiring test: read a register with a known value forever and count results.
// Wiggle one wire at a time; the one that makes fail/wrong climb is the culprit.
// Never returns.
void i2cHealthCheck(uint8_t address, uint8_t reg, uint8_t expected) {
    uint32_t ok = 0, fail = 0, wrong = 0;
    while (true) {
        uint8_t v;
        if (!readReg(address, reg, &v)) fail++;
        else if (v != expected)         wrong++;
        else                            ok++;
        if ((ok + fail + wrong) % 500 == 0)
            Serial.printf("ok %lu  fail %lu  wrong %lu\n",
                          (unsigned long)ok, (unsigned long)fail, (unsigned long)wrong);
    }
}