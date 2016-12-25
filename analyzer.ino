#include <Adafruit_ADS1015.h>
#include <Wire.h>
#include <LiquidCrystal.h>
 
Adafruit_ADS1015 ads1015;
LiquidCrystal display(12, 11, 5, 4, 3, 2);

 
int button1pin=3;//Calibration, Enter Button

//General Calibration setup
double  calibrationv; //used to store calibrated value
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
  
  
  

  if ((calibrationv > Sensor_highrange) || (calibrationv < Sensor_lowrange))
   {
    sensorcheck=1;
     current_function=1;//Sensor needs to be calibrated
     need_calibrating();//print need calibrating message
   } 
}

//Prints need calibrating text
void need_calibrating(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Sensor error");
  display.println("Please");
  display.println("calibrate"); 
  display.println(calibrationv);
  
}
int readADC(int adcChannel){
	return ads.readADC_SingleEnded(adcChannel);
}
//Take 20 readings and avaraging it to exclude minor deviations of the reading
int calibratePreCompressorSensor(){
  int16_t adc0=0;
  int result;
  for(int i=0; i<=19; i++)
       {
         adc0=adc0+readADC(0);
       }
  
  result=adc0/20;
  return result;
}
//Calibrate sensor which is on the exhaust side of the compressor
int calibratePostCompressorSensor(){
  int16_t adc1=0;
  int result;
  for(int i=0; i<=19; i++)
       {
         adc1=adc1+readADC(1);
       }
  
  result=adc1/20;
  return result;
}
//Calibrate the Pressure sensor with known gas source(Zero pressure in tanks when initial boot)
int zeroPressureSensor(){
  int16_t adc2=0;
  int result;
  for(int i=0; i<=19; i++)
       {
         adc2=adc2+readADC(2);
       }
  
  result=adc2/20;
  return result;
}

int analyzePreSensor(){
    int16_t adc0=0;  //Pre-compression o2 sensor
    //taking 20 samples. The sensor might spike for a millisecond. After we average the samples into one value
     for(int i=0; i<=19; i++)
       {
         adc0=adc0+ads.readADC_SingleEnded(0);
       }
	return adc0;            
}

int analyzePostSensor(){
    int16_t adc1=0;  //Pre-compression o2 sensor
    //taking 20 samples. The sensor might spike for a millisecond. After we average the samples into one value
     for(int i=0; i<=19; i++)
       {
         adc1=adc1+ads.readADC_SingleEnded(1);
       }
	return adc1;            
}

int analyzePressure(){
    int16_t adc2=0;  //Pre-compression o2 sensor
    //taking 20 samples. The sensor might spike for a millisecond. After we average the samples into one value
     for(int i=0; i<=19; i++)
       {
         adc2=adc2+ads.readADC_SingleEnded(2);
       }
	return adc2;            
}

void loop(void) {
  //********  Main Loop variable declaration *********** 
     double modr;//Variable to hold mod value in
    int16_t adc1=0;  //Post-compression o2 sensor
    int16_t adc2=0;  //Pressure Sensor
    
    double resultPre;//After calculations holds the current O2 percentage
    double resultPost;//After calculations holds the current O2 percentage
    double resultPressure;//After calculations holds the current O2 percentage        
    
    double currentmvPre; //the current mv put out by the oxygen sensor;
    double currentmvPost; //the current mv put out by the oxygen sensor;
    double currentPressure; //the current mv put out by the oxygen sensor;        
    double calibratev;
 
 //***** Function button read section ********  
 int button1state=digitalRead(button1pin);
  if(button1state==LOW){
    if(current_function==0){
       current_function=1;//Sensor needs to be calibrated
    }
  }
  	 
 
 switch(current_function){
   case 0:
     
      currentmvPre = analyzePreSensor()/20;
      resultPre = (currentmvPre/calibrationv)*20.9;
      currentmvPre = analyzePostSensor()/20;
      resultPost = (currentmvPost/calibrationv)*20.9;
      resultPressure = analyzePressure();
      //Write to display
      display.clear();
      display.setCursor(0,0);
      display.print("O2% and Pressure");
      display.setCursor(0,1);
      display.print("Pre: " + String(resultPre));
      display.setCursor(0,2);
      display.print("Post: " + String(resultPost));      
      display.setCursor(0,3);
	  display.print("Pressure: " + String(resultPressure));
      delay(5000); //retest pressure and o2% every 5 seconds
      break;

   case 1:     
     display.clear();
     display.println("Calibrating");
     display.display();
     
     current_function=0;//O2 analyzing
     calibrationv=calibrate();
     delay(2000);
      if ((calibrationv > Sensor_highrange) || (calibrationv < Sensor_lowrange)){
          current_function=1;//Sensor needs to be calibrated
          need_calibrating();//print need calibrating message
        } 
     break;
 }
 
}
 


