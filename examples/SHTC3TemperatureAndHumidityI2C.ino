#include <shtc3.h>

void setup() {
    Serial.begin(9600U);
    shtc3_initialize();
}



void loop() {
    float temperature = 0.0F;
    float humidity = 0.0F;

    int ret = shtc3_get_temperature_and_humidity(&temperature, &humidity);

    if(ret == SHTC3_RET_OK) {
        Serial.print("Temperature: ");
        Serial.println(temperature);
        Serial.print("Humidity: ");
        Serial.println(humidity);
    }
    else {
        Serial.println("Error! Invalid return value, from SHTC3 sensor...");
    }

    delay(1000U);
}
