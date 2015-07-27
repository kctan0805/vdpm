#include <cassert>
#include <cstdlib>
#include "vdpm/Allocator.h"

using namespace std;
using namespace vdpm;

#ifdef VDPM_REUSE_OBJECTS
AVertex* Allocator::freeAVertices = NULL;
AFace* Allocator::freeAFaces = NULL;
TStrip* Allocator::freeTStrips = NULL;
VMorph* Allocator::freeVMorphs = NULL;
#endif // VDPM_REUSE_OBJECTS

Allocator::Allocator()
{
    // do nothing
}

Allocator::~Allocator()
{
#ifdef VDPM_REUSE_OBJECTS
    AVertex *avertex, *avertexNext;
    AFace *aface, *afaceNext;
    TStrip *tstrip, *tstripNext;

#ifdef VDPM_GEOMORPHS
    VMorph *vmorph, *vmorphNext;

    vmorph = freeVMorphs;
    while (vmorph)
    {
        vmorphNext = vmorph->next;
        delete vmorph;
        vmorph = vmorphNext;
    }
#endif // VDPM_GEOMORPHS

    tstrip = freeTStrips;
    while (tstrip)
    {
        tstripNext = tstrip->next;
        delete tstrip;
        tstrip = tstripNext;
    }

    aface = freeAFaces;
    while (aface)
    {
        afaceNext = aface->next;
        delete aface;
        aface = afaceNext;
    }

    avertex = freeAVertices;
    while (avertex)
    {
        avertexNext = avertex->next;
        delete avertex;
        avertex = avertexNext;
    }
#endif // VDPM_REUSE_OBJECTS
}

Allocator& Allocator::getInstance()
{
    static Allocator self;
    return self;
}

AVertex* Allocator::allocAVertex()
{
#ifdef VDPM_REUSE_OBJECTS
    if (freeAVertices)
    {
        AVertex* avertex = freeAVertices;
        freeAVertices = freeAVertices->next;
        assert(avertex->prev == NULL);
        return avertex;
    }
    else
#endif // VDPM_REUSE_OBJECTS
        return new AVertex();
}

void Allocator::freeAVertex(AVertex* avertex)
{
    avertex->prev->next = avertex->next;
    avertex->next->prev = avertex->prev;

#ifdef VDPM_REUSE_OBJECTS
    avertex->next = freeAVertices;
    avertex->prev = NULL;
    freeAVertices = avertex;
#else
    delete avertex;
#endif // VDPM_REUSE_OBJECTS
}

AFace* Allocator::allocAFace()
{
#ifdef VDPM_REUSE_OBJECTS
    if (freeAFaces)
    {
        AFace* aface = freeAFaces;
        freeAFaces = freeAFaces->next;
        return aface;
    }
    else
#endif // VDPM_REUSE_OBJECTS
        return new AFace();
}

void Allocator::freeAFace(AFace* aface)
{
    aface->prev->next = aface->next;
    aface->next->prev = aface->prev;

#ifdef VDPM_REUSE_OBJECTS
    aface->next = freeAFaces;
    aface->prev = NULL;
    freeAFaces = aface;
#else
    delete aface;
#endif // VDPM_REUSE_OBJECTS
}

TStrip* Allocator::allocTStrip()
{
#ifdef VDPM_REUSE_OBJECTS
    if (freeTStrips)
    {
        TStrip* tstrip = freeTStrips;
        freeTStrips = freeTStrips->next;
        return tstrip;
    }
    else
#endif // VDPM_REUSE_OBJECTS
        return new TStrip();
}

void Allocator::freeTStrip(TStrip* tstrip, AFace& afaces)
{
    AFace* aface;

    for (aface = tstrip->afaces; aface->next; aface = aface->next)
        aface->tstrip = NULL;
    aface->tstrip = NULL;

    delete[] tstrip->vgIndices;

    aface->next = afaces.next;
    afaces.next->prev = aface;
    tstrip->afaces->prev = &afaces;
    afaces.next = tstrip->afaces;

    tstrip->prev->next = tstrip->next;
    tstrip->next->prev = tstrip->prev;

#ifdef VDPM_REUSE_OBJECTS
    tstrip->next = freeTStrips;
    freeTStrips = tstrip;
#else
    delete tstrip;
#endif // VDPM_REUSE_OBJECTS

}

#ifdef VDPM_GEOMORPHS
VMorph* Allocator::allocVMorph()
{
#ifdef VDPM_REUSE_OBJECTS
    if (freeVMorphs)
    {
        VMorph* vmorph = freeVMorphs;
        freeVMorphs = freeVMorphs->next;
        return vmorph;
    }
    else
#endif // VDPM_REUSE_OBJECTS
        return new VMorph();
}

void Allocator::freeVMorph(VMorph* vmorph)
{
    vmorph->prev->next = vmorph->next;
    vmorph->next->prev = vmorph->prev;

#ifdef VDPM_REUSE_OBJECTS
    vmorph->next = freeVMorphs;
    vmorph->prev = NULL;
    freeVMorphs = vmorph;
#else
    delete vmorph;
#endif // VDPM_REUSE_OBJECTS
}
#endif // VDPM_GEOMORPHS
