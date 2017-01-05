#include <Adafruit_ADS1015.h>
#include <Wire.h>
#include <LiquidCrystal.h>

Adafruit_ADS1015 ads;
LiquidCrystal display(12, 11, 5, 4, 3, 2);


const int buzzerPin = 9; //buzzer to arduino pin 9

//General Calibration setup
double calbrationPre, calibrationPost, calibrationPressure;
int Sensor_lowrange = 58;//When sensor is healthy and new it reads 58 on low
int Sensor_highrange = 106;//When sensor is healthy and new it reads 106 on high
int current_function = 0;
int pressureZero = 0;

double resultPre;//After calculations holds the current O2 percentage
double resultPost;//After calculations holds the current O2 percentage
double resultPressure;//After calculations holds the current O2 percentage


void setup(void)
{  
  pinMode(buzzerPin, OUTPUT); // Set buzzer - pin 9 as an output
  Serial.begin(9600);
    //LCD Setup
  display.begin(20, 4);  //20 character, 4 line display
  display.setCursor(0, 0);
  display.print("Analyzer V1");
  Serial.println("Analyzer V1");

  //starts ADS readings
//  ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  ads.begin();
  display.setCursor(0, 1);
  Serial.println("Calibrating");
  display.print("Calibrating");
  checkCalibration(calibratePreCompressorSensor(),   calibratePostCompressorSensor(), zeroPressureSensor());
  Serial.println("calibrated");
}
//Checks calibration
void checkCalibration(int calPre, int calPost, int calPress)
{
  delay(6000);  //delay since JJ sensors take 6 seconds to calibrate
//  if ((calPre > Sensor_highrange) || (calPre < Sensor_lowrange))
//  {
//    current_function=1;//Sensor needs to be calibrated
//    need_calibrating("Pre", calPre);//print need calibrating message
//   }
//  if ((calPost > Sensor_highrange) || (calPost < Sensor_lowrange))
//  {
//    current_function=1;//Sensor needs to be calibrated
//    need_calibrating("Post", calPost);//print need calibrating message
//   }
  if((calPress < 90) || (calPress > 140))
  {
    current_function = 1;
    need_calibrating("Pressure", calPress);
  }
  else
  {
    pressureZero = calPress; 
  }
}
//Prints need calibrating text
void need_calibrating(String sensor, double calibrationValue)
{
  display.clear();
  display.setCursor(0,0);
  display.print("Sensor error");
  display.setCursor(0, 1);
  display.print("Please");
  display.setCursor(0, 2);
  display.print("calibrate " + sensor);
  display.setCursor(0, 3);
  display.print(calibrationValue);
  current_function = 2;

}
int readADC(int adcChannel)
{
  return ads.readADC_SingleEnded(adcChannel);
}
//Calibrate a sensor
////Take 20 readings and avarage it to exclude minor deviations of the reading
int calibrateSensor(int sensorID)
{
  int16_t adc = sensorID;
  int result;
  for(int i = 0; i <= 19; i++)
  {
    adc = adc + readADC(sensorID);
  }

  result=adc / 20;
  return result;
}
//Calibrate the sensor that is pre-compression
int calibratePreCompressorSensor()
{
  //return calibrateSensor(0);
  return 0;
}
//Calibrate sensor which is on the exhaust side of the compressor
int calibratePostCompressorSensor()
{
  //return calibrateSensor(1);
  return 0;
}
//Calibrate the Pressure sensor with known gas source(Zero pressure in tanks when initial boot)
int zeroPressureSensor()
{
  return analogRead(A5);
}

int analyzePreSensor()
{
  return 0;
  //return readADC(0);
}

int analyzePostSensor()
{
  return 0;
//  return readADC(1);
}

int analyzePressure()
{
  int pressureRMV = analogRead(A5);
  double PSI = (map(pressureRMV,pressureZero,1023,0,4500));
  return PSI;
}
int printAnalysisData()
{
  display.clear();
  delay(1000);
  display.setCursor(0, 0);
  display.print("O2% and Pressure");
  display.setCursor(0, 1);
//  display.print("Pre: " + String(resultPre));
  display.setCursor(0, 2);
//  display.print("Post: " + String(resultPost));
  display.setCursor(0, 3);
  display.print("Pressure: " + String(resultPressure));
  Serial.println("O2% and Pressure");
//  Serial.println("Pre: " + String(resultPre));
//  Serial.println("Post: " + String(resultPost));
  Serial.println("Pressure: " + String(resultPressure));
}
void loop(void)
{
  //********  Main Loop variable declaration ***********
  double currentmvPre; //the current mv put out by the oxygen sensor;
  double currentmvPost; //the current mv put out by the oxygen sensor;
  double currentPressure; //the current mv put out by the oxygen sensor;

  switch(current_function)
  {
    case 0: //Analyze Gas
      Serial.println("in case 0");
      Serial.println("resultPre");
      currentmvPre = analyzePreSensor();      
      resultPre = (currentmvPre/calbrationPre)*20.9;
      Serial.println("resultPost");
      currentmvPre = analyzePostSensor();
      resultPost = (currentmvPost/calibrationPost)*20.9;
      Serial.println("about to read Pressure");
      resultPressure = analyzePressure();
      //Write to display
      Serial.println("about to print");
      printAnalysisData();
      delay(5000); //retest pressure and o2% every 5 seconds
      break;

    case 1:  //Calibrate sensor
    Serial.println("in case 1");
      display.clear();
      display.setCursor(0, 1);
      display.print("Calibrating");
      Serial.println("in case 1, Calibrating");

      current_function=0;//O2 analyzing
      checkCalibration(calibratePreCompressorSensor(),   calibratePostCompressorSensor(), zeroPressureSensor());
      break;

    case 2:  //Error buzzer
    Serial.println("in case 2");
      tone(buzzerPin, 1000); // Send 1KHz sound signal...
      delay(1000);        // ...for 1 sec
      noTone(buzzerPin);     // Stop sound...
      delay(1000);        // ...for 1sec
      break;
   }
}
