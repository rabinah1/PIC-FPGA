String msg;

void setup() {
    Serial.begin(9600);
}

void loop() {
    if (Serial.available()) {
        while (Serial.available() > 0) {
            msg = Serial.readString();
        }
        Serial.print("Hello, I am Arduino Nano, and I received this from Raspberry Pi: ");
        Serial.print(msg);
        Serial.print("\n");
    }
}
