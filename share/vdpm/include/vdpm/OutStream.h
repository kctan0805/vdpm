#ifndef VDPM_OUTSTREAM_H
#define VDPM_OUTSTREAM_H

namespace vdpm
{
    class OutStream
    {
    public:
        virtual void writeChar(char& value) = 0;
        virtual void writeUInt(unsigned int& value) = 0;
        virtual void writeFloat(float& value) = 0;
    };
} // namespace vdpm

#endif // VDPM_OUTSTREAM_H
