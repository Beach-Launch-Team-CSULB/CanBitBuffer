#include <Arduino.h>
#include <Streaming.h> //not necessary but nice for print statements
#include "CanBitBuffer.h"

void setup()
{
  while (!Serial) //WARNING DISABLE BEFORE LAUNCH TESTING
    ;             //wait for serial to begin
}

void loop()
{
  int8_t buf[32];

  // for(int i =0; i < 32; i++)
  // {
  //   buf[i]=0;
  // }
  // for(int i =0; i < 32; i++)
  // {
  //   Serial << buf[i] << " ";
  // }

  int32_t bitSize = 32 * 8;
  CanBitBuffer bitBuffer(buf, bitSize);
  bitBuffer.printBuffer();
  Serial << "Free space: " << bitBuffer.getFreeBits() << endl;

  int data1Size = 1; //1 bit
  int data1 = 1;
  if (bitBuffer.canFit(data1Size))
    bitBuffer.writeBits(data1, data1Size);

  int data2Size = 5; //5 bit
  int data2 = 0b11111;
  //int data2 = (1 << 5) - 1; //31
  if (bitBuffer.canFit(data1Size))
    bitBuffer.writeBits(data2, data2Size);

  int data3Size = 5; //5 bit
  int data3 = 0b10101;
  if (bitBuffer.canFit(data1Size))
    bitBuffer.writeBits(data3, data3Size);
  Serial << "Free space: " << bitBuffer.getFreeBits() << endl;

  while (bitBuffer.canFit(10))
  {
    bitBuffer.writeBits(0b0001111000, 10);
    Serial << "Free space: " << bitBuffer.getFreeBits() << endl;
  }
  Serial << "Free space: " << bitBuffer.getFreeBits() << endl;

  while (bitBuffer.canFit(1))
  {
    //Serial << "here";
    bitBuffer.writeBits(0b1, 1);
  }

  Serial << "binary CAN Message: ";
  bitBuffer.printBuffer();
  Serial << endl;

  int8_t *output = bitBuffer.getBuffer(); //ready for CAN Bus
  //sendCAN frame here
  // for (int i = 0; i < 32; i++)
  //   Serial <<"output[i] " <<  output[i] << endl;

  //after CAN Receive
  CanBitBuffer fromCan(output, bitSize); //construct from external array
  int data1Copy;
  int data2Copy;
  int data3Copy;
  data1Copy = fromCan.readBits(data1Size);
  data2Copy = fromCan.readBits(data2Size);
  data3Copy = fromCan.readBits(data3Size);

  //ProofOfConcept:
  Serial << "\ndata1:     " << data1 << endl;
  Serial << "data1Copy: " << data1Copy << endl
         << endl;

  Serial << "data2:     " << data2 << endl;
  Serial << "data2Copy: " << data2Copy << endl
         << endl;

  Serial << "data3:     " << data3 << endl;
  Serial << "data3Copy: " << data3Copy << endl
         << endl;

  while (fromCan.canFit(10))
  {
    Serial << "Filler: " << fromCan.readBits(10) << endl;
  }

  while (fromCan.canFit(1))
  {
    Serial << "Filler: " << fromCan.readBits(1) << endl;
  }
  Serial << "Free space: " << bitBuffer.getFreeBits() << endl;

  while (1)
    delay(1000); //only do this sketch once
}