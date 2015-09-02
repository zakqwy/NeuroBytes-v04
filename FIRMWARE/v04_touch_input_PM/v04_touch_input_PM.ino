/*
  September 1, 2015
 Zach Fredin
 NeuroBytes v04 Pro Mini touch input
 */

#include <CapacitiveSensor.h>

CapacitiveSensor cs_7_6 = CapacitiveSensor(7,6);
CapacitiveSensor cs_7_5 = CapacitiveSensor(7,5);
CapacitiveSensor cs_7_4 = CapacitiveSensor(7,4);
CapacitiveSensor cs_7_3 = CapacitiveSensor(7,3);
CapacitiveSensor cs_7_2 = CapacitiveSensor(7,2);

void setup()
{
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
}

void loop()
{
  if (cs_7_6.capacitiveSensor(3) > 20) 
  {
    digitalWrite(10, HIGH);
  }
  else
  {
    digitalWrite(10, LOW);
  }

  if (cs_7_5.capacitiveSensor(3) > 20)
  {
    digitalWrite(9, HIGH);
  }
  else
  {
    digitalWrite(9, LOW);
  }

  if (cs_7_4.capacitiveSensor(3) > 20) 
  {
    digitalWrite(11, HIGH);
  }
  else
  {
    digitalWrite(11, LOW);
  }

  if (cs_7_3.capacitiveSensor(3) > 20) 
  {
    digitalWrite(8, HIGH);
  }
  else
  {
    digitalWrite(8, LOW);
  }

  if (cs_7_2.capacitiveSensor(3) > 20) 
  {
    digitalWrite(12, HIGH);
  }
  else
  {
    digitalWrite(12, LOW);
  }
}

