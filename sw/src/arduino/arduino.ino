String msg;
String cmd;
int sensor_pin = A0;
int reading = 0;
int space_idx = 0;
int length = 0;
float reading_scaled = 0.0;
float temperature = 0.0;

void setup() {
    Serial.begin(9600);
    analogReference(EXTERNAL);
}

void loop() {
    if (Serial.available()) {
        while (Serial.available() > 0)
            msg = Serial.readString();
        space_idx = msg.indexOf(" ");
        if (space_idx == -1) {
            cmd = msg;
        } else {
            cmd = msg.substring(0, space_idx);
            length = msg.length();
        }
        if (cmd == "read_temperature") {
            reading = analogRead(sensor_pin);
            reading_scaled = reading * 5.0 / 1024.0;
            temperature = (reading_scaled - 0.5) * 100;
            Serial.print("Temperature is ");
            Serial.print(temperature);
        } else if (cmd == "echo") {
            Serial.print("I received a message");
            Serial.print(msg.substring(space_idx, length));
        } else {
            Serial.print("I received an invalid message: ");
            Serial.print(msg);
        }
        Serial.print("\n");
    }
}
