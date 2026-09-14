//Function to write data to a register.
byte writeRegByte(uint8_t address, uint8_t reg, uint8_t val) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(val);
    return Wire.endTransmission();
}

//Function to read a register (Single read).
uint8_t readReg(uint8_t address, uint8_t reg) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.endTransmission(false);   // false = repeated START, keeps the bus
    Wire.requestFrom(address, 1);  // returns how many bytes actually arrived
    uint8_t id = Wire.available() ? Wire.read() : 0xFF;
    return id;
}
//Function to read a burst of n = len  bytes from a register and store the data into a buffer list.
bool readRegBurst(uint8_t address, uint8_t reg, uint8_t *buffer, uint8_t len) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.endTransmission(false);
    uint8_t lenCheck = Wire.requestFrom(address, len);
    if (lenCheck != len) return false;
    for (int i = 0; i < len; i++) {
        buffer[i] = Wire.read();
    }
    return true;
}
