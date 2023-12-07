#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <dht.h>

#define DHT11_PIN 18
#define DS18B20 5
#define REPORTING_PERIOD_MS 1000

float temperature, humidity, BPM, SpO2, bodytemperature;
int ecgValue, ppgValue;
int xValue, yValue, zValue;

const char *ssid = "Alexahome"; // Enter SSID here
const char *password = "12345678"; // Enter Password here

dht DHT;
PulseOximeter pox;
uint32_t tsLastReport = 0;
OneWire oneWire(DS18B20);
DallasTemperature sensors(&oneWire);

#define ECG_PIN_1 26
#define ECG_PIN_2 27
#define PPG_PIN 34
#define EEG_PIN 32
const int groundpin = 18;
const int powerpin = 19;
const int xpin = 35;
const int ypin = 34;
const int zpin = 33;

WebServer server(80);

void onBeatDetected()
{
  Serial.println("Beat!");
}

void setup()
{
  Serial.begin(115200);
  pinMode(groundpin, OUTPUT);
  pinMode(powerpin, OUTPUT);
  digitalWrite(groundpin, LOW);
  digitalWrite(powerpin, HIGH);

  Serial.println("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");

  Serial.print("Initializing pulse oximeter..");

  if (!pox.begin())
  {
    Serial.println("FAILED");
    for (;;)
      ;
  }
  else
  {
    Serial.println("SUCCESS");
    pox.setOnBeatDetectedCallback(onBeatDetected);
  }

  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
}

void loop()
{
  server.handleClient();
  pox.update();
  sensors.requestTemperatures();
  int chk = DHT.read11(DHT11_PIN);

  temperature = DHT.temperature;
  humidity = DHT.humidity;
  BPM = pox.getHeartRate();
  SpO2 = pox.getSpO2();
  bodytemperature = sensors.getTempCByIndex(0);

  ecgValue = digitalRead(ECG_PIN_1) || digitalRead(ECG_PIN_2);
  ppgValue = analogRead(PPG_PIN);

  xValue = analogRead(xpin);
  yValue = analogRead(ypin);
  zValue = analogRead(zpin);

  float sensorValue = analogRead(EEG_PIN);
  float millivolt = (sensorValue / 4095.0) * 3300.0; // Assuming 12-bit ADC and 3.3V reference

  if (millis() - tsLastReport > REPORTING_PERIOD_MS)
  {
    Serial.print("Room Temperature: ");
    Serial.print(DHT.temperature);
    Serial.println("°C");

    Serial.print("Room Humidity: ");
    Serial.print(DHT.humidity);
    Serial.println("%");

    Serial.print("BPM: ");
    Serial.println(BPM);

    Serial.print("SpO2: ");
    Serial.print(SpO2);
    Serial.println("%");

    Serial.print("Body Temperature: ");
    Serial.print(bodytemperature);
    Serial.println("°C");

    Serial.print("ECG Value: ");
    Serial.println(ecgValue);

    Serial.print("PPG Value: ");
    Serial.println(ppgValue);

    Serial.print("Accelerometer Values: ");
    Serial.print(xValue);
    Serial.print("\t");
    Serial.print(yValue);
    Serial.print("\t");
    Serial.println(zValue);

    Serial.print("EEG Sensor Value: ");
    Serial.print(sensorValue);
    Serial.println(" mV");

    tsLastReport = millis();
  }

  delay(100);
}

void handle_OnConnect()
{
  server.send(200, "text/html", SendHTML(temperature, humidity, BPM, SpO2, bodytemperature, ecgValue, ppgValue, xValue, yValue, zValue));
}

void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float temperature, float humidity, float BPM, float SpO2, float bodytemperature, int ecgValue, int ppgValue, int xValue, int yValue, int zValue)
{
  String ptr = "<!DOCTYPE html>";
  // ... (rest of the HTML code is the same as before)
  return ptr;
}
