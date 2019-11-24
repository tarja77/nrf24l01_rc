/*
 Copyright (C) 2011 James Coliz, Jr. <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */
#include <Wire.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2); 

const int X1Pin = 10;
const int Y1Pin = 9;
const int X2Pin = 6;
const int Y2Pin = 5;
const int APin = 3;
const int S1Pin = 4;
const int S2Pin = 2;


Servo EX1Pin; 
Servo EY1Pin; 
Servo EX2Pin; 
Servo EY2Pin; 
Servo EAPin; 


unsigned long tempM;
typedef struct{
   unsigned long M;
  word X1;
  word Y1;
  word X2;
  word Y2;
  word A;
  boolean S1;
  boolean S2;
}
TX; 
TX tx;


  typedef struct{
 unsigned long M;
}RX;
RX rx;

int ping;


RF24 radio(7,8);
boolean Pressed=false;
boolean Timeout=true;;
int counter;
boolean role;
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };



void Tick()
{
  char kml[20];
unsigned long Time = millis();
static unsigned long TimePrev = Time;
static unsigned long Interval1 = 0;// 1. timer
static unsigned long Interval2 = 0;// 1. timer
Interval1 += Time - TimePrev;
Interval2 += Time - TimePrev;


  if(Interval1 > 500){

    if(Timeout){
      lcd.clear();
     lcd.setCursor(2, 1);
lcd.print("TimeOut");
    }else{
       lcd.clear();
    sprintf(kml,"Ping: %d Cn: %d",tempM,counter);
 lcd.setCursor(1, 1);
 lcd.print(kml);
      
    }


 Interval1 = 0;
   
  }
  if(Interval2 > 100){

  if (digitalRead(4)){
    if(!Pressed){
      counter++;   
      Pressed=true;
    }
  }else{

    Pressed=false;
  }

 Interval2 = 0;
   
  }
TimePrev = Time;
 }

void setup(void)
{


    role = false;
 
  //Serial.begin(115200);


radio.begin();
  if ( role)
  {
pinMode(4,INPUT);
pinMode(5,INPUT);
pinMode(6,INPUT);

  lcd.begin();

  // Turn on the blacklight and print a message.
  lcd.backlight();
  radio.begin();

    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
  }
  else
  {


pinMode(X1Pin,OUTPUT);
    pinMode(Y1Pin,OUTPUT);
    pinMode(X2Pin,OUTPUT);
    pinMode(Y2Pin,OUTPUT);
    pinMode(APin,OUTPUT);
      pinMode(S1Pin,OUTPUT);
        pinMode(S2Pin,OUTPUT);

 EX1Pin.attach(X1Pin,1000,2000);
 EY1Pin.attach(Y1Pin,1000,2000);
EX2Pin.attach(X2Pin,1000,2000);
  EY2Pin.attach(Y2Pin,1000,2000); 
 EAPin.attach(APin,1000,2000);

       
    radio.openWritingPipe(pipes[1]);
    radio.openReadingPipe(1,pipes[0]);
  }
  radio.startListening();
  radio.printDetails();

}

void loop(void)
{




  if (role)
  {
    Tick();
    radio.stopListening();

    // Take the time, and send it.  This will block until complete
    unsigned long time = millis();
    printf("Now sending %lu...",time);

 tx.M = time;
  tx.X1 = analogRead(A0);
  tx.Y1 = analogRead(A1);
 tx.X2 = analogRead(A2);
 tx.Y2 = analogRead(A3);
 tx.A = analogRead(A6);
  tx.S1=digitalRead(5);
 tx.S2=digitalRead(6);


    
    radio.write( &tx, sizeof(tx) );

    // Now, continue listening
    radio.startListening();

    // Wait here until we get a response, or timeout (250ms)
    unsigned long started_waiting_at = millis();
    bool timeout = false;
    while ( ! radio.available() && ! timeout )
      if (millis() - started_waiting_at > 200 )
        timeout = true;

    // Describe the results
    if ( timeout )
    {
Timeout=true;
  
    }
    else
    {
      Timeout=false;
      // Grab the response, compare, and send to debugging spew
      unsigned long got_time;
      radio.read( &rx, sizeof(rx) );


if(tempM!=millis()-rx.M){

 tempM=millis()-rx.M;

}
      // Spew it
      printf("Got response %lu, round-trip delay: %lu\n\r",rx.M,millis()-rx.M);
  
    }

    // Try again  later
    delay(20);
  }

  //
  // Pong back role.  Receive each packet, dump it out, and send it back
  //

  if (!role)
  {

    // if there is data ready
    if ( radio.available() )
    {

      while (radio.available())
      {
        // Fetch the payload, and see if this was the last one.
        radio.read(&tx, sizeof(tx) );
      }
	// Delay just a little bit to let the other unit
	// make the transition to receiver
	//delay(20);
      //}

      // First, stop listening so we can talk
      radio.stopListening();

 EX1Pin.write(map(tx.X1, 0, 1023, 0, 180));
      EY1Pin.write(map(tx.Y1, 0, 1023, 0, 180)); 
       EX2Pin.write(map(tx.X2, 0, 1023, 160,20)); 
             EY2Pin.write(map(tx.Y2, 0, 1023, 0, 180)); 
             EAPin.write(map(tx.A, 0, 1023, 0, 180)); 
      /*
  Serial.print(tx.X1);
  Serial.print("!");
  Serial.print(tx.Y1);
  Serial.print("!");
  Serial.print(tx.X2);
  Serial.print("!");;
  Serial.print(tx.Y2);
  Serial.print("!");
  Serial.print(tx.A);
  Serial.print("!");
  Serial.print(tx.S1);
  Serial.print("!");
  Serial.println(tx.S2);
  */
      // Send the final one back.
rx.M =tx.M;


      
      radio.write( &rx, sizeof(rx) );

      // Now, resume listening so we catch the next packets.
      radio.startListening();
    //  printf("Sent response.\n\r");

    }
  }
 

}
// vim:cin:ai:sts=2 sw=2 ft=cpp
