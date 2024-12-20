#include <SPI.h>
#include <Ethernet.h>


byte mac[] = { 0xD4, 0x28, 0xB2, 0xFF, 0xA0, 0xA1 };


char thingSpeakAddress[] = "api.thingspeak.com";
String writeAPIKey = "ZDAKS2KEQQH85IS6";
const int updateThingSpeakInterval = 16 * 1000;


long lastConnectionTime = 0;
boolean lastConnected = false;
int failedCounter = 0;

EthernetClient client;

// Sensor Pins
const int heartRatePin = A1;   // Heart rate sensor 
const int bloodPressurePin = A2; // Blood pressure sensor 
const int lampPin = 4;         // Lamp indicator
const int switchPin = 5;       // Switch 


const int lm35Pin = A2; // LM35 connected to analog pin A2

void setup() {
 
    Serial.begin(115200);

  
    startEthernet();

    pinMode(lampPin, OUTPUT);
    pinMode(switchPin, INPUT_PULLUP);  // Switch is connected to ground

    // Add a small delay for proper initialization
    delay(1000);
}

void loop() {
   
    float tempValue = analogRead(lm35Pin) * (5.0 / 1023.0) * 100; 
    String temperature = String(tempValue, 2); 


    int heartRate = analogRead(heartRatePin);
    int bloodPressure = analogRead(bloodPressurePin);

    bool switchState = digitalRead(switchPin) == LOW; 
    if (switchState) {
        digitalWrite(lampPin, HIGH); 
    } else {
        digitalWrite(lampPin, LOW);  
    }
    String lampState = switchState ? "1" : "0";  

    
    if (client.available()) {
        char c = client.read();
        Serial.print(c);
    }

    if (!client.connected() && lastConnected) {
        Serial.println("...disconnected");
        Serial.println();
        client.stop();
    }

    if (!client.connected() && (millis() - lastConnectionTime > updateThingSpeakInterval)) {
        updateThingSpeak("field1=" + String(heartRate) + 
                         "&field2=" + temperature + 
                         "&field3=" + lampState + 
                         "&field4=" + String(bloodPressure));
    }

    if (failedCounter > 3) {
        startEthernet();
    }

    lastConnected = client.connected();
}

void updateThingSpeak(String tsData) {
    if (client.connect(thingSpeakAddress, 80)) {
        client.print("POST /update HTTP/1.1\n");
        client.print("Host: api.thingspeak.com\n");
        client.print("Connection: close\n");
        client.print("X-THINGSPEAKAPIKEY: " + writeAPIKey + "\n");
        client.print("Content-Type: application/x-www-form-urlencoded\n");
        client.print("Content-Length: ");
        client.print(tsData.length());
        client.print("\n\n");

        client.print(tsData);
        lastConnectionTime = millis();
        if (client.connected()) {
            Serial.println("Connecting to ThingSpeak...");
            Serial.println();
            failedCounter = 0;
        } else {
            failedCounter++;
            Serial.println("Connection to ThingSpeak failed (" + String(failedCounter, DEC) + ")");
            Serial.println();
        }
    } else {
        failedCounter++;
        Serial.println("Connection to ThingSpeak Failed (" + String(failedCounter, DEC) + ")");
        Serial.println();
        lastConnectionTime = millis();
    }
}

void startEthernet() 
{
    client.stop();
    Serial.println("Connecting Arduino to network...");
    Serial.println();
    delay(1000);

    if (Ethernet.begin(mac) == 0) {
        Serial.println("DHCP Failed, reset Arduino to try again");
        Serial.println();
    } else {
        Serial.println("Arduino connected to network using DHCP");
        Serial.println();
    }

    delay(1000);
}

