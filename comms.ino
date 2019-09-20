unsigned long getTime() {
  unsigned long interval = (millis() - reference_millis)/1000;
   return reference_time + interval;
}

unsigned long getTimeOfDay() {
  return (getTime() % 86400);
}

unsigned long getMotorConst(int pump_no) {
    long address = (12 + (pump_no * 4));
    return Serial.println(EEPROMReadlong(address));
}

void setMotorConst(long pump_no, float constant) {
    long address = (12 + (pump_no * 4));
    long motor_const = (long)constant;
    EEPROMWritelong(address, motor_const);
    //For all intensive testing purposes 
    Serial.println(motor_const);
}

void clearEeprom() {
  for (int i = 0 ; i < EEPROM_LENGTH; i++) {
    EEPROM.write(i, '\0');
  }
}


void EEPROMWritelong(int address, long value) {
  //Decomposition from a long to 4 bytes by using bitshift.
  //One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);
  
  //Write the 4 bytes into the eeprom memory.
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}

long EEPROMReadlong(long address){
  //Read the 4 bytes from the eeprom memory.
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);
  
  //Return the recomposed long by using bitshift.
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

void process_comms() {

  while (Serial.available()) {
    String data_string = Serial.readString();  
  
    unsigned long timestamp = data_string.substring(0, data_string.indexOf(' ')).toInt();
    reference_millis = millis();
    reference_time = timestamp;
    time_set= true;
    
    String command = data_string.substring(data_string.indexOf(' ') + 1);
    String arg = command.substring(command.indexOf(' ') + 1);
    command = command.substring(0, command.indexOf(' '));  
    
    // respond to comman1d
    if (command == "echo") {
      Serial.println(arg);
    }
    else if (command == "getTimeOfDay") {
      Serial.println(getTimeOfDay());
    }
    else if (command == "setLightOnTime") {
      long hour = arg.substring(0, arg.indexOf(':')).toInt();
      long minute = arg.substring(arg.indexOf(':') + 1).toInt();
      long interval = hour * 3600 + minute * 60;
      EEPROMWritelong(light_on_time_address, interval);
      Serial.println(interval);
      last_light_sample_time = 0;
    }
    else if (command == "setLightOffTime") {
      long hour = arg.substring(0, arg.indexOf(':')).toInt();
      long minute = arg.substring(arg.indexOf(':') + 1).toInt();
      long interval = hour * 3600 + minute * 60;
      EEPROMWritelong(light_off_time_address, interval);
      Serial.println(interval);
      last_light_sample_time = 0;
    }
    else if (command == "getLightOnTime") {
      Serial.println(EEPROMReadlong(light_on_time_address));
    }
    else if (command == "getLightOffTime") {
      Serial.println(EEPROMReadlong(light_off_time_address));
    }
    else if (command == "setLightSampleInterval") {
      long interval = arg.toInt();
      EEPROMWritelong(light_sample_interval_address, interval);
      Serial.println(EEPROMReadlong(light_sample_interval_address));
    }
    else if (command == "getLightSampleInterval") {
      Serial.println(EEPROMReadlong(light_sample_interval_address));
    }

    else if (command == "setLightThreshold") {
      long threshold = arg.toInt();
      EEPROMWritelong(light_threshold_address, threshold);
      Serial.println(EEPROMReadlong(light_threshold_address));
    }
    else if (command == "getLightThreshold") {
      Serial.println(EEPROMReadlong(light_threshold_address));
    }

    else if (command == "getLightSample") {
      Serial.println(sampleLightSensor((arg=="True")));
    }
    
    // format should be: setMotorConstant [pump_no] [constant]
    else if (command == "setMotorConstant") {
      long pump_no = arg.substring(0, arg.indexOf(' ')).toInt();
      //This gives us three decimal places and allows the user to input a float
      float constant = (arg.substring(arg.indexOf(' ') + 1).toFloat()) * 1000; 
      setMotorConst(pump_no, constant);
    }

    // format should be: getMotorConstant [pump_no] [constant]
    else if (command == "getMotorConstant") {
      long pump_no = arg.substring(0, arg.indexOf(' ')).toInt();
      //Unecessary?
      long constant = arg.substring(arg.indexOf(' ') + 1).toInt();
      Serial.println(getMotorConst(pump_no));
    }
    
    // dispense for a certain time
    // format should be: dispenseMilliseconds [pump_no] [milliseconds]
    else if (command == "dispenseMilliseconds") {
      long pump_no = arg.substring(0, arg.indexOf(' ')).toInt();
      long milliseconds = arg.substring(arg.indexOf(' ') + 1).toInt();
      dispenseMilliseconds(pump_no, milliseconds);
      Serial.println(0x06);
    }
    
    // dispense a certain amount
    // format should be: dispenseMilliliters [pump_no] [milliliters]
    else if (command == "dispenseMilliliters") {
      long pump_no = arg.substring(0, arg.indexOf(' ')).toInt();
      long milliliters = arg.substring(arg.indexOf(' ') + 1).toInt();
      dispenseMilliliters(pump_no, milliliters);
      Serial.println(0x06);
    }
    
  }
}
