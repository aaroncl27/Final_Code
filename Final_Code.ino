//DT01
//Heat Therapy Pseudocode

#include <project.h>

void setup() {
  delay(200);
  Serial.begin(115200);\
  
  pinMode(Timer, INPUT);
  pinMode(TempPin1, INPUT);                                         //Declare pin to read temperature switch as an input
  pinMode(TempPin2, INPUT);                                         //Declare pin to read temperature switch as an input
  pinMode(TimePin, INPUT);                                          //Declare pin to read timer switch as an input
  pinMode(Button, INPUT);                                           //Declare pin to read start button as an input
  pinMode(RelayCheck, INPUT);                                       //Declare pin to check if the relay is open or closed as an input
  pinMode(RelayPin, OUTPUT);                                        //Declare pin to toggle the latch on the relay as an output
  pinMode(LedPin, OUTPUT);                                          //Declare pin to turn on/off the light in the power button output
  pinMode(CoilRelayPin, OUTPUT);                                    //Declare pin to toggle the latch on the relay as an output
  
  digitalWrite(Button, LOW);                                        //Initialize the state of the Start Button pin as Low
  digitalWrite(RelayCheck, LOW);                                    //Initialize the state of the Relay Check pin as Low
  digitalWrite(CoilRelayPin, HIGH);                                 //Initialize the state of the Coil Relay Toggle pin as Low
  //InitTimersSafe(); //initialize all timers except for 0, to save time keeping functions
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);                        // Initialize the LCD screen
  display.display();                                                // Turns on the display and shows contents of display buffer
  delay(250);
  display.clearDisplay();                                           //Clears Display
  sensors.begin();                                                  //Initializes sensors
  Init();                                                           //Calls the function to read the position of settings switches

}

void (* resetFunc)(void) = 0;                                       //standalone function that when called, will restart the arduino


void Init() {                                                       //Get Operating Temperature and Runtime Settings
  if (digitalRead(TimePin) == HIGH) {
    SetTime = 900000;                                               //15 desired minutes in milliseconds
  }
  else {
    SetTime = 1800000;                                              //30 desired minutes in milliseconds
    SetTimeConst = SetTime;
  }

  if (digitalRead(TempPin1) == HIGH) {
    SetTemp = 38;                                                   //Sets desired temp to 38C
  }
  else if (digitalRead(TempPin2) == HIGH) {
    SetTemp = 34;                                                   //Sets desired temp to 34C
  }
  else {
    SetTemp = 36;                                                   //Sets desired temp to 36C
  }
  Serial.print("Device Runtime: ");
  Serial.println(SetTime / 60 / 1000);                              //Display Desired Runtime in Serial Monitor on Computer for debugging purposes
  Serial.print("Device Temperature Setpoint: ");
  Serial.println(SetTemp);                                          //Display Desired Operating Temperature in Serial Monitor on Computer for debugging purposes

}

void tempCheck() {                                                  // Check if temperature is within the safe range and control power to coil to control current temperature
  float temp;
  temp =  Temp();                                                   //Read temperature from bandage
  if (temp >= 40) {
    pulseOff();                                                     //If the temperature read by the device is over 40C, kill power to the device
  }

                                                                    // Control temperature to keep within desired range
  if ((temp < SetTemp - 0.2) && (digitalRead(RelayCheck) == LOW)) { // Close the relay if the temperature is 0.2 degrees below the desired temperature
    digitalWrite(CoilRelayPin, LOW);                                //This will provide power to the coil, heating up the bandage
    delay(20);
    digitalWrite(CoilRelayPin, HIGH);
    delay(1000);
  }

  if ((temp >= SetTemp + 0.2) && (digitalRead(RelayCheck) == HIGH)) {//Open the relay if the temperature is 0.2 degrees above the desired temperature
    digitalWrite(CoilRelayPin, LOW);                                 //This will remove power from the coil, allowing the bandage to cool
    delay(20);
    digitalWrite(CoilRelayPin, HIGH);
    delay(1000);
  }

}

