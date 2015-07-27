#ifndef VDPM_STDINSTREAM_H
#define VDPM_STDINSTREAM_H

#include <fstream>
#include "vdpm/InStream.h"

namespace vdpm
{
    class StdInStream : public InStream
    {
    public:
        StdInStream(const char filePath[]);
        StdInStream(std::istream* is);
        ~StdInStream();

        void close();

        void readChar(char& value);
        void readUInt(unsigned int& value);
        void readFloat(float& value);

    private:
        std::istream* is;
        std::ifstream fin;
    };
} // namespace vdpm

#endif // VDPM_STDINSTREAM_H
