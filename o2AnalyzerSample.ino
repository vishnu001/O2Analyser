//ADS1050 12 bit ADC
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
 
Adafruit_ADS1115 ads;     /* Use thi for the 12-bit version */
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Button and backlight pin declaration
int button1pin=3;//Calibration, Enter Button

//General Cariable setup
double  calibrationv; //used to store calibrated value
double calibrationvCo2;
int sensorcheck=0;//to check health on sensor. If value is 0 sensor works, if value is 1 sensor out of range or not connected
int Sensor_lowrange=104;//When sensor is healthy and new it reads 58 on low
int Sensor_highrange=128;//When sensor is healthy and new it reads 106 on high
int current_function=0;
unsigned int error = 0;

void setup(void) 
{
 
    
  
  Serial.begin(9600);
  pinMode(button1pin,INPUT);
   
  
//starts ADS readings
 ads.setGain(GAIN_ONE);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  
  ads.begin();

  Serial.println("calibrating sensor, wait for 10 seconds");
  calibrationv=calibrate();
  calibrationvCo2=calibrateCo2(); //co2 calibration
  
  //Nokia Display Setup
  display.begin();
   
  
  if ((calibrationv > Sensor_highrange) || (calibrationv < Sensor_lowrange))
   {
    sensorcheck=1;
    error = 1; // use this error flag to avoid malfunctioning
     current_function=1;//Sensor needs to be calibrated
     need_calibrating();//print need calibrating message
   } 
}

//Prints need calibrating text
void need_calibrating(){
   display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.println("Sensor error");
  display.println("Please");
  display.println("calibrate"); 
  display.println(calibrationv);
  display.display();
  
}

//Take 20 readings and avaraging it to exclude miner diviations of hte reading
int calibrate(){
  int16_t adc0=0;
  int result;
  for(int i=0; i < 20; i++)
       {
         adc0=adc0+ads.readADC_SingleEnded(0);
         delay(200);
       }
  
  result=adc0/20;
  return result;
}

//Take 20 readings and avaraging it to exclude miner diviations of hte reading
int calibrateCo2(){
  int16_t adc1=0;
  int result;
  for(int i=0; i<40; i++)
       {
         adc1=adc1+ads.readADC_SingleEnded(1);
         delay(250);
       }
  
  result=adc1/40;
  return result;
}

void loop(void) {
  //********  Main Loop variable declaration *********** 
     double modr;//Variable to hold mod value in
    int16_t adc0=0, adc1=0;
    double result;//After calculations holds the current O2 percentage
    double currentmv; //the current mv put out by the oxygen sensor;
    double currentvCo2;
    double co2Conc;
    double calibratev;
 
 //***** Function button read section ********  
 int button1state=digitalRead(button1pin);

 
 
  if(button1state==LOW){
    if(current_function==0){
       current_function=1;//Sensor needs to be calibrated
    }
  }
 
 
 switch(current_function){
   case 0://Analyzing O2
     //taking 20 samples. The sensor might spike for a millisecond. After we average the samples into one value
     for(int i=0; i<=19; i++)
       {
         adc0=adc0+ads.readADC_SingleEnded(0);
         adc1=adc1+ads.readADC_SingleEnded(1);
       }
      
      
      currentmv = adc0/20;
      currentvCo2 = adc1/40;
      calibratev=calibrationv;
      result=(currentmv/calibratev)*20.9;
      co2Conc = ((currentvCo2/calibrationvCo2)/3.3)*100; // change 3.3 with the voltage input to co2 sensor
      //Write to display
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE, BLACK);
      display.setCursor(0,0);
      display.print("O2%");
      display.setTextColor(SSD1306_WHITE);
      display.print(" ");
      display.setTextSize(2);
      display.println(result,1);
      display.setCursor(0,45);
      display.print("CO2%");
      display.setTextColor(SSD1306_WHITE);
      display.print(" ");
      display.setTextSize(2);
      display.println(co2Conc,1);
      display.display();
           delay(1000);
    break;
   case 1:
     
     display.clearDisplay();
     display.setCursor(0,0);
     display.setTextSize(1);
     display.setTextColor(SSD1306_WHITE);
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
 
