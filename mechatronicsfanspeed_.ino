

#include <LiquidCrystal.h>

volatile byte revolutions;

unsigned int rpm;
unsigned long timeold;

const int buttonPinUp = 5;
const int buttonPinDown = 6;

int fanPin = 3;    
int fanVal = 1024;
int sensorPin2 = A2;
int sensorPin1 = A1;
int sensorPin0 = A0;
int light = 4;
int looper = 1;
int setTemp = 65;
int temperatureFavg;
int RedLed = 23;
int BlueLed = 22;
int GreenLed = 21;


int buttonStateUp = 0;
int buttonStateDown = 0;
int temp10; int temp20; int temp30;
int temp11; int temp21; int temp31;
int temp12; int temp22; int temp32;
int temperatureFavg0; int temperatureFavg1; int temperatureFavg2;

//initialize the time stores as volatile so that can be accessed by the ISR
volatile long timeStoreA;
volatile long timeStoreB;
volatile bool timeStoreSwitch;
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

void MQTT_connect();

void setup()
{
  Serial1.begin(115200);
  Serial.begin(9600);
  
  pinMode(buttonPinUp, INPUT);
  pinMode(buttonPinDown, INPUT);

  pinMode(RedLed, OUTPUT);
  pinMode(BlueLed, OUTPUT);
  pinMode(GreenLed, OUTPUT);

  attachInterrupt(18, rpm_fun, RISING);
  pinMode(light, OUTPUT);
  
  revolutions = 0;
  rpm = 0;
  timeold = 0;


  timeStoreA = 0;
  timeStoreB = 0;
  timeStoreSwitch = true;
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
}
void loop()
{
//Control Set Temp with Buttons
  buttonStateUp = digitalRead(buttonPinUp);
  buttonStateDown = digitalRead(buttonPinDown);

  if (buttonStateUp == HIGH) {
    //Serial.print("High Works!!");
    setTemp++;
  }
  if (buttonStateDown == HIGH) {
    //Serial.print("Low Works!!");
    setTemp --;
  }

//Calculate Fan Speed
  //Serial.println(looper);
  if (micros() - timeold > 1e6) {
    //     rpm = (60./7.)*1000000./(micros() - timeold)*revolutions;
    //     timeold = micros();
    //     revolutions = 0;
    //     Serial.println(rpm,DEC);

    //Calculate the time per tick
    long deltaT = timeStoreA - timeStoreB;

    //make all positive chance in time
    deltaT = abs(deltaT);

    //7 ticks per revolution
    long revs = deltaT * 7.;

    //calculate revs per microsecond
    long RPS = 1. / revs * 1000000.;

    //convert to RPM
    long RPM = RPS * 60.;
    if (RPM > 3500) {
      RPM = 0;
    }

    //Print out the RPM over Serial
    //Serial.print("RPM:"); Serial.print(RPM); Serial.println("    ");
    analogWrite(fanPin, fanVal);
    
//Control LED Lights based on current temp and set temp    
    if (setTemp > temperatureFavg ) {
      digitalWrite(light, HIGH);   // turn the LED on (HIGH is the voltage level)
      analogWrite(fanPin, 0);
      RPM = 0;
      digitalWrite(BlueLed, HIGH);
      digitalWrite(RedLed, LOW);
      digitalWrite(GreenLed, LOW);
    }
    else if (setTemp < temperatureFavg) {
      digitalWrite(light, LOW);
      analogWrite(fanPin, fanVal);
      digitalWrite(BlueLed, LOW);
      digitalWrite(RedLed, HIGH);
      digitalWrite(GreenLed, LOW);
    }
    else if (setTemp == temperatureFavg) {
      digitalWrite(light, LOW);
      analogWrite(fanPin, 0);
      digitalWrite(BlueLed, LOW);
      digitalWrite(RedLed, LOW);
      digitalWrite(GreenLed, HIGH);
      delay(50);
    }

//Get Temperature Values From 3 Temperature Sensors
    int reading2 = analogRead(sensorPin2);
    int reading1 = analogRead(sensorPin1);
    int reading0 = analogRead(sensorPin0);
    // converting that reading to voltage, for 3.3v arduino use 3.3
    float voltage0 = reading0 * 3.3;
    float voltage1 = reading1 * 3.3;
    float voltage2 = reading2 * 3.3;
    voltage0 /= 1024.0;
    voltage1 /= 1024.0;
    voltage2 /= 1024.0;
    // print out the voltage
    // Serial.print("volts:"); Serial.print(voltage);
    // now print out the temperature
    float temperatureC0 = (voltage0 - 0.5) * 100 ; //converting from 10 mv per degree wit 500 mV offset
    float temperatureC1 = (voltage1 - 0.5) * 100 ; //converting from 10 mv per degree wit 500 mV offset
    float temperatureC2 = (voltage2 - 0.5) * 100 ; //converting from 10 mv per degree wit 500 mV offset
    //to degrees ((voltage - 500mV) times 100)
    //Serial.print(temperatureC0); Serial.println(" degrees C");
    //Serial.print(temperatureC1); Serial.println(" degrees C");
    //Serial.print(temperatureC2); Serial.println(" degrees C");
    // now convert to Fahrenheit
    float temperatureF0 = (temperatureC0 * 9.0 / 5.0) + 32.0;
    float temperatureF1 = (temperatureC1 * 9.0 / 5.0) + 32.0;
    float temperatureF2 = (temperatureC2 * 9.0 / 5.0) + 32.0;

// Find Average Temperature From 3 Last Readings
    if (looper == 0) {
      //Serial.print("loop1");
      temp10 = temperatureF0;
      temperatureFavg0 = (temp10 + temp20 + temp30) / 3;
      temp11 = temperatureF1;
      temperatureFavg1 = (temp11 + temp21 + temp31) / 3;
      temp12 = temperatureF1;
      temperatureFavg2 = (temp12 + temp22 + temp32) / 3;
      looper++;
    }
    else if (looper == 1) {
      //Serial.print(" loop2 ");
      temp20 = temperatureF0;
      temperatureFavg0 = (temp10 + temp20 + temp30) / 3;

      temp21 = temperatureF1;
      temperatureFavg1 = (temp11 + temp21 + temp31) / 3;

      temp22 = temperatureF2;
      temperatureFavg2 = (temp12 + temp22 + temp32) / 3;
      looper++;
    }
    else if (looper == 2) {
      //Serial.print("loop3");
      temp30 = temperatureF0;
      temperatureFavg0 = (temp10 + temp20 + temp30) / 3;

      temp31 = temperatureF1;
      temperatureFavg1 = (temp11 + temp21 + temp31) / 3;

      temp32 = temperatureF2;
      temperatureFavg2 = (temp12 + temp22 + temp32) / 3;
      looper = 0;
    }
    //Serial.println(temperatureFavg0);
    //Serial.println(temperatureFavg1);
    //Serial.println(temperatureFavg2);

    temperatureFavg = (temperatureFavg0 + temperatureFavg1 + temperatureFavg2) / 3;

    //Serial.print(temperatureFavg); Serial.println(" degrees F");
    //Serial.println();
//Print to LCD
    lcd.setCursor(0, 0);
    lcd.print("Set Temp: "); lcd.print(setTemp); lcd.print("F");
    lcd.setCursor(0, 1);
    lcd.print("Temp: "); lcd.print(temperatureFavg); lcd.print("F");

//Send Data over Serial
    Serial1.print(temperatureFavg); Serial1.print(','); Serial1.print(setTemp); Serial1.print(','); Serial1.print(RPM); Serial1.println();
   //erial.print(temperatureFavg); Serial.print(','); Serial.print(RPM); Serial.println(); 

   if (Serial1.available()>0){
    String tempWord = Serial1.readStringUntil('\n');
    setTemp = tempWord.toInt();
   }
  delay(1000);
  }
}


void rpm_fun()
{
  // revolutions++;
  //Each rotation, this interrupt function is run 7 times

  //read the current time
  long time = micros();

  //alternate which time store to use
  if (timeStoreSwitch) {
    timeStoreA = time;
    timeStoreSwitch = false;
  }

  else {
    timeStoreB = time;
    timeStoreSwitch = true;
  }
}
//-----------------------------------------------
