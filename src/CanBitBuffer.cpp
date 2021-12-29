/*
    Author Jacob Waters
    fierywaters13@gmail.com
    Designed for CSULB Beach Launch Team
*/
#define dataSize 8//the size of an element in buf[]



#include "CanBitBuffer.h"
#include <Streaming.h> //testing
void CanBitBuffer::init()
{
    usedBits = 0;
}

int CanBitBuffer::getArraySize()
{
    //Serial <<"maxSize " << maxSize << endl;
    int arraySize = maxSize / dataSize;
    if (maxSize % dataSize != 0)//when there is a remainder, need to go to the next higher size
    {
        arraySize++;
    }      
    //Serial <<"in getArraySize() " << arraySize << endl;

    return arraySize;
}
CanBitBuffer::CanBitBuffer()
{
    maxSize = 64;
    buf = new int8_t[getArraySize()];
    reset();
    init();
}
CanBitBuffer::CanBitBuffer(int8_t* data, int sizeInBits)
{
    init();
    buf = data;
    maxSize = sizeInBits;    
    reset();

}

int CanBitBuffer::getMaxBufferSize()
{
    //Serial << "here " << maxSize << endl;
    return maxSize;
}
/*
returns the total number of additional bits which can be written to this CanBitBuffer.
*/
int CanBitBuffer::getFreeBits()
{    
    //Serial << "here " << maxSize << "  " << usedBits<< endl;
    return getMaxBufferSize() - usedBits;
}
//returns true if it can fit nBits more bits
bool CanBitBuffer::canFit(int nBits)
{
    //Serial << "HERE " << getFreeBits() << endl;
    //delay(30);
    if (getFreeBits() - nBits >= 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//private send helper methods

//returns index for buf[index]
int_fast32_t CanBitBuffer::getBufferIndex()
{
    return usedBits / dataSize;
}
/*
returns the size of the low-level data unit we're writing to. 
This can be 29, 11, or 8. 
*/
int8_t CanBitBuffer::getBufferSize()
{
    return 8;
}

/*
returns the index of leftmost free bit, 
which is also the size of the free space of the current 
buffer we are writing to (id or buf[i])
*/
int8_t CanBitBuffer::getBufferFreeSpace()
{
    return 8 - (usedBits % 8); //usedBits % 8 is used space, 8 - that is free space
}
/*
Returns the leftmost index which data of bitWidth size can be written to without overwriting data on the left. 
*/
int8_t CanBitBuffer::getBitBoundaryIndex(int8_t bitWidth)
{
    return getBufferFreeSpace() - bitWidth;
}
void CanBitBuffer::writeBitsHelper(int32_t data, int8_t dataWidth, int8_t dataOffset)
{
    config compression = {dataWidth, dataOffset}; //take relevant bits our of data

    int8_t destinationOffset = getBitBoundaryIndex(dataWidth); //find leftmost index which will contain data (pack from left to right)
    config extraction = {dataWidth, destinationOffset};         //mapping for where our relevant bits go

    BitChopper bitChopper;

    //maps relevant bits in input to the right offset for the output
    int32_t selectedData = bitChopper.compress(compression, data);
    int32_t dataToWrite = bitChopper.extract(extraction, selectedData);

    int8_t index = getBufferIndex(); //figure out which part of msg to write to

    buf[index] = buf[index] | (int8_t)dataToWrite;

    usedBits += dataWidth; //update to indicate we wrote to CAN buffer

    //must be done after updating usedBits to ensure index is consistent
    len = getBufferIndex() + 1; //update length to make sure the message sends properly

    if (len > 8) //in edge case when bit buffer is totally full, len can erroneously be 9.
        len = 8; //testing will need to change before it can be general!!
}
void CanBitBuffer::writeBits(int32_t data, int8_t dataWidth)
{
    while (dataWidth > 0)
    {
        //you can only write as many bits the smaller of the current buffer free space and our input dataWidth
        int8_t dataWidthHelper = min(getBufferFreeSpace(), dataWidth);
        //start writing from the MSB to LSB, ie write left side first. Simplifies to 0 if we can write dataWidth bits
        int8_t dataOffset = dataWidth - dataWidthHelper;
        writeBitsHelper(data, dataWidthHelper, dataOffset);
        dataWidth -= dataWidthHelper;
    }
}

//how many bits to pull from CAN bit buffer, offset is how those bits should be shifted before returned
int32_t CanBitBuffer::readBitsHelper(int8_t bitWidth, int8_t offset)
{
    int8_t bufferIndex = getBufferIndex();
    //Serial << "readBitsHelper: \n";
    //Serial.print("bufferIndex: ");
    //Serial.println(bufferIndex);
    int8_t bitIndex = getBitBoundaryIndex(bitWidth);
    config compression = {bitWidth, bitIndex};
    config extraction = {bitWidth, offset};

    BitChopper bitChopper;

    int32_t data;
    data = buf[bufferIndex];

    //maps relevant bits in input to the right offset for the output
    int32_t selectedData = bitChopper.compress(compression, data);
    int32_t dataToWrite = bitChopper.extract(extraction, selectedData);

    usedBits += bitWidth;
    if (usedBits > getMaxBufferSize())
    {
        Serial.print("\n\nBUFFER OVERFLOW: you read past the end of the CAN Frame, returned value contains garbage data\n");
        Serial.print("Bruh, bruh, you're really stress testing my code here.\nPshhh, trying to make me look bad.\n\n");
        usedBits = getMaxBufferSize();
    }

    return dataToWrite;
}
int32_t CanBitBuffer::readBits(int8_t bitWidth)
{
    int32_t toReturn = 0;
    while (bitWidth > 0)
    {
        //you can only write as many bits the smaller of the current buffer free space and our input dataWidth
        int8_t dataWidthHelper = min(getBufferFreeSpace(), bitWidth);
        Serial << "getBufferFreeSpace() " << getBufferFreeSpace() << endl;
        
        //start writing from the MSB to LSB, ie write left side first. Simplifies to 0 if we can write dataWidth bits
        int8_t dataOffset = bitWidth - dataWidthHelper;
        toReturn = toReturn | readBitsHelper(dataWidthHelper, dataOffset);
        bitWidth -= dataWidthHelper;
    }
    //Serial << "ReadBits return: " << toReturn << endl;
    return toReturn;
}
void CanBitBuffer::reset()
{
    usedBits = 0;
    int size = getArraySize();
    //Serial << "outside it " << size << endl;
    for (int i = 0; i < getArraySize(); i++)//testing error fix here
        buf[i] = 0;
}
//will be deleted, for testing only.
int8_t* CanBitBuffer::getBuffer()
{
    return buf;
}

//useful for testing and debug
void CanBitBuffer::printBits(int data, int size)
{
    for (int i = size - 1; i >= 0; i--)
    {
        Serial.print(bitRead(data, i));
        if (i % 4 == 0)
            Serial.print("");
    }
    Serial.print("-");
}
void CanBitBuffer::printBuffer()
{
    for (int i = 0; i < getArraySize(); i++)
    {
        printBits(buf[i], 8);
    }
}