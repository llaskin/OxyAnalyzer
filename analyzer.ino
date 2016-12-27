#include <Adafruit_ADS1015.h>
#include <Wire.h>
#include <LiquidCrystal.h>

Adafruit_ADS1015 ads;
LiquidCrystal display(12, 11, 5, 4, 3, 2);


const int buttonPin =3;//Calibration, Enter Button
const int buzzerPin = 9; //buzzer to arduino pin 9

//General Calibration setup
double calbrationPre, calibrationPost, calibrationPressure;
int Sensor_lowrange=58;//When sensor is healthy and new it reads 58 on low
int Sensor_highrange=106;//When sensor is healthy and new it reads 106 on high
int current_function=0;

double resultPre;//After calculations holds the current O2 percentage
double resultPost;//After calculations holds the current O2 percentage
double resultPressure;//After calculations holds the current O2 percentage


void setup(void)
{
  Serial.begin(9600);
  pinMode(buttonPin,INPUT);
  pinMode(buzzerPin, OUTPUT); // Set buzzer - pin 9 as an output

    //LCD Setup
  display.begin(20, 4);  //20 character, 4 line display
  display.print("Analyzer V1");

  //starts ADS readings
  ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  ads.begin();

  display.print("Calibrating");
  checkCalibration(calibratePreCompressorSensor(),   calibratePostCompressorSensor(), zeroPressureSensor());

}
//Checks calibration
void checkCalibration(int calPre, int calPost, int calPress)
{
  delay(10000);  //delay since JJ sensors take 6 seconds to calibrate
  if ((calPre > Sensor_highrange) || (calPre < Sensor_lowrange))
  {
    current_function=1;//Sensor needs to be calibrated
    need_calibrating("Pre", calPre);//print need calibrating message
   }
  if ((calPost > Sensor_highrange) || (calPost < Sensor_lowrange))
  {
    current_function=1;//Sensor needs to be calibrated
    need_calibrating("Post", calPost);//print need calibrating message
   }
  if(calPress > 100)
  {
    current_function = 1;
    need_calibrating("Pressure", calPress);
  }
}
//Prints need calibrating text
void need_calibrating(String sensor, double calibrationValue)
{
  display.clear();
  display.setCursor(0,0);
  display.println("Sensor error");
  display.println("Please");
  display.println("calibrate " + sensor);
  display.println(calibrationValue);
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
  return calibrateSensor(0);
}
//Calibrate sensor which is on the exhaust side of the compressor
int calibratePostCompressorSensor()
{
  return calibrateSensor(1);
}
//Calibrate the Pressure sensor with known gas source(Zero pressure in tanks when initial boot)
int zeroPressureSensor()
{
  return calibrateSensor(2);
}

int analyzePreSensor()
{
  return readADC(0);
}

int analyzePostSensor()
{
  return readADC(1);
}

int analyzePressure()
{
  return readADC(2);
}
int printAnalysisData()
{
  display.clear();
  display.println("O2% and Pressure");
  display.println("Pre: " + String(resultPre));
  display.println("Post: " + String(resultPost));
  display.println("Pressure: " + String(resultPressure));
}
void loop(void)
{
  //********  Main Loop variable declaration ***********
  double currentmvPre; //the current mv put out by the oxygen sensor;
  double currentmvPost; //the current mv put out by the oxygen sensor;
  double currentPressure; //the current mv put out by the oxygen sensor;

  //***** Function button read section ********
  int button1state = digitalRead(buttonPin);
  if(button1state == LOW)
  {
    if(current_function == 0)
    {
      current_function = 1;//Sensor needs to be calibrated
    }
  }

  switch(current_function)
  {
    case 0: //Analyze Gas
      currentmvPre = analyzePreSensor();
      resultPre = (currentmvPre/calbrationPre)*20.9;
      currentmvPre = analyzePostSensor();
      resultPost = (currentmvPost/calibrationPost)*20.9;
      resultPressure = analyzePressure();
      //Write to display
      display.clear();
      printAnalysisData();
      delay(5000); //retest pressure and o2% every 5 seconds
      break;

    case 1:  //Calibrate sensor
      display.clear();
      display.println("Calibrating");
      display.display();

      current_function=0;//O2 analyzing
      checkCalibration(calibratePreCompressorSensor(),   calibratePostCompressorSensor(), zeroPressureSensor());
      break;

    case 2:  //Error buzzer
      tone(buzzerPin, 1000); // Send 1KHz sound signal...
      delay(1000);        // ...for 1 sec
      noTone(buzzerPin);     // Stop sound...
      delay(1000);        // ...for 1sec
      break;
   }
}