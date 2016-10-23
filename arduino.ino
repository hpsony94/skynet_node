/*Thesis 2016-2017
Author: Huynh Pham So Ny - 51202655*/
#include "MQ135.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include <SoftwareSerial.h>
#include "TimerOne.h"

//Bao nhieu lan pushData la duoc khi send_error=1???
#define TIMEON 5
#define TIMEOFF 10
#define ONE_WIRE_BUS 4
#define MQ135_PIN A0
#define MQ7_PIN A1
#define RX_SIM_OUT 11
#define TX_SIM_OUT 10
#define DUST_OUT A2
#define DUST_PIN 2




int A0Value = 0;
int A1Value = 0;
float VRes = 0;
float VPPM = 0;
float VRZero = 0;
float TempValue = 0;
long count = 0;

float VRDust = 0;
float VDust = 0;
float VVoltDust = 0;
int initialTime = 280;
int deltaTime = 40;
int sleepTime = 9680;

char buf[32];
String StrAir;
String StrCO;
String StrTemp;
String StrDust;
//Thinkspeak API Key
//String apiKey = "G1JJIY5JTMO7MLXE";
String apiKey = "1";
int send_error=0;

int minute = 0;



SoftwareSerial sim900(RX_SIM_OUT, TX_SIM_OUT);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature temp_sensor(&oneWire);
MQ135 mq135 = MQ135(MQ135_PIN);


void setup(){
  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
  digitalWrite( 3, HIGH );
  Serial.begin(9600);
  sim900.begin(9600);
  Serial.println("SIM 900 already!");
  //  Start temperture sensor
  temp_sensor.begin();
  //  Reset Module SIM900A
  sim900.println("AT+RST");
  // Initialize the timer 1
  Timer1.initialize(1000000); // ~ 1s
  Timer1.attachInterrupt(timerIsr);
}

void loop(){
  if(minute == TIMEON)
  { 
    VRes = mq135.getResistance();
    VPPM = mq135.getPPM();
    VRZero = mq135.getRZero();
    A0Value = analogRead(MQ135_PIN);
    A1Value = analogRead(MQ7_PIN);
    temp_sensor.requestTemperatures(); // Send the command to get temperatures
    getDust();
    TempValue = temp_sensor.getTempCByIndex(0);
    Serial.print("\n\r \n\r Pushing to server ");
    Serial.print(".");
    delay(100);
    Serial.print(".");
    delay(100);
    Serial.print(".");
    delay(100);
    Serial.print(".");
    delay(100);
    Serial.print(".");
    delay(100);
    Serial.print(".");
    delay(100);
    Serial.print(".");
    delay(100);
    Serial.print(".");
    delay(100);
    Serial.print(".");
    delay(100);
    Serial.print(".");
    delay(100);
    Serial.print(".");
    delay(100);
    Serial.print(". ");
    pushData();
    digitalWrite( 3, LOW );
    minute++;
  }
  if(send_error==1) {
    Serial.println("Try it again");
    pushData();
   }
  delay(9000);
}



void getDust(){
  digitalWrite(DUST_PIN,LOW); //Turn on the LED
  delayMicroseconds(initialTime);
  VRDust = analogRead(DUST_OUT); //Read the dust value
  delayMicroseconds(deltaTime);
  digitalWrite(DUST_PIN,HIGH);
  VVoltDust = VRDust * (3.3 / 1024);
  VDust = 0.17 * (VVoltDust) - 0.1;
}


void timerIsr(){
  count ++;
  if(count == 60){
    minute ++;
    count = 0;
  }
  if(minute == (TIMEOFF+TIMEON))
  {
    digitalWrite( 3, HIGH );
    minute = 0;
    count = 0;
  }
}

void pushData(){
  // convert to string
  StrAir = dtostrf( VPPM, 3, 1, buf);
  StrCO = dtostrf( A1Value, 4, 1, buf);
  StrTemp = dtostrf( TempValue, 5, 2, buf);
  StrDust = dtostrf( VVoltDust, 3, 2, buf);
  //TCP Connection
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "http://www.codingyourfuture.com"; // api.thingspeak.com
  cmd +="\",80"; //Connection with port 80
  sim900.println(cmd);
  delay(4000);

  if(sim900.find("Error")){
    Serial.println("AT+CIPSTART error");
    return;
  }
  delay(2000);
  // prepare GET string
  String getStr = "GET /add?node=";
  getStr += apiKey;
  getStr +="&s1=";
  getStr += String(StrCO);
  getStr +="&s2=";
  getStr += String(StrTemp);
  getStr +="&s3=";
  getStr += String(StrDust);
  getStr +="&s4=";
  getStr += String(StrAir);
  getStr += "\r\n\r\n";
  Serial.println(getStr);
  // send data length
  cmd = "AT+CIPSEND="; 
  cmd += String(getStr.length());
  sim900.println(cmd);
  delay(2000);
  if(sim900.find(">")){
    sim900.print(getStr);
    send_error= 0 ;
    Serial.println("DONE!!!");
  }
  else
  {
    sim900.println("AT+CIPCLOSE");
    // alert user
    Serial.println("Send error");
    send_error = 1;
  }
   sim900.println("AT+CIPCLOSE");
}



