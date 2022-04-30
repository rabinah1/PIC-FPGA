String msg;
float sensor_pin_out = 0.0;
int reading = 0;
float temperature = 0.0;
int sensor_pin = A0;

void setup() {
    Serial.begin(9600);
    analogReference(EXTERNAL);
}

void loop() {
    if (Serial.available()) {
        while (Serial.available() > 0) {
            msg = Serial.readString();
        }
        int idx = msg.indexOf(" ");
        String cmd;
        int length;
        if (idx == -1) {
            cmd = msg;
        } else {
            cmd = msg.substring(0, idx);
            length = msg.length();
        }
        if (cmd == "read_temperature") {
            reading = analogRead(sensor_pin);
            sensor_pin_out = reading * (5.0 / 1024.0);
            temperature = (sensor_pin_out - 0.5) * 100;
            Serial.print("Temperature is ");
            Serial.print(temperature);
            Serial.print("\n");
        } else if (cmd == "echo") {
            Serial.print("I received a message ");
            Serial.print(msg.substring(idx, length));
            Serial.print("\n");
        } else {
            Serial.print("I received an invalid message: ");
            Serial.print(msg);
            Serial.print("\n");
        }
    }
}
