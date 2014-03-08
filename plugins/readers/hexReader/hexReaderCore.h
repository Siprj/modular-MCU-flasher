#ifndef HEXREADERCORE_H
#define HEXREADERCORE_H

#include <fstream>
#include <string>
#include <string.h>
#include <sstream>
#include <iostream>
#include <list>

using namespace std;

class HexReaderCore
{
public:
    HexReaderCore();

    int open(const char *fileName);
    unsigned int getDataSize();
    int getData(unsigned int addr, int maxSize, char* data);
    unsigned int readHex();

    string getErrorStr();

    void printAll();
    void close();
    int isOpen();

private:
    ifstream file;
    unsigned int dataSize;

    string trim(string &str);

    string errorStr;

    unsigned int HexToInt(char *str, int length);
    unsigned char crcCalculate(char* str, int length);


    typedef struct{
        unsigned int address;
        unsigned char c;
    }Record;

    Record* findBlock(unsigned int address);
    static bool compareAddress(Record* first, Record* second);
    list<Record*> dataList;

};

#endif // HEXREADER_H