float Temp() {
  float avgTemp, compAvgTemp;                                       //Holds the average temperature and a temporary average temperature to compare sensors to
  int devCount, devCountAdj;                                        //Integers for the device count and the adjusted device count, accounting for sensors that aren't reading properly.
  
  sensors.requestTemperatures();                                    //Send the command to get temperatures
  devCount = sensors.getDeviceCount();                              //Set the number of connected devices into an integer
  devCountAdj = devCount;                                           //Set the initial adjusted device count to the value of the total number of connected devices
  float temp[devCount];                                             //Create an array based on the number of devices on the data bus
  compAvgTemp = sensors.getTempCByIndex(2);                         //Set the temperature from the second temperature sensor to be 
  for (int j = 0; j < devCount; j++) {                              //Populate the temperature array with readings from the sensors
    temp[j] = sensors.getTempCByIndex(j);
  }
  for (int j = 0; j < devCount; j++) {                              //Increment through the temperature array and set any sensors outside the average to be -127
    if ((temp[j] < compAvgTemp - 2)) {                              //Setting the sensors as -127 sets them as off and excludes them from average temperature calculations
      temp[j] = -127;
    }
  }
  for (int j = 0; j < devCount; j++) {                              //Increment through the temperature array and only include positive temperatures in the average temperature calculation
    if (temp[j] > 0) {
      avgTemp += temp[j];
    }
    else {
      devCountAdj--;                                                //If a device is negative, subtract one from the number of devices on the bus. This is used for the calculation of the average
    }
  }
  avgTemp = avgTemp / devCountAdj;                                  //Divide the average temperature of the devices which read positive by the adjusted device count
  compAvgTemp = avgTemp;                                            //Set the average for the sensors to be compared to as current average temperature
  Serial.print("Desired Temp: ");                                   //Print the temperatures on in the serial debugger
  Serial.println(SetTemp);
  Serial.print("Average Temp: ");
  Serial.println(avgTemp);
  Display(avgTemp);                                                  //Call the display function and pass in the average temperature of the sensors
  return avgTemp;
}

void Display(float tempReadout) {                                    //Update the display with the current temperature and the remaining time
  display.clearDisplay();
  display.setTextSize(1);                                            //Set Text Size to one for the top line. This allows 2 lines of text to fit. Otherwise text size should be 2
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Operating Temp");
  display.println(SetTemp);
  display.setTextSize(2);
  display.println("Temp ");
  //Serial.println(Temp());
  display.print(tempReadout);
  display.println(" C");
  display.print(timeDisp);                                           //Print minutes remaining onto display
  display.println(" Min Rem");
  display.display();                                                 //Print contents of display buffer (desired operating temperature, current temperature, and minutes remaining) on the display

}


