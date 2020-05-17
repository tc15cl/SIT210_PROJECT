// Library includes for temp, MQTT & UV
#include <DS18B20.h>
#include <Adafruit_SI1145.h>
#include "MQTT.h"

//MQTT initialise
MQTT client("test.mosquitto.org", 1883, callback);

void callback(char* topic, byte* payload, unsigned int length) 
{
}


//constants to define pinouts
const int VALVEPIN = D8; 
const int TEMPPIN = A4;
const int MAXRETRY = 4;
const int MOIST1PIN = A1;
const int MOIST2PIN = A2;
const int FLOWPIN = A0;

//variables for flowmeter
volatile int NbTopsFan; //measures rising edge of flowmeter squarewave
int flowCalc;
int hallsensor = A0;
bool VALVE1;
bool VALVE2;

//soil moisture variables
const int AirValue = 3000;   //calibration value for open air
const int WaterValue = 1850;  //calibration value for water
int soilMoistureValue1 = 0;
int soilMoistureValue2 = 0;
int intervals = (AirValue - WaterValue) /3; //hystersis interval 
String moist1;
String moist2;

//object for temperature
DS18B20 ds18b20(TEMPPIN, true);
double celsius;

//object for UV
Adafruit_SI1145 uv = Adafruit_SI1145();
float UVindex;

//watchdog pointer
ApplicationWatchdog *wd;


void setup() {

  //MQTT connect
  client.connect("photonDev");

  //application watchdog initialisation
  wd = new ApplicationWatchdog(60000, System.reset, 1536);
  
  pinMode(FLOWPIN, INPUT); //initializes A0 as an input
  attachInterrupt(FLOWPIN, rpm, RISING); //attach interrupt to A0

  pinMode(VALVEPIN, OUTPUT); //initializes D8 as output

  uv.begin(); //start uv
}



void loop() {

  if (client.isConnected()){
      //call individual program functions
      getUV();
      getTemp(); 
      getFlow();
      getSoil();
      publishData();
      wd->checkin(); //watchdog checkin
      client.loop();
    }
}

void rpm ()     //Interrupt call function
{
    NbTopsFan++;  //This function measures the rising and falling edge of the flow sensor
}


//function for flowmeter
void getFlow(){
  NbTopsFan = 0;   //Set NbTops to 0 ready for calculations
  interrupts();      //Enable interrupt
  delay (1000);   
  noInterrupts();      //Disable interrupt
  flowCalc = (NbTopsFan * 60); //(Pulse frequency x 60) / = flow rate in L/hour   
}


//function for temperature
void getTemp(){

  //local variables
  float _temp;
  int   i = 0;

  //whilst temp sesnor returns valid reading keep going
  do 
  {
    _temp = ds18b20.getTemperature();
  } while (!ds18b20.crcCheck() && MAXRETRY > i++);

  if (i < MAXRETRY)
  {
    celsius = _temp;
  }
  else 
  {
    celsius = NAN;
  }
}


//function for soil moisture calculation and control of water valve
void getSoil(){

  //read soil moisture from each sensor
  soilMoistureValue1 = analogRead(MOIST1PIN);
  soilMoistureValue2 = analogRead(MOIST2PIN);

  //take average of each sensor
  int soilAvg = ((soilMoistureValue1+soilMoistureValue2)/2);


  //VERY WET if soil sensor more than water calibrated value
  if(soilAvg > WaterValue && soilAvg < (WaterValue+intervals)) 
  {
    moist1 = ("Very Wet");
    digitalWrite(VALVEPIN, LOW);
    VALVE1 = false;
  }
  //WET if soil sensor more than water calibrated value and less than ait calibrated value
  else if(soilAvg > (WaterValue+intervals) && soilAvg < (AirValue-intervals)) 
  {
    moist1 = ("Wet");
    digitalWrite(VALVEPIN, LOW);
    VALVE1 = false;
  }
  //DRY if soil sensor less than water calibrated value and more than air calibrated value
  else if(soilAvg  < AirValue && soilAvg > (AirValue-intervals))
  {
    moist1 = ("Dry");

    //if temp & UV safe - switch water valve on
    if(celsius < 30 && UVindex < 3)
    {
      digitalWrite(VALVEPIN, HIGH);
      VALVE1 = true;
    }
  }
  //VERY DRY if soil sensor more than air calibrated value
  else if(soilAvg  > AirValue)
  {
    moist1 = ("Very Dry");

    //if temp & UV safe - switch water valve on
    if(celsius < 30 && UVindex < 4)
    {
      digitalWrite(VALVEPIN, HIGH);
      VALVE1 = true;
    }
  }

}

//function for UV
void getUV(){
    UVindex = uv.readUV();
}


//funciton for publishing data to MQTT
void publishData(){
  
  client.publish( "Temp", String(celsius));
  client.publish( "Soil", String(moist1));
  client.publish( "UVIdx", String(UVindex));
  client.publish( "Flow", String(flowCalc));
  client.publish("Valve", String(VALVEPIN));
  delay(2000);
} 






