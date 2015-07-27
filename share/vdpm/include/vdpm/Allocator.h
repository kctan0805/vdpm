#ifndef VDPM_ALLOCATOR_H
#define VDPM_ALLOCATOR_H

#include "vdpm/Types.h"

namespace vdpm
{
    class Allocator
    {
    public:
        ~Allocator();

        static Allocator& getInstance();

        AVertex* allocAVertex();
        void freeAVertex(AVertex* avertex);
        AFace* allocAFace();
        void freeAFace(AFace* aface);
        TStrip* allocTStrip();
        void freeTStrip(TStrip* tstrip, AFace& afaces);

    #ifdef VDPM_GEOMORPHS
        VMorph* allocVMorph();
        void freeVMorph(VMorph* vmorph);
    #endif // VDPM_GEOMORPHS

    private:
        Allocator();

    #ifdef VDPM_REUSE_OBJECTS
        static AVertex* freeAVertices;
        static AFace* freeAFaces;
        static TStrip* freeTStrips;
        static VMorph* freeVMorphs;
    #endif // VDPM_REUSE_OBJECTS
    };
} // namespace vdpm

#endif // VDPM_ALLOCATOR_H
