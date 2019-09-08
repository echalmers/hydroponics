#include <EEPROM.h>

#define light_sample_interval_address 0
#define light_threshold_address 4
#define light_on_time_address 8
#define light_off_time_address 12
#define ebb_flow_pump_interval_address 16
#define ebb_flow_pump_activation_address 20
#define data_start_address 64

#define EBB_FLOW_PUMP_PIN 8
#define LIGHT_CONTROL_PIN 10
#define LIGHT_SENSOR_PIN 7
#define INDICATOR_PIN 13

#define EEPROM_LENGTH 1024

unsigned long reference_time;
unsigned long reference_millis;

unsigned long ebb_flow_pump_change_millis = 300000;
bool ebb_flow_pump_is_on = 0;

bool time_set = false;

float light_value = -1;
unsigned long last_light_sample_time = 0;

void setup() {

  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
  pinMode(EBB_FLOW_PUMP_PIN, OUTPUT);
  pinMode(LIGHT_CONTROL_PIN, OUTPUT);
  pinMode(INDICATOR_PIN, OUTPUT);
  
  pinMode(LIGHT_SENSOR_PIN, INPUT);

  lightOff();
  ebb_flow_pump_off();
  
}

void loop() {

  // process comms
  process_comms();

  if (time_set) {
    // enable/disable light using specified schedule
    unsigned long time_of_day = getTimeOfDay();
    unsigned long light_on_time = EEPROMReadlong(light_on_time_address);
    unsigned long light_off_time = EEPROMReadlong(light_off_time_address);
    bool light_enable = false;
    if ((time_of_day > light_on_time) & (time_of_day < light_off_time)) {
      light_enable = true;
    }
  
    // sample ambient light on schedule
    unsigned long current_time = getTime();
    unsigned long sample_interval = EEPROMReadlong(light_sample_interval_address);
    if ((unsigned long)(current_time - last_light_sample_time) >= sample_interval) {
  
      int sample = sampleLightSensor(true);
  
      int threshold = EEPROMReadlong(light_threshold_address);
      if ((light_enable) and (sample < threshold)) {lightOn();}
      else {lightOff();}
  
      last_light_sample_time = current_time;
    }
  }

  // run ebb & flow pump
  unsigned long current_millis = millis();
  if (current_millis > ebb_flow_pump_change_millis) {
    if (ebb_flow_pump_is_on) {
      unsigned long interval = EEPROMReadlong(ebb_flow_pump_interval_address);
      ebb_flow_pump_change_millis = current_millis + interval;
      ebb_flow_pump_off();
    }
    else {
      unsigned long activation_time = EEPROMReadlong(ebb_flow_pump_activation_address);
      ebb_flow_pump_change_millis = current_millis + activation_time;
      ebb_flow_pump_on();
    }
  }
  
}

void lightOn() {
  digitalWrite(LIGHT_CONTROL_PIN, LOW);
}

void lightOff() {
  digitalWrite(LIGHT_CONTROL_PIN, HIGH);
}

void ebb_flow_pump_on() {
  digitalWrite(EBB_FLOW_PUMP_PIN, LOW);
  digitalWrite(INDICATOR_PIN, HIGH);
  ebb_flow_pump_is_on = 1;
}

void ebb_flow_pump_off() {
  digitalWrite(EBB_FLOW_PUMP_PIN, HIGH);
  digitalWrite(INDICATOR_PIN, LOW);
  ebb_flow_pump_is_on = 0;
}

int sampleLightSensor(bool daylight_only) {
  bool currentState = digitalRead(LIGHT_CONTROL_PIN);
  if (daylight_only) {
    lightOff();
    delay(500);
  }
  int sample = analogRead(LIGHT_SENSOR_PIN);
  if (daylight_only) { digitalWrite(LIGHT_CONTROL_PIN, currentState); }
  return sample;
}
