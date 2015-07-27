#ifndef VDPM_SERIALIZER_H
#define VDPM_SERIALIZER_H

#include "vdpm/InStream.h"
#include "vdpm/OutStream.h"
#include "vdpm/Types.h"

namespace vdpm
{
    class Serializer
    {
    public:
        ~Serializer();

        static Serializer& getInstance();
        SRMesh* loadSRMesh(InStream& is);
        SRMesh* loadSRMesh(const char filePath[]);

        SRMesh* readSRMesh(InStream& is);
        int writeSRMesh(OutStream& os, SRMesh* srmesh);

    private:
        Serializer();

        int readSRMesh(InStream& is, SRMesh* srmesh);
        int readTextureName(InStream& is, SRMesh* srmesh);
    };
} // namespace vdpm

#endif // VDPM_SERIALIZER_H
