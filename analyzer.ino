#include <Adafruit_ADS1015.h>
#include <Wire.h>
#include <LiquidCrystal.h>

Adafruit_ADS1015 ads1015;
LiquidCrystal display(12, 11, 5, 4, 3, 2);


int button1pin=3;//Calibration, Enter Button

//General Calibration setup
//double  calibrationv; //used to store calibrated value
double calbrationPre, calibrationPost, calibrationPressure;
int sensorcheck=0;//to check health on sensor. If value is 0 sensor works, if value is 1 sensor out of range or not connected
int Sensor_lowrange=58;//When sensor is healthy and new it reads 58 on low
int Sensor_highrange=106;//When sensor is healthy and new it reads 106 on high
int current_function=0;

void setup(void)
{
  Serial.begin(9600);
  pinMode(button1pin,INPUT);

    //LCD Setup
  display.begin(20, 4);  //20 character, 4 line display
  display.print("Analyzer V1");

  //starts ADS readings
  ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  ads.begin();

  display.print("Calibrating")
  calibratePreCompressorSensor();
  calibratePostCompressorSensor();
  zeroPressureSensor();
  checkCalibration();

}
//Checks calibration
void checkCalibration()
{
  if ((calibrationPre > Sensor_highrange) || (calibrationPre < Sensor_lowrange))
  {
    sensorcheck=1;
    current_function=1;//Sensor needs to be calibrated
    need_calibrating("Pre");//print need calibrating message
   }
  if ( || (calibrationPost > Sensor_highrange) || (calibrationPost < Sensor_lowrange))
  {
    sensorcheck=1;
    current_function=1;//Sensor needs to be calibrated
    need_calibrating("Post");//print need calibrating message
   }
}
//Prints need calibrating text
void need_calibrating(String sensor)
{
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Sensor error");
  display.println("Please");
  display.println("calibrate " + sensor);
  display.println(calibrationv);

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
  double resultPre;//After calculations holds the current O2 percentage
  double resultPost;//After calculations holds the current O2 percentage
  double resultPressure;//After calculations holds the current O2 percentage
  double currentmvPre; //the current mv put out by the oxygen sensor;
  double currentmvPost; //the current mv put out by the oxygen sensor;
  double currentPressure; //the current mv put out by the oxygen sensor;
  double calibratev;

  //***** Function button read section ********
  int button1state = digitalRead(button1pin);
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
      currentmvPre = analyzePreSensor()/20;
      resultPre = (currentmvPre/calibrationv)*20.9;
      currentmvPre = analyzePostSensor()/20;
      resultPost = (currentmvPost/calibrationv)*20.9;
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
      calibrationPre=calibratePreCompressorSensor();
      calibrationPost=calibratePostCompressorSensor();
      calibrationPressure = calibratePressureSensor();
      delay(2000);
      checkCalibration();
      break;
   }
}



