#include "hexReaderCore.h"
#include <utility>

HexReaderCore::HexReaderCore()
{

    dataSize = 0;
}

int HexReaderCore::open(const char *fileName)
{

    file.open(fileName);
    return file.is_open();
}

bool HexReaderCore::compareAddress(Record* first, Record* second)
{

    if (first->address<second->address)
        return true;
    else
        return false;
}

HexReaderCore::Record *HexReaderCore::findBlock(unsigned int address)
{

    for(list<Record*>::iterator it = dataList.begin(); it != dataList.end(); it++)
    {
        if((*it)->address == address)
            return (*it);
    }
    return NULL;
}

unsigned int HexReaderCore::readHex()
{

    char *line;
    int dataLen;
    int address;
    int recordType;
    unsigned addrApender = 0;
    bool notAtEndHex = true;
    unsigned int crc;
    string lineString;

    while(!file.eof() && notAtEndHex)
    {
        getline(file, lineString);
        lineString = trim(lineString);
        line = (char*)lineString.c_str();
        unsigned short lineLen = strlen(line);
        if(lineLen >= 11)  //header of hex line is :llAAAATT[DD]CC -> smalleste line is 11 characters long
        {
            dataLen = HexToInt(&line[1], 2);
            if((lineLen - 11)/2 == dataLen)     //dataLen and data size in *.hex control
            {
                address = HexToInt(&line[3], 4);
                recordType = HexToInt(&line[7], 2);
                crc = crcCalculate(&line[1], lineLen-3);
                if(crc == HexToInt(&line[lineLen-2], 2))
                {
                    Record *record;
                    switch(recordType)
                    {
                    case 0:     //data send //address used
                        for(int y = 9, z = 0; z < dataLen; y += 2, z++)
                        {
                            record = new Record;
                            record->address = address + addrApender;
                            record->c = (unsigned char)HexToInt(&line[y], 2);
                            dataList.push_back(record);
                            address++;
                        }
                        break;
                    case 1:     //end of record
                        notAtEndHex = false;
                        break;
                    case 2:     //extended segment address record
                        addrApender = HexToInt(&line[9], 4);
                        addrApender = addrApender <<4;
                        break;
                    case 3:     //start segment address record (MDK-ARM only, not need to implement)
                        break;
                    case 4:     //extended linear address record
                        addrApender = HexToInt(&line[9], 4);
                        addrApender = addrApender <<16;
                        break;
                    case 5:     //start linear address record (MDK-ARM only, not need to implement)
                        break;
                    default:
                        return 0;   //recordType error
                    }
                }
                else
                {
                    errorStr = "bad crc\n";
                    return 0;       //bad crc
                }
            }
            else
            {
                errorStr = "data length or data are corupted\n";
                return 0;
            }
        }
        else
        {
            errorStr = "HEX file is corupted\n";
            return 0;
        }
    }
    dataList.sort(compareAddress);          //sort before add "empty" bocks


    list<Record*>::iterator end = dataList.end();
    end--;
    for(list<Record*>::iterator it = dataList.begin(); it != end; it++)
    {
        list<Record*>::iterator pom = it;
        pom++;
        if(!(1 == (*(pom++))->address-(*it)->address))
        {
            int number_of_bolcks;
            number_of_bolcks = (((*(pom++))->address-(*it)->address)/16);

            for(int i = 1; i < number_of_bolcks; i++)
            {
                Record *record = new Record;
                record->address = (*it)->address + (i);
                record->c = 0xFF;
                dataList.push_back(record);
            }
        }
    }
    dataList.sort(compareAddress);          //final sort
    dataSize = dataList.size();
    return dataSize;
}

unsigned int HexReaderCore::HexToInt(char *str, int length)
{

    unsigned int val;
    stringstream stream;
    stream.write(str, length);
    stream >> std::hex >> val;

    return val;
}

unsigned char HexReaderCore::crcCalculate(char* str, int length)
{

    unsigned char crc = 0;
    for(int i = 0; i < length; i+=2)
    {
        crc += HexToInt(&str[i], 2);
    }
    return (0xFF-crc)+1;
}


string HexReaderCore::getErrorStr()
{

    return errorStr;
}

void HexReaderCore::printAll()
{

    cout<<"vypis"<<endl;
    int i =0;
    for(list<Record*>::iterator it = dataList.begin(); it != dataList.end(); it++)
    {
        if(i == 0)
        {
            cout<<endl;
            cout<<"address: "<<(*it)->address<<"\t";
            i=16;
        }
        i--;
        cout<<hex;

        cout<<(unsigned int)(*it)->c<<" ";
    }
    cout<<endl<<"vypis konec"<<endl;
}

unsigned int HexReaderCore::getDataSize()
{

    return dataSize;
}

int HexReaderCore::getData(unsigned int addr, int maxSize, char* data)
{

    list<Record*>::iterator it = dataList.begin();
    for(; (*it)->address != addr; it++)
        ;

    for(int i = 0; i < maxSize; i++, it++)
    {
        data[i] = (*it)->c;
    }

    return 0;
}

void HexReaderCore::close()
{

    file.close();
    for(list<Record*>::iterator it = dataList.begin(); it != dataList.end(); it++)
    {
        delete *it;
    }
    dataList.clear();
}

int HexReaderCore::isOpen()
{

    return file.is_open();
}

string HexReaderCore::trim(std::string &str)
{

    size_t end = str.find_last_not_of(" \n\r\t");
    if(end != std::string::npos)
        str.resize(end + 1);

    size_t start = str.find_first_not_of(" \n\r\t");
    if(start != std::string::npos)
        str = str.substr(start);

    return str;
}

