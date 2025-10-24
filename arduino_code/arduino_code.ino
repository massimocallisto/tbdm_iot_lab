#include <SoftwareSerial.h>
#include "rgb_lcd.h"

#define TOUCH_PIN 2
#define BUTTON_PIN 6
#define LED_PIN 3
#define BUZZER_PIN 5

#define TEMP_SENSOR A0
#define LIGHT_SENSOR A1
#define SOUND_SENSOR A2

#define B 3975

#define BT_TX_PIN 11
#define BT_RX_PIN 10


rgb_lcd lcd;

SoftwareSerial Ser =  SoftwareSerial(BT_RX_PIN, BT_TX_PIN);


float read_temperature(){

  // Get the (raw) value of the temperature sensor.
  int val = analogRead(TEMP_SENSOR);

  // Determine the current resistance of the thermistor based on the sensor value.
  float resistance = (float)(1023-val)*10000/val;

  // Calculate the temperature based on the resistance value.
  float temperature = 1/(log(resistance/10000)/B+1/298.15)-273.15;

  return temperature;
}

int read_light(){
  int lightLevel = analogRead(LIGHT_SENSOR);
  return lightLevel;
}

int read_sound(){
  int soundLevel = analogRead(SOUND_SENSOR);

  return soundLevel;
}

void setup() {
  //pinMode(TOUCH_PIN, INPUT);
  //pinMode(BUTTON_PIN, INPUT);
  //pinMode(LED_PIN, OUTPUT);
  //pinMode(BUZZER_PIN, OUTPUT);
  //digitalWrite(LED_PIN, LOW);
  //digitalWrite(BUZZER_PIN, LOW);

  // Inizializzazione della delle colonne e righe del display LCD
  lcd.begin(16, 2);
  // Stampa di un messaggio
  lcd.print("Starting...");

  Serial.begin(9600);

    // Definizione modalità pin
  pinMode(BT_RX_PIN, INPUT);
  pinMode(BT_TX_PIN, OUTPUT);
  // Inizializzazione della comunicazione Bluetooth
  Ser.begin(9600);
}

void send_data() {
  int soundLevel = read_sound();
  float temperatureC = read_temperature();
  int lightLevel = analogRead(LIGHT_SENSOR);
  Ser.print("{\"device_id\":\"arduino01\"");
  Ser.print(",\"adc_temp\":");  Ser.print(temperatureC);
  Ser.print(",\"adc_light\":"); Ser.print(lightLevel);
  Ser.print(",\"adc_sound\":"); Ser.print(soundLevel);
  Ser.println("}");
  delay(1000);
}

void print_info(){
  lcd.clear();
  // Lettura sensori
  int soundLevel = read_sound();
  float temperatureC = read_temperature();
  int lightLevel = analogRead(LIGHT_SENSOR);

  lcd.setCursor(0, 0);
  lcd.print("T.:");
  lcd.setCursor(3, 0);
  lcd.print(temperatureC);

  lcd.setCursor(9, 0);
  lcd.print("L.:");
  lcd.setCursor(12, 0);
  lcd.print(lightLevel);

  lcd.setCursor(0, 1);
  lcd.print("Snd:");
  lcd.setCursor(4, 1);
  lcd.print(soundLevel);
}

void loop() {
  Serial.println("....");
  print_info();
  send_data();
 
  delay(15000);
}
