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

float gyroBias[3];  // List containing the bias for each gyro axis, computed from the 2 gyro bias functions

//Enum for different bias failure cases
enum BiasResult : int8_t { BIAS_OK,            //Everything is good
                           BIAS_MOVED,         //External noise flag caused by sudden movement or board not being stationary
                           BIAS_BUS_FAILED };  //Flag if bus itself is dead.

constexpr uint8_t NUM_CONFIGS = sizeof(configRegisters) / sizeof(configRegisters[0]);  //size of configReg list to be used later on in config functions

void setup() {
    //Start serial; baud rate = 115,200
    Serial.begin(115200);
    Serial.println("boot");
    //Start I2C communication with clock speed = 100000 (100kHz)
    Wire.begin(sda, scl);
    Wire.setClock(100000);
    //--------------------------------------------------------------------------------------------------------------------------------------
    // ---START TEMP ---
    // i2cScan();
    // i2cHealthCheck(add, 0x75, 0x70);   // never returns, WHO_AM_I = 0x70
    // --- TEMP: I2C address scan ---
    // --- END TEMP ---
    //--------------------------------------------------------------------------------------------------------------------------------------
    //Read WHO_AM_I register. (Has to be 0x70)
    uint8_t id;
    bool check = readReg(add, 0x75, &id);
    if (check) {
        if (id != 0x70) {
            Serial.printf("WHO AM I Check failed, MPU WHO_AM_I = 0x%02X\n", id);
            while (true) delay(1000);
        } else {
            Serial.printf("MPU WHO_AM_I = 0x%02X\n", id);
        }
    } else {
        Serial.printf("Bus dead or no respose at 0x68!\n");
        while (true) delay(1000);
    }
    //--------------------------------------------------------------------------------------------------------------------------------------
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
    //--------------------------------------------------------------------------------------------------------------------------------------
    //Sample Run
    AxisStats before[6], after[6];

    //200 samples Before
    bool beforeCheck = collectStats(add, before, 200);
    int numAxisStats = sizeof(before) / sizeof(before[0]);
    printAxisStats("Before", before, numAxisStats);
    if (!beforeCheck) {
        Serial.printf("Before run incomplete: %d/200 samples, I2C unreliable\n", before[0].count);
        while (true) delay(1000);
    }

    //config
    config(add, configRegisters, NUM_CONFIGS);
    bool configCheck = verifyConfig(add, configRegisters, NUM_CONFIGS);
    printConfigRegisters(configRegisters, NUM_CONFIGS);
    if (!configCheck) {
        Serial.println("The config registers were not written properly !!");
        while (true) delay(1000);
    }

    //200 samples After
    bool afterCheck = collectStats(add, after, 200);
    numAxisStats = sizeof(after) / sizeof(after[0]);
    printAxisStats("After", after, numAxisStats);
    if (!afterCheck) {
        Serial.printf("After run incomplete: %d/200 samples, I2C unreliable\n", after[0].count);
        while (true) delay(1000);
    }
    //--------------------------------------------------------------------------------------------------------------------------------------
    //Bias.
    float diff[3];  //Error handling for difference of means for bias calc.

    BiasResult flagBias = checkBias(add, gyroBias, diff, 100);  // compute and check bias over 2 x 100 samples
    uint8_t attempts = 0;
    while (flagBias == BIAS_MOVED && attempts < 5) {

        Serial.println("The board must be kept still for the biases to be computed!!");
        printBias("Diff", diff);
        delay(1500);
        flagBias = checkBias(add, gyroBias, diff, 100);
        attempts++;
    }
    if (flagBias == BIAS_MOVED) {
        Serial.println("Gyro noise still not stable!!");
        while (true) delay(1000);
    } else if (flagBias == BIAS_BUS_FAILED) {
        Serial.print("Count mismatch, bus may be ded!!\n");
        while (true) delay(1000);
    } else if (flagBias == BIAS_OK) printBias("Bias", gyroBias);

    //--------------------------------------------------------------------------------------------------------------------------------------

    //Sample run after removing Bias
    float avg[6] = {0.0,0.0,0.0,0.0,0.0,0.0};
    bool biasSample = avgSample(add, gyroBias, avg, 100);
    printBias("corrected", avg + 3);  //avg +3 is pointer to avg[3](pointer decay)
    if (!biasSample) {
        Serial.printf("Sample run incomplete : attempts exceeded 2*n!!\n");
        while (true) delay(1000);
    }
    //--------------------------------------------------------------------------------------------------------------------------------------

}

void loop() {
}