// --- Print Functions ---

// 1. Function to print AxisStats arrays (handles both 'before' and 'after')
void printAxisStats(const char* listName, AxisStats stats[], int size) {
    Serial.print("=== AxisStats: ");
    Serial.print(listName);
    Serial.println(" ===");

    for (int i = 0; i < 3; i++) {
        Serial.print("Index [");
        Serial.print(i);
        Serial.print("] ");
        Serial.print("Min: ");
        Serial.print(stats[i].min / 16384.0f);
        Serial.print(" | Max: ");
        Serial.print(stats[i].max / 16384.0f);
        Serial.print(" | Avg: ");
        Serial.print(stats[i].sum / (float)stats[i].count / 16384.0f);
        Serial.println();
    }
    Serial.println();

    for (int i = 3; i < 6; i++) {
        Serial.print("Index [");
        Serial.print(i);
        Serial.print("] ");
        Serial.print("Min: ");
        Serial.print(stats[i].min / 131.0f);
        Serial.print(" | Max: ");
        Serial.print(stats[i].max / 131.0f);
        Serial.print(" | Avg: ");
        Serial.print(stats[i].sum / (float)stats[i].count / 131.0f);
        Serial.println();
    }
    Serial.println();
}
// 2. Function to print configReg arrays
void printConfigRegisters(configReg configs[], int size) {
    Serial.println("=== Config Registers ===");

    for (int i = 0; i < size; i++) {
        Serial.print("Index [");
        Serial.print(i);
        Serial.print("] ");

        // Print as HEX for easier reading (e.g., 0x1B)
        Serial.print("Reg: 0x");
        Serial.print(configs[i].reg, HEX);
        Serial.print(" | Val: 0x");
        Serial.print(configs[i].val, HEX);

        Serial.print(" | Status: ");
        Serial.print(configs[i].status);
        Serial.print(" | valCheck: ");
        Serial.println(configs[i].valCheck ? "true" : "false");
    }
    Serial.println();
}