#include <EEPROM.h>

#define light_sample_interval_address 0
#define light_threshold_address 4
#define light_on_time_address 8
#define light_off_time_address 12
//For now, the motor consts are using adresses 12 to 64
#define data_start_address 64

#define INDICATOR_PIN 13
#define LIGHT_CONTROL_PIN 10
#define LIGHT_SENSOR_PIN 7

#define EEPROM_LENGTH 1024

unsigned long reference_time;
unsigned long reference_millis;

int pump_control_pins[3][2] = { {2, 3}, {-1, -1}, {-1, -1} };

bool time_set = false;

float light_value = -1;
unsigned long last_light_sample_time = 0;

void setup() {

  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
  pinMode(INDICATOR_PIN, OUTPUT);
  pinMode(LIGHT_CONTROL_PIN, OUTPUT);
  lightOff();
  
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
  
}

void lightOn() {
  digitalWrite(LIGHT_CONTROL_PIN, LOW);
  digitalWrite(INDICATOR_PIN, HIGH);
}

void lightOff() {
  digitalWrite(LIGHT_CONTROL_PIN, HIGH);
  digitalWrite(INDICATOR_PIN, LOW);
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

void dispenseMilliseconds(int pump_no, long milliseconds) {
  float clearing_time = 4000;
  
  // lookup the control pins for the specified pump number
  int PUMP_PIN1 = pump_control_pins[pump_no][0];
  int PUMP_PIN2 = pump_control_pins[pump_no][1];

  if(PUMP_PIN2 == -1) {
       //Activate the peristaltic pump for the activation time
       digitalWrite(PUMP_PIN1, HIGH);
       delay(milliseconds);
       //Do I need to reverse this pump?
       digitalWrite(PUMP_PIN1, LOW);
    } else {
       //Activate the peristaltic pump for the activation time
       digitalWrite(PUMP_PIN1, HIGH);
       digitalWrite(PUMP_PIN2, LOW);
       delay(milliseconds);

       //Reverse the pump to clear it for the duration of the clearing time
       digitalWrite(PUMP_PIN1, LOW);
       digitalWrite(PUMP_PIN2, HIGH);
       delay(clearing_time);

       //Turn off the pump
       digitalWrite(PUMP_PIN1, LOW);
       digitalWrite(PUMP_PIN2, LOW);
    }
}

void dispenseMilliliters(int pump_no, long milliliters) {
  long milliseconds;
  long motor_const = getMotorConst(pump_no);

  // calculate the milliseconds to run: millilitres * constant
  //Does this need to be divided instead?
  milliseconds = milliliters * motor_const;
  dispenseMilliseconds(pump_no, milliseconds);
} 
