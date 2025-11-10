#include <SoftwareSerial.h>
#include "rgb_lcd.h"

#define TEMP_SENSOR A0
#define LIGHT_SENSOR A1
#define SOUND_SENSOR A2

#define B 3975

#define BT_RX_PIN 10
#define BT_TX_PIN 11


rgb_lcd lcd;

SoftwareSerial Ser =  SoftwareSerial(BT_RX_PIN, BT_TX_PIN);


float convert_temperature(int val){
  // Determine the current resistance of the thermistor based on the sensor value.
  float resistance = (float)(1023-val)*10000/val;

  // Calculate the temperature based on the resistance value.
  float temperature = 1/(log(resistance/10000)/B+1/298.15)-273.15;

  return temperature;
}

float read_temperature(){

  // Get the (raw) value of the temperature sensor.
  int val = analogRead(TEMP_SENSOR);

  float temperature = convert_temperature(val);

  return temperature;
}

int read_light(){
  int lightLevel = analogRead(LIGHT_SENSOR);
  // TODO: use analogRead from LIGHT_SENSOR

  return lightLevel;
}

void setup() {
  // Init LCD columns and rows
  lcd.begin(16, 2);
  
  lcd.print("Starting...");

  Serial.begin(9600);

  // Serial PIN pin
  pinMode(BT_RX_PIN, INPUT);
  pinMode(BT_TX_PIN, OUTPUT);
  Ser.begin(9600);
}


void send_data_as_json(float temperatureC, int lightLevel, int soundLevel) {
  Ser.print("{\"device_id\":\"arduino01\""); // <-- Important: Change with YOUR TEAM NAME (use only [a-zA-Z0-9]*)
  Ser.print(",\"adc_temp\":");  Ser.print(temperatureC);
  Ser.print(",\"adc_light\":"); Ser.print(lightLevel);
  Ser.print(",\"adc_sound\":"); Ser.print(soundLevel);
  Ser.println("}");
}

void print_info(float temperatureC, int lightLevel, int soundLevel){
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("T.:");
  lcd.setCursor(3, 0);
  lcd.print(temperatureC);

  lcd.setCursor(9, 0);
  lcd.print("L.:");
  lcd.setCursor(12, 0);
  lcd.print(lightLevel);

  lcd.setCursor(0, 1);
  lcd.print("S.:");
  lcd.setCursor(12, 0);
  lcd.print(soundLevel);

}

void loop() {
  Serial.println("....");

  // READ sensor data
  float temperatureC = read_temperature();
  int lightLevel = analogRead(LIGHT_SENSOR);
  int soundLevel = analogRead(SOUND_SENSOR);

  // TODO: add the sound measure

  print_info(temperatureC, lightLevel, soundLevel);
  send_data_as_json(temperatureC, lightLevel, soundLevel);
 
  delay(5000);
}
