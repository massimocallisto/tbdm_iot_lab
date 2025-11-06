#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include <time.h> // SNTP + timezone
#include <Wire.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ESP32 (DevKit): RX2=GPIO16, TX2=GPIO17
HardwareSerial SerialArduino(2); // UART2

// WiFi
#define WIFI_SSID "BIGDATA"
#define WIFI_PASS "bigdata1234"

// MQTT (TCP)
#define MQTT_HOST "64.225.82.0" // IP o hostname broker
#define MQTT_PORT 1883                                                                             // 8883 se TLS
#define MQTT_USER "user"
#define MQTT_PASS "user1234"
#define MQTT_TOPIC "/test/01/" //esp01" // topic di pubblicazione

// Identità dispositivo / payload
//static const char *DEVICE_ID = "esp01";
static const char *PAN_ID = "0001";
static const char *DEVICE_UUID = "d8826188-b1bd-4062-873f-48a35f7dffa4";
static const char *SENSOR_TYPE = "devtest";
static const char *FW_VERSION = "1.-esp32s-dht22"; // mantengo la stringa proposta
static const char *MSG_TYPE = "status";

// Timezone Italia (CET/CEST). POSIX TZ:
// CET-1CEST,M3.5.0,M10.5.0/3  => CET (UTC+1), CEST DST dalla 5a dom di Mar alla 5a dom di Ott alle 03:00
static const char *TZ_EU_ROME = "CET-1CEST,M3.5.0,M10.5.0/3";

// Per MQTT TCP (non TLS):
WiFiClient wifiClient;

PubSubClient mqtt(wifiClient);

unsigned long lastRead = 0;


void connectWiFi()
{
  if (WiFi.status() == WL_CONNECTED)
    return;
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi: connessione a ");
  Serial.print(WIFI_SSID);
  Serial.print(" ...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi connesso. IP: ");
  Serial.println(WiFi.localIP());
}

void setupTime()
{
  // Imposta timezone e server NTP (pool predefiniti)
  configTzTime(TZ_EU_ROME, "pool.ntp.org", "time.google.com", "time.cloudflare.com");
  Serial.print("Sync orologio NTP");
  // attende che il tempo sia sincronizzato
  for (int i = 0; i < 20; ++i)
  {
    time_t now = time(nullptr);
    if (now > 1700000000)
      break; // ~2023-11-14: tempo valido
    Serial.print(".");
    delay(500);
  }
  Serial.println(" OK");
}

// Formatta timestamp locale ISO8601 con offset, es. "2025-09-25T18:24:38.128+02:00"
String iso8601Now()
{
  struct timeval tv;
  gettimeofday(&tv, nullptr);

  time_t sec = tv.tv_sec;
  struct tm tm_local;
  localtime_r(&sec, &tm_local);

  char buf[40];
  // frazione millisecondi
  int ms = (int)(tv.tv_usec / 1000);

  // Calcola offset da UTC (in secondi) manualmente
  time_t local_sec = mktime(&tm_local);
  struct tm tm_utc;
  gmtime_r(&sec, &tm_utc);
  time_t utc_sec = mktime(&tm_utc);
  long gmtoff = difftime(local_sec, utc_sec); // positivo a est di Greenwich in secondi
  int off_h = (int)(gmtoff / 3600);
  int off_m = (int)((labs(gmtoff) % 3600) / 60);

  // segno dell’offset
  char sign = (gmtoff >= 0) ? '+' : '-';

  // data e ora base
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tm_local);

  char finalBuf[64];
  snprintf(finalBuf, sizeof(finalBuf), "%s.%03d%c%02d:%02d", buf, ms, sign, abs(off_h), off_m);

  return String(finalBuf);
}

void ensureMqtt()
{
  while (!mqtt.connected())
  {
    Serial.print("MQTT: connessione a ");
    Serial.print(MQTT_HOST);
    Serial.print(":");
    Serial.print(MQTT_PORT);
    Serial.print(" ... ");

    // Se usi TLS:
    // wifiClientTLS.setInsecure(); // o setCACert(...)
    // mqtt.setClient(wifiClientTLS);

    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    const char *DEVICE_ID = clientId.c_str();

    if (mqtt.connect(DEVICE_ID, MQTT_USER, MQTT_PASS))
    {
      Serial.println("OK");
    }
    else
    {
      Serial.print("fallita, rc=");
      Serial.print(mqtt.state());
      Serial.println(" (ritento tra 2s)");
      delay(2000);
    }
  }
}

void publishReading(const char *dev, float temperature, int light, int sound)
{
  // Costruzione JSON
  DynamicJsonDocument doc(512);
  doc["tz"] = iso8601Now(); // stringa ISO8601 locale con offset
  doc["type"] = MSG_TYPE;
  doc["sensorType"] = SENSOR_TYPE;
  doc["fw_version"] = FW_VERSION; // corretto ":" (non ";")
  doc["uuid"] = DEVICE_UUID;
  doc["id"] = PAN_ID;
  doc["id"] = dev;
  doc["temperature"] = temperature; // float
  doc["light"] = light;             // int
  doc["sound"] = sound;             // int

  // Serializza in buffer
  String payload;
  serializeJson(doc, payload);
  String topic = String(MQTT_TOPIC) + dev;


  Serial.print("PUB ");
  Serial.print(topic.c_str());
  Serial.print(" -> ");
  Serial.println(payload);

  // QoS 0, retain false
  mqtt.publish(topic.c_str(), payload.c_str());
}

void setup()
{
  Serial.begin(115200);
  delay(200);

  SerialArduino.begin(9600, SERIAL_8N1, 16, 17); // (baud, config, RX, TX)

  connectWiFi();
  setupTime();

  // MQTT
  mqtt.setServer(MQTT_HOST, MQTT_PORT);

  Serial.println();
  Serial.println(F("ESP32 + reader"));

  // Piccolo warmup: la prima lettura spesso è NaN
  delay(2000);
}

String line;

void loop()
{
    // mantieni Wi-Fi / MQTT
  if (WiFi.status() != WL_CONNECTED)
    connectWiFi();
  if (!mqtt.connected())
    ensureMqtt();
  mqtt.loop();

   while (SerialArduino.available())
  {
    char c = (char)SerialArduino.read();
    if (c == '\n')
    {
      Serial.print("FROM ARDUINO: ");
      Serial.println(line);

      // buffer for JSON
      StaticJsonDocument<256> doc;

      // Parsing
      DeserializationError err = deserializeJson(doc, line);
      if (err)
      {
        Serial.print("JSON parse failed: ");
        Serial.println(err.c_str());
      }
      else
      {
        const char *dev = doc["device_id"];
        int temp_adc = doc["adc_temp"];
        int light_adc = doc["adc_light"];
        int sound_adc = doc["adc_sound"];

        // 4️(Facoltativo) Conversioni / logica
        float temp_c = temp_adc;//(temp_adc / 1023.0) * 100.0; // esempio fittizio

        // 5️Output
        Serial.println("Parsed successfully:");
        Serial.printf(" Device: %s\n", dev);
        Serial.printf(" Temp: %.2f°C\n", temp_adc);
        Serial.printf(" Light: %d\n", light_adc);
        Serial.printf(" Sound: %d\n", sound_adc);
        
        publishReading(dev, temp_c, light_adc, sound_adc);
      }

      line = "";
      //delay(5000);
    }
    else if (c != '\r')
    {
      line += c;
    }
  }
}
