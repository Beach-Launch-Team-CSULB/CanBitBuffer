#include <Arduino.h>
#include <Streaming.h>//not necessary but nice for print statements
#include "CanBitBuffer.h"

void setup()
{
  while(!Serial);//wait for serial to begin
}

void loop()
{
  CanBitBuffer bitBuffer;
  int data1Size = 1; //1 bit
  int data1 = 1;
  if (bitBuffer.canFit(data1Size))
    bitBuffer.writeBits(data1, data1Size);

  int data2Size = 5;      //5 bit
  int data2 = (1 << 5) - 1; //31
  if (bitBuffer.canFit(data1Size))
    bitBuffer.writeBits(data2, data2Size);

  int data3Size = 5; //5 bit
  int data3 = 21;    //10101
  if (bitBuffer.canFit(data1Size))
    bitBuffer.writeBits(data3, data3Size);

  Serial << "binary CAN Message: ";
  bitBuffer.printCanMessage();
  Serial << endl;
  
  CAN_message_t output = bitBuffer.getCanMessage();//ready for CAN Bus
  //sendCAN frame here

  //after CAN Receive
  CanBitBuffer fromCan(output);//construct from CAN Bus
  int data1Copy;
  int data2Copy;
  int data3Copy;
  data1Copy = fromCan.readBits(data1Size);
  data2Copy = fromCan.readBits(data2Size);
  data3Copy = fromCan.readBits(data3Size);

  //ProofOfConcept:
  Serial << "\ndata1:     " << data1 << endl;
  Serial << "data1Copy: " << data1Copy << endl << endl;
  
  Serial << "data2:     " << data2 << endl;
  Serial << "data2Copy: " << data2Copy << endl << endl;
  
  Serial << "data3:     " << data3 << endl;
  Serial << "data3Copy: " << data3Copy << endl << endl;
  

  // put your main code here, to run repeatedly:

  while(1)
    delay(1000);//only do this sketch once
}