bool startButton() {
  int reading;
  ButtonState = LOW;
  reading = digitalRead(Button);                                   //Set the state of the button into a variable

  if (reading != lastButtonState) {
    lastDebounceTime = millis();                                   //Reset the debouncing timer
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {             //Compare the run time since the button was pressed to the preset debounce delay of 50ms

    if (reading != ButtonState) {                                  //If the state of the button reading does not match the state of the button, then the button is fine
      ButtonState = reading;
    }
  }
  lastButtonState = reading;                                      //Set the state of the button to to be compared on the next iteration
  return ButtonState;                                             //Return whether the debounced button is high or low
}

void pulseOff() {                                                 //The pulseOff() function is used to pulse off the relay when the temperature is too high or when the time is up
  digitalWrite(LedPin, LOW);
  if (digitalRead(RelayCheck) == HIGH) {                          //Only pulse off the relay if its currently closed. If the relay is currently open, then just reset the arduino
    digitalWrite(CoilRelayPin, LOW);                              //Brings pin tied to the latch on the relay to low
    delay(50);                                                    //waits for 20 ms
    digitalWrite(CoilRelayPin, HIGH);                             //Brings pin tied to the relay to high, flipping the latch on the relay
    delay(5000);
  }
  resetFunc();                                                    //Call the reset function to reset the arduino when the time is up or temperature is exceeded

}

void Main()
{
  int k = 0;
  bool estop;
  
  while (true)
  {
    estop = startButton();                                        //Set the state of the Start/Stop button to a variable upon each iteration of the loop
    if (millis() % 1000 == 0) {                                   //Update the time/temperature on the screen once per second
      Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");       //Seperates the debug information in the serial display on the PC
      tempCheck();                                                //Check the temperature of the device/set coil state to adjust temperature
      
      if ((digitalRead(TimePin) == HIGH && SetTime != 900000) || ((digitalRead(TimePin) == LOW && SetTime != 1800000))) {
        delay(100);
        Init();                                                   //Calls the function to read the position of settings switches if the state of the switches has changed since startup
      }
      if ((digitalRead(TempPin1) == HIGH && SetTemp != 38) || ((digitalRead(TempPin2) == HIGH && SetTemp != 34)) || (((digitalRead(TempPin1) == LOW && digitalRead(TempPin2) == LOW) && SetTemp != 36))) {
        delay(250);
        Init();                                                   //Calls the function to read the position of settings switches if the state of the switches has changed since startup
      }
      
      delay(500);                                                 
      //if (digitalRead(RelayState) == 1) {
      //  Serial.println("RELAY CLOSED");
      //}
      //else
      //  Serial.println("RELAY OPEN");
        
      runTime = millis() - StandbyTime;                           //Subtract the time that the device waited for the user to start it from the time the device has been operating, and declare this as the actual time of operation
      Serial.print("Runtime in seconds: ");                       
      Serial.println(runTime / 1000);                             //Display the adjusted runtime of the device in seconds for debugging purposes
      Serial.println(startButton());                              
      //Serial.println(sensors.getTempCByIndex(0));
      //Serial.println(sensors.getTempCByIndex(1));
      //Serial.println(sensors.getTempCByIndex(2));
      //Serial.println(sensors.getTempCByIndex(3));
      //Serial.println(sensors.getTempCByIndex(4));
      //Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
      timeDisp = (((SetTime / 1000) / 60) - ((runTime / 1000) / 60));   //compare the desired run time in minutes to the current run time in minutes. This number is printed on the dispaly

      if (timeDisp <= 0 || estop == HIGH) {                             //when the timer on the device reaches zero
        display.clearDisplay();
        display.setTextSize(2);                                         //Set Text Size to one for the top line. This allows 2 lines of text to fit. Otherwise text size should be 2
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println();
        display.println("    Done");
        display.print("  Heating");
        display.display();
        delay(500);
        pulseOff();
      }
    }
  }
}

void initDisplay() {                                                  //Prints a fun animated loading screen while the device pauses to allow components to initialize
  display.clearDisplay();
  display.setTextSize(2);                                             //Set Text Size to one for the top line
  display.setTextColor(WHITE);                                        //Set the text color to white
  display.setCursor(0, 0);                                            //Set cursor on first line of the screen to start printing
  display.println();                                                  //Leaves top line blank
  display.println("  Initial");
  display.println("   Setup");
  display.println("    .");
  display.display();
  delay(1000);
  display.clearDisplay();
  display.setTextSize(2);                                             //Set Text Size to one for the top line
  display.setTextColor(WHITE);                                        //Set the text color to white
  display.setCursor(0, 0);                                            //Set cursor on first line of the screen to start printing
  display.println();                                                  //Leaves top line blank
  display.println("  Initial");
  display.println("   Setup");
  display.println("    ..");
  display.display();
  delay(1000);
  display.clearDisplay();
  display.setTextSize(2);                                            //Set Text Size to one for the top line
  display.setTextColor(WHITE);                                       //Set the text color to white
  display.setCursor(0, 0);                                           //Set cursor on first line of the screen to start printing
  display.println();                                                 //Leaves top line blank
  display.println("  Initial");
  display.println("   Setup");
  display.println("    ...");
  display.display();
}

void loop () {
    display.clearDisplay();                                         //Clear the display
    display.setTextSize(3);                                         //Set Text Size to one for the top line
    display.setTextColor(WHITE);                                    //Set the text color to white
    display.setCursor(0, 0);                                        //Set cursor on first line of the screen to start printing
    display.println();                                              //Leaves top line blank
    display.display();                                              //Prints contents of display buffer onto display
    
  if ((digitalRead(RelayCheck) == HIGH)&&initRelay==LOW) {          //If the relay is on when the device is powered on, it is toggled low immediately to keep the device from heating uncontrollably and/or keep the relay state from being inverted upon launch
    digitalWrite(CoilRelayPin, LOW);                                //Brings pin tied to the latch on the relay to low
    delay(50);                                                      //waits for 20 ms
    digitalWrite(CoilRelayPin, HIGH);                               //Brings pin tied to the relay to high, flipping the latch on the relay
    delay(3500);
    initRelay=HIGH;                                                 //Tell the system that the relay is now intialized to be open, and it is ready for operation
  }
  
  StandbyTime = millis();                                           //Keep track of how long the device waits for the user to press the button. This is used for calculating the runtime of the device.
  
  if (startButton() == HIGH) {                                      //Loop until the user presses the Start button. The user must press the button to start the program. Otherwise it'll loop forever.
    digitalWrite(LedPin, HIGH);                                     //Turn on the light in the button
    initDisplay();                                                  //Function to show a "loading screen" of sorts on the screen.

    digitalWrite(RelayPin, LOW);                                    //Brings pin tied to the latch on the relay to low
    delay(20);                                                      //waits for 20 ms
    digitalWrite(RelayPin, HIGH);                                   //Brings pin tied to the relay to high, flipping the latch on the relay

    Main();                                                         //Goes to main function of device
  }
}
