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
        if (msg.indexOf("read_temperature") != -1) {
            reading = analogRead(sensor_pin);
            sensor_pin_out = reading * (5.0 / 1024.0);
            temperature = (sensor_pin_out - 0.5) * 100;
            Serial.print("Temperature is ");
            Serial.print(temperature);
            Serial.print("\n");
        } else {
            Serial.print("I received an invalid message: ");
            Serial.print(msg);
            Serial.print("\n");
        }
    }
}
