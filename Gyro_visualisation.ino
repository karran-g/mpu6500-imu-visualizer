#include <Wire.h>

//Declaring the IMU address
constexpr uint8_t add = 0x68;  //Guaranteed to be initialized at compile time.

//Declaring scl and sda pins
byte sda = 4;
byte scl = 5;

//Structs
struct AxisStats {
    int16_t min = INT16_MAX;
    int16_t max = INT16_MIN;
    int32_t sum = 0;
    int count = 0;
};
struct configReg {
    uint8_t reg;
    uint8_t val;
    byte status;
    bool valCheck;
};

configReg configRegisters[5] = {
    { 0x1B, 0x00, 0xFF, false },
    { 0x1C, 0x00, 0xFF, false },
    { 0x1A, 0x03, 0xFF, false },
    { 0x1D, 0x03, 0xFF, false },
    { 0x19, 0x09, 0xFF, false },
};

constexpr uint8_t NUM_CONFIGS = sizeof(configRegisters) / sizeof(configRegisters[0]);

void setup() {
    //Start serial; baud rate = 115,200
    Serial.begin(115200);
    Serial.println("boot");
    //Start I2C communication with clock speed = 100000 (100kHz)
    Wire.begin(sda, scl);
    Wire.setClock(100000);

    //Read WHO_AM_I register. (Has to be 0x70)
    uint8_t check = readReg(add, 0x75);
    if (check == 0x70) Serial.printf("MPU WHO_AM_I = 0x%02X\n", check);
    else {
        Serial.printf("WHO AM I Check failed, MPU WHO_AM_I = 0x%02X\n", check);
        while (true) delay(1000);
    }
    
    //Reset routine
    byte reset = writeRegByte(add, 0x6B, 0x80);  //Write 1 to DEVICE_RESET in PWR_MGMT_1.
    if (reset != 0) {
        Serial.printf("Device not reset, Error type = %u\n", reset);
        while (true) delay(1000);
    }
    delay(100);
    byte clkStatus = writeRegByte(add, 0x6B, 0x01);  //Write 1 to CLKSEL in PWR_MGMT_1, selects the auto-select PLL source, else 20MHz RC clock.
    if (clkStatus != 0) {
        Serial.printf("Clock not chosen, Error type = %u\n", clkStatus);
        while (true) delay(1000);
    }
    delay(100);

    //Sample Run
    AxisStats before[6], after[6];

    //200 samples Before
    collectStats(add, before, 200);
    int numAxisStats = sizeof(before) / sizeof(before[0]);
    printAxisStats("Before", before, numAxisStats);

    //config
    config(add, configRegisters, NUM_CONFIGS);
    bool configCheck = verifyConfig(add, configRegisters, NUM_CONFIGS);
    printConfigRegisters(configRegisters, NUM_CONFIGS);
    if (!configCheck) {
        Serial.println("The config registers were not written properly !!");
        while (true) delay(1000);
    }

    //200 samples After
    collectStats(add, after, 200);
    printAxisStats("After", after, numAxisStats);
}

void loop() {
}