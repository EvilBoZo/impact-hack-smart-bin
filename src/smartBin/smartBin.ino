#include <WiFi.h>
#include <HTTPClient.h>

// Задайте свои данные Wi-Fi
const char* ssid = "";
const char* password = "";

const char* boardHost = "http://10.15.2.62";
const char* getVideoUrl = "/docmd?cmd=103";
const char* getImagesUrl = "/docmd?cmd=104";
const char* prevImagesUrl = "/docmd?cmd=300";
const char* nextImagesUrl = "/docmd?cmd=301";

const int sensorThreshold = 80;

void httpGETRequest(const char* requestUrl) {
    HTTPClient http;
    http.begin(String(boardHost)+String(requestUrl));
    int httpCode = http.GET();

    if (httpCode <= 200 && httpCode >= 300) {
        Serial.printf("GET request failed, error: %s\n", http.errorToString(httpCode).c_str());;
    }

        // Check if the request was successful
    if (httpCode > 0) {
      Serial.printf("HTTP Response Code: %d\n", httpCode);
      String payload = http.getString();
      Serial.println(payload);
    } else {
      Serial.printf("Error in HTTP GET request: %s\n", http.errorToString(httpCode).c_str());
    }
    
    http.end();
}

int getRandomValue() {
    const int values[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    const int size = sizeof(values) / sizeof(values[0]);
    int idx = random(0, size); // random(min, max) возвращает [min, max)
    return values[idx];
}

int getNextValue() {
    static const int values[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    static const int size = sizeof(values) / sizeof(values[0]);
    static int index = 0;
  
    int result = values[index];
    index = (index + 1) % size; // переход к следующему, с циклом
    return result;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Подключение к Wi-Fi
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected!");
}

void loop() {
    static int prevSensorValue = 0;
    static bool isInThreshold = false;

    Serial.print("Loop started...");

    if (WiFi.status() == WL_CONNECTED) {
        int currentSensorValue = getNextValue();
        Serial.print("Current sensor value: " + String(currentSensorValue));

        if (currentSensorValue > prevSensorValue) {
            Serial.print("Show video");
            httpGETRequest(getVideoUrl);
            delay(5000);
            httpGETRequest(getImagesUrl);
        }

        if (currentSensorValue >= sensorThreshold && !isInThreshold) {
            Serial.print("Show next image");
            httpGETRequest(nextImagesUrl);
            isInThreshold = true;
        }

        if (currentSensorValue < sensorThreshold && isInThreshold) {
            Serial.print("Show prev image");
            httpGETRequest(prevImagesUrl);
            isInThreshold = false;
        }
    
        Serial.print("Wait for new loop...");
        delay(5000);
    } else {
        Serial.println("WiFi not connected!");
        delay(1000);
    }
}