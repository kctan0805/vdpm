#ifndef VDPM_INSTREAM_H
#define VDPM_INSTREAM_H

namespace vdpm
{
    class InStream
    {
    public:
        virtual void readChar(char& value) = 0;
        virtual void readUInt(unsigned int& value) = 0;
        virtual void readFloat(float& value) = 0;
    };
} // namespace vdpm

#endif // VDPM_INSTREAM_H
