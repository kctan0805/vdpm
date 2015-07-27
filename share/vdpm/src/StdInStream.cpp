#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include "vdpm/Log.h"
#include "vdpm/StdInStream.h"

using namespace std;
using namespace vdpm;

StdInStream::StdInStream(const char filePath[])
{
    fin.open(filePath, ifstream::in | ifstream::binary);

    if (!fin.is_open())
    {
        Log::println("failed to open %s", filePath);
        return;
    }
    is = &fin;
}

StdInStream::StdInStream(std::istream* is)
{
    this->is = is;
}

StdInStream::~StdInStream()
{
    close();
}

void StdInStream::close()
{
    if (is == &fin)
    {
        fin.close();
        is = NULL;
    }
}

void StdInStream::readChar(char& value)
{
    is->read((char*)&value, sizeof(char));
}

void StdInStream::readUInt(unsigned int& value)
{
    is->read((char*)&value, sizeof(unsigned int));
}

void StdInStream::readFloat(float& value)
{
    is->read((char*)&value, sizeof(float));
}
