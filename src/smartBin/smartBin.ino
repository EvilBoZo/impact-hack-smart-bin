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

const int sensorThresholdPercent = 80;
const int sensorMinimalStepPercent = 5;
const int binHeightCm = 60;

const int trigPin = 5;
const int echoPin = 18;

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701



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

// Функция опроса ультразвукового датчика
int getBinFillLevelPercent() {
    long duration;
    float distanceCm;
  
    // Генерируем ультразвуковой импульс
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
  
    // Считываем время возврата сигнала
    duration = pulseIn(echoPin, HIGH);
  
    // Вычисляем расстояние
    distanceCm = duration * SOUND_SPEED / 2;
    Serial.println("Sensor value in cm: " + String(distanceCm));
  
    // Преобразуем расстояние в процент заполнения
    int percent = 100 - (int)((distanceCm / binHeightCm) * 100);
  
    return percent;
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

    pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(echoPin, INPUT); // Sets the echoPin as an Input
}

void loop() {
    static int prevSensorValue = 0;
    static bool isInThreshold = false;

    Serial.print("Loop started...");

    if (WiFi.status() == WL_CONNECTED) {
        int currentSensorValue = getBinFillLevelPercent();
        Serial.print("Current sensor value: " + String(currentSensorValue));

        if (currentSensorValue > 100 || currentSensorValue < 0) {
            Serial.println(" - Sensor error");
            delay(5000);
            return;
        }

        if (abs(currentSensorValue - prevSensorValue) < sensorMinimalStepPercent) {
            Serial.println(" - No new garbage");
            delay(5000);
            return;
        }

        if (currentSensorValue >= sensorThresholdPercent && !isInThreshold) {
            Serial.println(" - Show next image");
            httpGETRequest(nextImagesUrl);
            isInThreshold = true;
            delay(1000);
        }

        if (currentSensorValue < sensorThresholdPercent && isInThreshold) {
            Serial.println(" - Show prev image");
            httpGETRequest(prevImagesUrl);
            isInThreshold = false;
            delay(1000);
        }

        if (currentSensorValue > prevSensorValue) {
            Serial.println(" - Show video");
            httpGETRequest(getVideoUrl);
            delay(5000);
            httpGETRequest(getImagesUrl);
        }
    
        prevSensorValue = currentSensorValue;
        Serial.println("Wait for new loop");
        delay(5000);
    } else {
        Serial.println("WiFi not connected!");
        delay(1000);
    }
}