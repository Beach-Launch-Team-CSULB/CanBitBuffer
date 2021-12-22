/*
    Author Jacob Waters
    fierywaters13@gmail.com
    Designed for CSULB Beach Launch Team
*/
#include "CanBitBuffer.h"
#include <Streaming.h> //testing

void CanBitBuffer::init()
{
    usedBits = 0;
}
//TODO testing
CanBitBuffer::CanBitBuffer()
{
    buf = new uint8_t[8];
    reset();
    init();
}
CanBitBuffer::CanBitBuffer(uint8_t* data)
{
    init();
    buf = data;
}

uint8_t CanBitBuffer::getMaxBufferSize()
{
    return 64;
}
/*
returns the total number of additional bits which can be written to this CanBitBuffer.
*/
uint8_t CanBitBuffer::getFreeBits()
{
    return getMaxBufferSize() - usedBits;
}
//returns true if it can fit nBits more bits
bool CanBitBuffer::canFit(uint8_t nBits)
{
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
int8_t CanBitBuffer::getBufferIndex()
{
    return usedBits / 8;
}
/*
returns the size of the low-level data unit we're writing to. 
This can be 29, 11, or 8. 
*/
uint8_t CanBitBuffer::getBufferSize()
{
    return 8;
}

/*
returns the index of leftmost free bit, 
which is also the size of the free space of the current 
buffer we are writing to (id or buf[i])
*/
uint8_t CanBitBuffer::getBufferFreeSpace()
{
    return 8 - (usedBits % 8); //usedBits % 8 is used space, 8 - that is free space
}
/*
Returns the leftmost index which data of bitWidth size can be written to without overwriting data on the left. 
*/
uint8_t CanBitBuffer::getBitBoundaryIndex(uint8_t bitWidth)
{
    return getBufferFreeSpace() - bitWidth;
}
void CanBitBuffer::writeBitsHelper(uint32_t data, uint8_t dataWidth, uint8_t dataOffset)
{
    config compression = {dataWidth, dataOffset}; //take relevant bits our of data

    uint8_t destinationOffset = getBitBoundaryIndex(dataWidth); //find leftmost index which will contain data (pack from left to right)
    config extraction = {dataWidth, destinationOffset};         //mapping for where our relevant bits go

    BitChopper bitChopper;

    //maps relevant bits in input to the right offset for the output
    uint32_t selectedData = bitChopper.compress(compression, data);
    uint32_t dataToWrite = bitChopper.extract(extraction, selectedData);

    int8_t index = getBufferIndex(); //figure out which part of msg to write to

    buf[index] = buf[index] | (uint8_t)dataToWrite;

    usedBits += dataWidth; //update to indicate we wrote to CAN buffer

    //must be done after updating usedBits to ensure index is consistent
    len = getBufferIndex() + 1; //update length to make sure the message sends properly

    if (len > 8) //in edge case when bit buffer is totally full, len can erroneously be 9.
        len = 8; //testing will need to change before it can be general!!
}
void CanBitBuffer::writeBits(uint32_t data, uint8_t dataWidth)
{
    while (dataWidth > 0)
    {
        //you can only write as many bits the smaller of the current buffer free space and our input dataWidth
        uint8_t dataWidthHelper = min(getBufferFreeSpace(), dataWidth);
        //start writing from the MSB to LSB, ie write left side first. Simplifies to 0 if we can write dataWidth bits
        uint8_t dataOffset = dataWidth - dataWidthHelper;
        writeBitsHelper(data, dataWidthHelper, dataOffset);
        dataWidth -= dataWidthHelper;
    }
}

//how many bits to pull from CAN bit buffer, offset is how those bits should be shifted before returned
uint32_t CanBitBuffer::readBitsHelper(uint8_t bitWidth, uint8_t offset)
{
    int8_t bufferIndex = getBufferIndex();
    //Serial << "readBitsHelper: \n";
    //Serial.print("bufferIndex: ");
    //Serial.println(bufferIndex);
    uint8_t bitIndex = getBitBoundaryIndex(bitWidth);
    config compression = {bitWidth, bitIndex};
    config extraction = {bitWidth, offset};

    BitChopper bitChopper;

    uint32_t data;
    data = buf[bufferIndex];

    //maps relevant bits in input to the right offset for the output
    uint32_t selectedData = bitChopper.compress(compression, data);
    uint32_t dataToWrite = bitChopper.extract(extraction, selectedData);

    usedBits += bitWidth;
    if (usedBits > getMaxBufferSize())
    {
        Serial.print("\n\nBUFFER OVERFLOW: you read past the end of the CAN Frame, returned value contains garbage data\n");
        Serial.print("Bruh, bruh, you're really stress testing my code here.\nPshhh, trying to make me look bad.\n\n");
        usedBits = getMaxBufferSize();
    }

    return dataToWrite;
}
uint32_t CanBitBuffer::readBits(uint8_t bitWidth)
{
    uint32_t toReturn = 0;
    while (bitWidth > 0)
    {
        //you can only write as many bits the smaller of the current buffer free space and our input dataWidth
        uint8_t dataWidthHelper = min(getBufferFreeSpace(), bitWidth);
        //start writing from the MSB to LSB, ie write left side first. Simplifies to 0 if we can write dataWidth bits
        uint8_t dataOffset = bitWidth - dataWidthHelper;
        toReturn = toReturn | readBitsHelper(dataWidthHelper, dataOffset);
        bitWidth -= dataWidthHelper;
    }
    return toReturn;
}
void CanBitBuffer::reset()
{
    usedBits = 0;

    len = 0;
    for (int i = 0; i < 8; i++)
        buf[i] = 0;
}
//will be deleted, for testing only.
uint8_t* CanBitBuffer::getBuffer()
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
    for (int i = 0; i < len; i++)
    {
        printBits(buf[i], 8);
    }
}