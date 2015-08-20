/* vdpm - View-dependent progressive meshes library
* Copyright 2015 Jim Tan
* https://github.com/kctan0805/vdpm
*
* osgEarth is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>
*/
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include "vdpm/Allocator.h"
#include "vdpm/Log.h"
#include "vdpm/Renderer.h"
#include "vdpm/SRMesh.h"
#include "vdpm/Viewport.h"

using namespace std;
using namespace vdpm;

#define VSTACK_SIZE             10
#define INDICES_BUFFER_SIZE     3
#define INDICES_ARRAY_SIZE      1
#define MAX_TAU                 1.0f
#define MAX_GTIME               72
#define MIN_GTIME               2
#define AMORTIZATION_STEP       1
typedef Vertex*                 VertexPointer;

SRMesh::SRMesh()
{
    ::memset(this, 0, sizeof(SRMesh));

    avertices.next = &averticesEnd;
    averticesEnd.prev = &avertices;
    afaces.next = &afacesEnd;
    afacesEnd.prev = &afaces;
    tstrips.next = &tstripsEnd;
    tstripsEnd.prev = &tstrips;
#ifdef VDPM_GEOMORPHS
    vmorphs.next = &vmorphsEnd;
    vmorphsEnd.prev = &vmorphs;
    gtime = 8;
    gmorphTstrips.next = &gmorphTstripsEnd;
    gmorphTstripsEnd.prev = &gmorphTstrips;
#endif
    vstackSize = VSTACK_SIZE;

#ifdef VDPM_REGULATION
    targetAFaceCount = UINT_MAX;
#endif

#ifdef VDPM_AMORTIZATION
    amortizeAvertex = &averticesEnd;
    amortizeStep = AMORTIZATION_STEP;
#endif
}

SRMesh::~SRMesh()
{
    AVertex *avertex, *avertexNext;
    AFace *aface, *afaceNext;
    TStrip *tstrip, *tstripNext;
#ifdef VDPM_GEOMORPHS
    VMorph *vmorph, *vmorphNext;

    vmorph = vmorphs.next;
    while (vmorph != &vmorphsEnd)
    {
        vmorphNext = vmorph->next;
        delete vmorph;
        vmorph = vmorphNext;
    }
    tstrip = gmorphTstrips.next;
    while (tstrip != &gmorphTstripsEnd)
    {
        tstripNext = tstrip->next;
        delete[] tstrip->vgIndices;
        delete tstrip;
        tstrip = tstripNext;
    }
#endif // VDPM_GEOMORPHS
    tstrip = tstrips.next;
    while (tstrip != &tstripsEnd)
    {
        tstripNext = tstrip->next;
        delete[] tstrip->vgIndices;
        delete tstrip;
        tstrip = tstripNext;
    }
    aface = afaces.next;
    while (aface != &afacesEnd)
    {
        afaceNext = aface->next;
        delete aface;
        aface = afaceNext;
    }
    avertex = avertices.next;
    while (avertex != &averticesEnd)
    {
        avertexNext = avertex->next;
        delete avertex;
        avertex = avertexNext;
    }
    delete[] vsplits;
    delete[] faces;

#ifdef VDPM_RENDERER_OPENGL_IBO
    if (renderer)
        renderer->destroyBuffer(ibo);
#endif

    geometry.destroy();

    delete[] vertices;
    delete[] indicesCountArray;
    delete[] indicesArray;
    ::free(indicesBuffer);
    ::free(vstack);
    delete[] texname;
}

int SRMesh::realize(Renderer* renderer)
{
    vstack = (VertexPointer*)::malloc(sizeof(VertexPointer) * VSTACK_SIZE);
    if (!vstack)
        goto error;

    indicesBuffer = (unsigned int*)::malloc(sizeof(unsigned int) * INDICES_BUFFER_SIZE);
    if (!indicesBuffer)
        goto error;

    indicesBufferSize = INDICES_BUFFER_SIZE;

    indicesArray = new unsigned int*[INDICES_ARRAY_SIZE];
    if (!indicesArray)
        goto error;

    indicesArraySize = INDICES_ARRAY_SIZE;

    indicesCountArray = new unsigned int[INDICES_ARRAY_SIZE];
    if (!indicesCountArray)
        goto error;

    if (geometry.realize(renderer))
        goto error;

#ifdef VDPM_TSTRIP_RESTRIP_ALL
    tstripDirty = true;
#endif

    this->renderer = renderer;
    return 0;

error:
    ::free(vstack);
    ::free(indicesBuffer);
    delete[] indicesArray;
    delete[] indicesCountArray;

    return -1;
}

void SRMesh::setViewport(Viewport* viewport)
{
    assert(viewport);
    this->viewport = viewport;
}

void SRMesh::setViewAngle(float fov)
{
    tanPhi = 2.0f * tan(fov / 2.0f);
    kappa2 = tau * tanPhi;
    kappa2 *= kappa2;
}

void SRMesh::setTau(float tau)
{
    this->tau = tau;
    kappa2 = tau * tanPhi;
    kappa2 *= kappa2;

#ifdef VDPM_REGULATION
    targetTau = tau;
#endif
}

#ifdef VDPM_REGULATION
void SRMesh::setTargetAFaceCount(unsigned int count)
{
    targetAFaceCount = count;
}
#endif

#ifdef VDPM_AMORTIZATION

void SRMesh::setAmortizeStep(unsigned int step)
{
    amortizeStep = step;
}
#endif

#ifdef VDPM_GEOMORPHS

void SRMesh::setGTime(unsigned int gtime)
{
    if (gtime < MIN_GTIME || gtime > MAX_GTIME)
        return;

    this->gtime = gtime;
}

void SRMesh::updateVMorphs()
{
    VMorph* vmorph = vmorphs.next;
    Vertex* v_parent;

    if (vmorph != &vmorphsEnd && !vmorphVgeoms)
    {
    #ifdef VDPM_RENDERER_OPENGL_VBO_MAP_VGEOM
        geometry.mapVGeom();
        vmorphVgeoms = getVGeom(vcount);
    #else
        vmorphVgeoms = (VGeom*)renderer->mapBuffer(RENDERER_VERTEX_BUFFER, geometry.vbo, geometry.vgeomSize * vcount, geometry.vgeomSize * vmorphSize, RENDERER_WRITE_ONLY);
    #endif // VDPM_RENDERER_OPENGL_VBO_MAP_VGEOM
    }

    while (vmorph != &vmorphsEnd)
    {
        assert(vmorph->prev);

        if (vmorph->gtime <= 0)
        {
            if (vmorph->coarsening)
            {
                v_parent = vmorph->avertex->vertex->parent;

                if (v_parent && ecolLegal(v_parent))
                {
                    VMorph* vmorph_next = vmorph->next;
                    VMorph* vmorph_next_next = vmorph->next->next;
                    assert(vmorph_next->prev);
                    assert(vmorph->avertex->vmorph == vmorph);
                    if (finishCoarsening(vmorph->avertex))
                    {
                        ecol(v_parent);
                        if (vmorph_next->prev)
                            vmorph = vmorph_next;
                        else
                            vmorph = vmorph_next_next;
                    }
                    else
                    {
                        vmorph = vmorph_next;
                    }
                    assert(vmorph->prev);
                }
                else
                {
                    assert(vmorph->prev);
                    assert(vmorph->avertex->vmorph == vmorph);
                    abortCoarsening(vmorph->avertex);
                    vmorph = vmorph->next;
                    assert(vmorph->prev);
                }
            }
            else
            {
                VMorph* vmorph_next = vmorph->next;
                removeVMorph(vmorph);
                vmorph = vmorph_next;
                assert(vmorph->prev);
            }
        }
        else
        {
            unsigned int t = vmorph->gtime;
            VGeom* vmorphVgeom = getVMorphVGeom(vmorph->vgIndex);
            VGeom* vgeom;

            if (vmorph->coarsening)
            {
                Vertex* v_parent = vmorph->avertex->vertex->parent;
                if (v_parent->avertex)
                    vgeom = getVGeom(v_parent->avertex->i);
                else
                    vgeom = getVGeom(getVGeomIndex(v_parent));
            }
            else
            {
                vgeom = getVGeom(vmorph->avertex->i);
            }

            vmorphVgeom->point = vgeom->point - vmorph->vgInc.point * (float)t;
            vmorphVgeom->normal = vgeom->normal - vmorph->vgInc.normal * (float)t;

            if (hasColor())
            {
                float* vmorphVgeomColor = geometry.getVGeomColor(vmorphVgeom);
                float* vgeomColor = geometry.getVGeomColor(vgeom);
                float* vgIncColor = geometry.getVGeomColor(&vmorph->vgInc);

                for (int i = 0; i < 3; ++i)
                    vmorphVgeomColor[i] = vgeomColor[i] - vgIncColor[i] * t;
            }

            if (hasTexCoord())
            {
                float* vmorphVgeomTexCoord = geometry.getVGeomTexCoord(vmorphVgeom);
                float* vgeomTexCoord = geometry.getVGeomTexCoord(vgeom);
                float* vgIncTexCoord = geometry.getVGeomTexCoord(&vmorph->vgInc);

                for (int i = 0; i < 2; ++i)
                    vmorphVgeomTexCoord[i] = vgeomTexCoord[i] - vgIncTexCoord[i] * t;
            }
            --vmorph->gtime;
            vmorph = vmorph->next;
            assert(vmorph->prev);
        }
    }

#ifndef NDEBUG
    assertVMorphs();
#endif
}
#endif // VDPM_GEOMORPHS

void SRMesh::updateViewport()
{
    renderer->updateViewport(viewport);
}

void SRMesh::adaptRefine()
{
    Vertex* vs;
    AVertex* avertex;

#ifdef VDPM_AMORTIZATION
    if (amortizeAvertex == &averticesEnd)
    {
        amortizeAvertex = avertices.next;
        amortizeBudget = avertexCount / amortizeStep;
    }
    amortizeCount = 0;
    avertex = amortizeAvertex;
#else
    avertex = avertices.next;

#endif // VDPM_AMORTIZATION

#ifdef VDPM_RENDERER_OPENGL_VBO_MAP_VGEOM
    if (avertex != &averticesEnd && !geometry.vgeoms)
    {
        geometry.mapVGeom();
    #ifdef VDPM_GEOMORPHS
        vmorphVgeoms = getVGeom(vcount);
    #endif
    }
#endif // VDPM_RENDERER_OPENGL_VBO_MAP_VGEOM

#ifdef VDPM_PREDICT_VIEW_POSITION
    viewport->predictViewPos = viewport->viewPos + viewport->delta_e * gtime;
#endif

#ifndef NDEBUG
    assertAVertices();
#endif

    while (avertex != &averticesEnd
    #ifdef VDPM_AMORTIZATION
        && amortizeCount++ <= amortizeBudget
    #endif
        )
    {
        vs = avertex->vertex;
    #ifdef VDPM_GEOMORPHS
        VMorph* vmorph = avertex->vmorph;
    #endif

        if (vs->i != UINT_MAX && !outsideViewFrustum(vs) &&
        #ifdef VDPM_ORIENTED_AWAY
            !orientedAway(vs) &&
        #endif
            screenErrorIllegal(vs))
        {
        #ifdef VDPM_REGULATION_FORCE
            if (afaceCount < targetAFaceCount)
        #endif
            forceVSplit(vs);

            assert(avertex->next);
            avertex = avertex->next;
        }
        else if (vs->parent && ecolLegal(vs->parent))
        {
            if (outsideViewFrustum(vs->parent)
            #ifdef VDPM_ORIENTED_AWAY
                || orientedAway(vs->parent)
            #endif
                )
            {
            #ifdef VDPM_GEOMORPHS
                if (vmorph && vmorph->coarsening)
                {
                    if (finishCoarsening(avertex))
                    {
                        assert(avertex->next);
                        avertex = avertex->next;
                        ecol(vs->parent);
                    }
                    else
                    {
                        avertex = avertex->next;
                    }
                }
                else if (!vmorph)
                {
                    if (finishCoarsening(avertex))
                    {
                        assert(avertex->next);
                        avertex = avertex->next;
                        ecol(vs->parent);
                    }
                    else
                    {
                        assert(avertex->next);
                        avertex = avertex->next;
                    }
                }
                else
                {
                    assert(avertex->next);
                    avertex = avertex->next;
                }
            #else
                assert(avertex->next);
                avertex = avertex->next;
                ecol(vs->parent);
            #endif // VDPM_GEOMORPHS
            }
        #ifdef VDPM_GEOMORPHS
            else if (screenErrorIllegal(vs->parent))
            {
                if (vmorph && vmorph->coarsening)
                    abortCoarsening(avertex);

                assert(avertex->next);
                avertex = avertex->next;
            }
            else if (vmorph && vmorph->coarsening)
            {
                if (vmorph->gtime <= 0)
                {
                    if (finishCoarsening(avertex))
                    {
                        assert(avertex->next);
                        avertex = avertex->next;
                        ecol(vs->parent);
                    }
                    else
                    {
                        assert(avertex->next);
                        avertex = avertex->next;
                    }
                }
                else
                {
                    avertex = avertex->next;
                    assert(avertex);
                }
            }
        #endif // VDPM_GEOMORPHS
            else
            {
            #ifdef VDPM_GEOMORPHS
                startCoarsening(vs->parent);
            #endif
                assert(avertex->next);
                avertex = avertex->next;
            }
        }
    #ifdef VDPM_GEOMORPHS
        else if (vmorph && vmorph->coarsening)
        {
            abortCoarsening(avertex);
            assert(avertex->next);
            avertex = avertex->next;
        }
    #endif // VDPM_GEOMORPHS
        else
        {
            assert(avertex->next);
            avertex = avertex->next;
        }
    }

#ifdef VDPM_AMORTIZATION
    amortizeAvertex = avertex;
#endif

#ifdef VDPM_REGULATION
    tau = targetTau * afaceCount / targetAFaceCount;

    if (tau > MAX_TAU)
        tau = MAX_TAU;

    kappa2 = tanPhi * tau;
    kappa2 *= kappa2;

#endif // VDPM_REGULATION
}

void SRMesh::updateScene()
{
    unsigned int i;
    TStrip* tstrip;
    AFace* aface;
    bool updated = false;

#ifdef VDPM_RENDERER_OPENGL_VBO_MAP_VGEOM
    if (geometry.vgeoms)
    {
        geometry.unmapVGeom();
    #ifdef VDPM_GEOMORPHS
        vmorphVgeoms = NULL;
    #endif
    }
#else
#ifdef VDPM_GEOMORPHS
    if (vmorphVgeoms)
    {
        renderer->flushBuffer(RENDERER_VERTEX_BUFFER, geometry.vbo, geometry.vgeomSize * vcount, geometry.vgeomSize * vmorphSize);
        renderer->unmapBuffer(RENDERER_VERTEX_BUFFER, geometry.vbo);
        vmorphVgeoms = NULL;
    }
#endif // VDPM_GEOMORPHS
#endif // VDPM_RENDERER_OPENGL_VBO_MAP_VGEOM

#ifdef VDPM_TSTRIP_RESTRIP_ALL
    if (!tstripDirty)
        return;

    tstrip = tstrips.next;
    while (tstrip != &tstripsEnd)
    {
        allocator->freeTStrip(tstrip, afaces);
        tstrip = tstrips.next;
    }
    tstripCount = 0;

    tstripDirty = false;
#elif defined(VDPM_GEOMORPHS)
    tstrip = gmorphTstrips.next;
    while (tstrip != &gmorphTstripsEnd)
    {
        TStrip* next = tstrip->next;
        assert(tstrip->gtime != USHRT_MAX);
        if (tstrip->gtime <= 0)
        {
            allocator->freeTStrip(tstrip, afaces);
            --tstripCount;
        }
        --tstrip->gtime;
        tstrip = next;
    }
#endif // VDPM_TSTRIP_RESTRIP_ALL

    aface = afaces.next;
    while (aface != &afacesEnd)
    {
        AFace *afaceNext, *afacePrev;
        unsigned int indicesBufferTop;
        bool oddStrip, noExit0, noExit1, noExit2;
    #ifdef VDPM_TSTRIP_SWAP
        bool swapped;
    #endif

        // find an appropriate head of triangle strip
        oddStrip = true;    // three ways of exit as default
        afacePrev = aface;

        tstrip = allocator->allocTStrip();
        ++tstripCount;

    #if defined(VDPM_GEOMORPHS) && !defined(VDPM_TSTRIP_RESTRIP_ALL)
        tstrip->gtime = USHRT_MAX;
    #endif

        while (afacePrev != &afacesEnd)
        {
            noExit0 = !afacePrev->n0 || afacePrev->n0->tstrip;
            noExit1 = !afacePrev->n1 || afacePrev->n1->tstrip;
            if (noExit0 && noExit1)
            {
                indicesBuffer[0] = getVertexIndex(afacePrev->v1, tstrip);
                indicesBuffer[1] = getVertexIndex(afacePrev->v2, tstrip);
                indicesBuffer[2] = getVertexIndex(afacePrev->v0, tstrip);
                aface = afacePrev;
                afaceNext = afacePrev->n2;
                oddStrip = false;
                break;
            }
            noExit2 = !afacePrev->n2 || afacePrev->n2->tstrip;
            if (noExit0 && noExit2)
            {
                indicesBuffer[0] = getVertexIndex(afacePrev->v0, tstrip);
                indicesBuffer[1] = getVertexIndex(afacePrev->v1, tstrip);
                indicesBuffer[2] = getVertexIndex(afacePrev->v2, tstrip);
                aface = afacePrev;
                afaceNext = afacePrev->n1;
                oddStrip = false;
                break;
            }
            else if (noExit1 && noExit2)
            {
                indicesBuffer[0] = getVertexIndex(afacePrev->v2, tstrip);
                indicesBuffer[1] = getVertexIndex(afacePrev->v0, tstrip);
                indicesBuffer[2] = getVertexIndex(afacePrev->v1, tstrip);
                aface = afacePrev;
                afaceNext = afacePrev->n0;
                oddStrip = false;
                break;
            }
            else if (oddStrip)
            {
                if (noExit0)    // set n1 as exit
                {
                    indicesBuffer[0] = getVertexIndex(afacePrev->v0, tstrip);
                    indicesBuffer[1] = getVertexIndex(afacePrev->v1, tstrip);
                    indicesBuffer[2] = getVertexIndex(afacePrev->v2, tstrip);
                    aface = afacePrev;
                    afaceNext = afacePrev->n1;
                    oddStrip = false;
                }
                else if (noExit1)    // set n2 as exit
                {
                    indicesBuffer[0] = getVertexIndex(afacePrev->v1, tstrip);
                    indicesBuffer[1] = getVertexIndex(afacePrev->v2, tstrip);
                    indicesBuffer[2] = getVertexIndex(afacePrev->v0, tstrip);
                    aface = afacePrev;
                    afaceNext = afacePrev->n2;
                    oddStrip = false;
                }
                else if (noExit2)    // set n0 as exit
                {
                    indicesBuffer[0] = getVertexIndex(afacePrev->v2, tstrip);
                    indicesBuffer[1] = getVertexIndex(afacePrev->v0, tstrip);
                    indicesBuffer[2] = getVertexIndex(afacePrev->v1, tstrip);
                    aface = afacePrev;
                    afaceNext = afacePrev->n0;
                    oddStrip = false;
                }
            }
            afacePrev = afacePrev->next;
        }
        if (oddStrip)
        {
            aface = afaces.next;
            indicesBuffer[0] = getVertexIndex(aface->v0, tstrip);
            indicesBuffer[1] = getVertexIndex(aface->v1, tstrip);
            indicesBuffer[2] = getVertexIndex(aface->v2, tstrip);
            afaceNext = aface->n1;
        }
        else
            oddStrip = true;

        assert(aface->tstrip == NULL);
        assert(aface->next != NULL);

        indicesBufferTop = 2;
        aface->prev->next = aface->next;
        aface->next->prev = aface->prev;
        aface->tstrip = tstrip;
        tstrip->afaces = aface;
        updated = true;

    #ifdef VDPM_TSTRIP_SWAP
        swapped = false;
    #endif
        while (afaceNext && !afaceNext->tstrip)
        {
            if (indicesBufferTop >= indicesBufferSize - 1)
            {
                ++indicesBufferSize;
                indicesBuffer = (unsigned int*)::realloc(indicesBuffer, sizeof(unsigned int) * indicesBufferSize);
            }

        #ifdef VDPM_TSTRIP_SWAP
            if (!swapped)
        #endif
            {
                afaceNext->prev->next = afaceNext->next;
                afaceNext->next->prev = afaceNext->prev;
                aface->next = afaceNext;
                afaceNext->prev = aface;

                afacePrev = aface;
                aface = afaceNext;
            }

            if (oddStrip)
            {
                oddStrip = false;

                if (afacePrev == afaceNext->n0)
                {
                #ifdef VDPM_TSTRIP_SWAP
                    if (swapped || (afaceNext->n2 && !afaceNext->n2->tstrip))
                    {
                        i = getVertexIndex(afaceNext->v2, tstrip);
                        afaceNext = afaceNext->n2;
                        swapped = false;
                    }
                    else if (afaceNext->n1 && !afaceNext->n1->tstrip)
                    {
                        i = getVertexIndex(afaceNext->v1, tstrip);
                        swapped = true;
                    }
                    else
                    {
                #endif // VDPM_TSTRIP_SWAP
                        i = getVertexIndex(afaceNext->v2, tstrip);
                        afaceNext = afaceNext->n2;
                #ifdef VDPM_TSTRIP_SWAP
                        swapped = false;
                    }
                #endif // VDPM_TSTRIP_SWAP
                }
                else if (afacePrev == afaceNext->n1)
                {
                #ifdef VDPM_TSTRIP_SWAP
                    if (swapped || (afaceNext->n0 && !afaceNext->n0->tstrip))
                    {
                        i = getVertexIndex(afaceNext->v0, tstrip);
                        afaceNext = afaceNext->n0;
                        swapped = false;
                    }
                    else if (afaceNext->n2 && !afaceNext->n2->tstrip)
                    {
                        i = getVertexIndex(afaceNext->v2, tstrip);
                        swapped = true;
                    }
                    else
                    {
                #endif // VDPM_TSTRIP_SWAP
                        i = getVertexIndex(afaceNext->v0, tstrip);
                        afaceNext = afaceNext->n0;
                #ifdef VDPM_TSTRIP_SWAP
                        swapped = false;
                    }
                #endif // VDPM_TSTRIP_SWAP
                }
                else
                {
                    //assert(afacePrev == afaceNext->n2);
                #ifdef VDPM_TSTRIP_SWAP
                    if (swapped || (afaceNext->n2 && !afaceNext->n2->tstrip))
                    {
                        i = getVertexIndex(afaceNext->v1, tstrip);
                        afaceNext = afaceNext->n1;
                        swapped = false;
                    }
                    else if (afaceNext->n0 && !afaceNext->n0->tstrip)
                    {
                        i = getVertexIndex(afaceNext->v0, tstrip);
                        swapped = true;
                    }
                    else
                    {
                #endif // VDPM_TSTRIP_SWAP
                        i = getVertexIndex(afaceNext->v1, tstrip);
                        afaceNext = afaceNext->n1;
                #ifdef VDPM_TSTRIP_SWAP
                        swapped = false;
                    }
                #endif // VDPM_TSTRIP_SWAP
                }
            }
            else
            {
                oddStrip = true;

                if (afacePrev == afaceNext->n0)
                {
                #ifdef VDPM_TSTRIP_SWAP
                    if (swapped || (afaceNext->n1 && !afaceNext->n1->tstrip))
                    {
                        i = getVertexIndex(afaceNext->v2, tstrip);
                        afaceNext = afaceNext->n1;
                        swapped = false;
                    }
                    else if (afaceNext->n2 && !afaceNext->n2->tstrip)
                    {
                        i = getVertexIndex(afaceNext->v0, tstrip);
                        swapped = true;
                    }
                    else
                    {
                #endif // VDPM_TSTRIP_SWAP
                        i = getVertexIndex(afaceNext->v2, tstrip);
                        afaceNext = afaceNext->n1;
                #ifdef VDPM_TSTRIP_SWAP
                        swapped = false;
                    }
                #endif // VDPM_TSTRIP_SWAP
                }
                else if (afacePrev == afaceNext->n2)
                {
                #ifdef VDPM_TSTRIP_SWAP
                    if (swapped || (afaceNext->n0 && !afaceNext->n0->tstrip))
                    {
                        i = getVertexIndex(afaceNext->v1, tstrip);
                        afaceNext = afaceNext->n0;
                        swapped = false;
                    }
                    else if (afaceNext->n1 && !afaceNext->n1->tstrip)
                    {
                        i = getVertexIndex(afaceNext->v2, tstrip);
                        swapped = true;
                    }
                    else
                    {
                    #endif // VDPM_TSTRIP_SWAP
                        i = getVertexIndex(afaceNext->v1, tstrip);
                        afaceNext = afaceNext->n0;
                    #ifdef VDPM_TSTRIP_SWAP
                        swapped = false;
                        }
                    #endif // VDPM_TSTRIP_SWAP
                }
                else
                {
                    //assert(afacePrev == afaceNext->n1);

                #ifdef VDPM_TSTRIP_SWAP
                    if (swapped || (afaceNext->n2 && !afaceNext->n2->tstrip))
                    {
                        i = getVertexIndex(afaceNext->v0, tstrip);
                        afaceNext = afaceNext->n2;
                        swapped = false;
                    }
                    else if (afaceNext->n0 && !afaceNext->n0->tstrip)
                    {
                        i = getVertexIndex(afaceNext->v1, tstrip);
                        swapped = true;
                    }
                    else
                    {
                #endif // VDPM_TSTRIP_SWAP
                        i = getVertexIndex(afaceNext->v0, tstrip);
                        afaceNext = afaceNext->n2;
                #ifdef VDPM_TSTRIP_SWAP
                        swapped = false;
                    }
                #endif // VDPM_TSTRIP_SWAP
                }
            }

        #ifdef VDPM_TSTRIP_SWAP
            if (!swapped)
        #endif
            {
                aface->tstrip = tstrip;
            }
            indicesBuffer[++indicesBufferTop] = i;
        }
        aface->next = NULL;

    #if defined(VDPM_GEOMORPHS) && !defined(VDPM_TSTRIP_RESTRIP_ALL)
        if (tstrip->gtime != USHRT_MAX)
            addGMorphTStrip(tstrip);
        else
    #endif // defined(VDPM_GEOMORPHS) && !defined(VDPM_TSTRIP_RESTRIP_ALL)
            addTStrip(tstrip);

    #ifndef NDEBUG
        assertAFaces();
    #endif
        tstrip->vgCount = indicesBufferTop + 1;
        tstrip->vgIndices = new unsigned int[tstrip->vgCount];
        ::memcpy(tstrip->vgIndices, indicesBuffer, tstrip->vgCount * sizeof(unsigned int));

        aface = afaces.next;
    }

    if (updated)
    {
    #ifdef VDPM_RENDERER_OPENGL_IBO
        unsigned int index, bufsize = 0;
        unsigned int* ptr;
    #endif

        if (tstripCount > indicesArraySize)
        {
            delete[] indicesArray;
            indicesArray = new unsigned int*[tstripCount];
            indicesArraySize = tstripCount;

            delete[] indicesCountArray;
            indicesCountArray = new unsigned int[tstripCount];
        }

        i = 0;
        tstrip = tstrips.next;
        while (tstrip != &tstripsEnd)
        {
            indicesArray[i] = tstrip->vgIndices;
            indicesCountArray[i++] = tstrip->vgCount;

        #ifdef VDPM_RENDERER_OPENGL_IBO
            bufsize += sizeof(unsigned int) * tstrip->vgCount;
        #endif
            tstrip = tstrip->next;
        }

    #if defined(VDPM_GEOMORPHS) && !defined(VDPM_TSTRIP_RESTRIP_ALL)
        tstrip = gmorphTstrips.next;
        while (tstrip != &gmorphTstripsEnd)
        {
            indicesArray[i] = tstrip->vgIndices;
            indicesCountArray[i++] = tstrip->vgCount;

        #ifdef VDPM_RENDERER_OPENGL_IBO
            bufsize += sizeof(unsigned int) * tstrip->vgCount;
        #endif
            tstrip = tstrip->next;
        }
    #endif // defined(VDPM_GEOMORPHS) && !defined(VDPM_TSTRIP_RESTRIP_ALL)

    #ifdef VDPM_RENDERER_OPENGL_IBO
        if (bufsize > iboSize)
        {
            renderer->destroyBuffer(ibo);
            ibo = renderer->createBuffer(RENDERER_INDEX_BUFFER, sizeof(unsigned int) * bufsize, NULL);
            iboSize = bufsize;
        }
        ptr = (unsigned int*)renderer->mapBuffer(RENDERER_INDEX_BUFFER, ibo, 0, sizeof(unsigned int) * bufsize, RENDERER_WRITE_ONLY);
        index = 0;
        for (i = 0; i < tstripCount; ++i)
        {
            ::memcpy(ptr + index, indicesArray[i], sizeof(unsigned int) * indicesCountArray[i]);
            indicesArray[i] = (unsigned int*)(sizeof(unsigned int) * index);
            index += indicesCountArray[i];
        }
        renderer->flushBuffer(RENDERER_INDEX_BUFFER, ibo, 0, sizeof(unsigned int) * bufsize);
        renderer->unmapBuffer(RENDERER_INDEX_BUFFER, ibo);

    #endif // VDPM_RENDERER_OPENGL_IBO
    }
}

void SRMesh::draw()
{
    renderer->draw(this);
}

void SRMesh::printStatus()
{
    Log::println("vertices:");

    for (unsigned int i = 0; i < vcount; ++i)
    {
        VGeom* vgeom = getVGeom(i);
        char buf[512];

        sprintf(buf, "v[%u] p:{%f %f %f}", i, vgeom->point.x, vgeom->point.y, vgeom->point.z);

        if (hasColor())
        {
            float* vgeomColor = geometry.getVGeomColor(vgeom);
            sprintf(buf, "%s c:{%f %f %f}", buf, vgeomColor[0], vgeomColor[1], vgeomColor[2]);
        }

        if (hasTexCoord())
        {
            float* vgeomTexCoord = geometry.getVGeomTexCoord(vgeom);
            sprintf(buf, "%s t:{%f %f}", buf, vgeomTexCoord[0], vgeomTexCoord[1]);
        }
        Log::println(buf);
    }

    Log::println("active vertices:");
    AVertex *avertex, *avertexNext;

    avertex = avertices.next;
    while (avertex != &averticesEnd)
    {
        unsigned int i = avertex->i;
        VGeom* vgeom = getVGeom(i);

        avertexNext = avertex->next;

        Log::println("v[%d] p:{%f %f %f}", i, vgeom->point.x, vgeom->point.y, vgeom->point.z);

        avertex = avertexNext;
    }

    Log::println("active faces:");
    for (unsigned int i = 0; i < fcount; ++i)
    {
        if (faces[i].aface)
        {
            char buf0[128], buf1[32];

            buf0[0] = '\0';
            if (faces[i].aface->n0)
            {
                sprintf(buf1, " n0:{%d %d %d}", faces[i].aface->n0->v0->i, faces[i].aface->n0->v1->i, faces[i].aface->n0->v2->i);
                strcat(buf0, buf1);
            }
            if (faces[i].aface->n1)
            {
                sprintf(buf1, " n1:{%d %d %d}", faces[i].aface->n1->v0->i, faces[i].aface->n1->v1->i, faces[i].aface->n1->v2->i);
                strcat(buf0, buf1);
            }
            if (faces[i].aface->n2)
            {
                sprintf(buf1, " n2:{%d %d %d}", faces[i].aface->n2->v0->i, faces[i].aface->n2->v1->i, faces[i].aface->n2->v2->i);
                strcat(buf0, buf1);
            }

            Log::println("f[%d] {%d %d %d}%s", i, faces[i].aface->v0->i, faces[i].aface->v1->i, faces[i].aface->v2->i, buf0);
        }
    }

    Log::println("triangle strips:");
    TStrip *tstrip, *tstripNext;

    tstrip = tstrips.next;
    while (tstrip != &tstripsEnd)
    {
        AFace *aface, *afaceNext;
        char buf0[128], buf1[128], buf2[32];

        tstripNext = tstrip->next;

        buf0[0] = '\0';
        for (unsigned int i = 0; i < tstrip->vgCount; ++i)
        {
            sprintf(buf2, " %d", tstrip->vgIndices[i]);
            strcat(buf0, buf2);
        }

        buf1[0] = '\0';
        aface = tstrip->afaces;
        while (aface != NULL)
        {
            afaceNext = aface->next;

            sprintf(buf2, "{%d %d %d}", aface->v0->i, aface->v1->i, aface->v2->i);
            strcat(buf1, buf2);

            aface = afaceNext;
        }
        Log::println("count:%d indices:%s afaces: %s", tstrip->vgCount, buf0, buf1);

        tstrip = tstripNext;
    }
}

void SRMesh::printAVertex(AVertex* avertex)
{
    VGeom* vgeom = getVGeom(avertex->i);
    Log::println("v[%u] p:{%f %f %f}", avertex->i, vgeom->point.x, vgeom->point.y, vgeom->point.z);
}

#ifndef NDEBUG

void SRMesh::testEcol()
{
    AVertex *avertex, *avertexNext;
    unsigned int max_i = 0;

    avertex = avertices.next;
    while (avertex != &averticesEnd)
    {
        avertexNext = avertex->next;
        if (avertex->i != (unsigned int)(-1) && avertex->i > max_i && avertex->vertex->parent && ecolLegal(avertex->vertex->parent))
        {
            max_i = avertex->i;
        }
        avertex = avertexNext;
    }

    avertex = avertices.next;
    while (avertex != &averticesEnd)
    {
        avertexNext = avertex->next;
        if (avertex->i == max_i)
        {
            if (avertex->vertex->parent)
            {
            #ifdef VDPM_GEOMORPHS
                startCoarsening(avertex->vertex->parent);
            #else
                ecol(avertex->vertex->parent);
            #endif
            }
            break;
        }
        avertex = avertexNext;
    }
}

void SRMesh::testVsplit()
{
    AVertex *avertex, *avertexNext;
    unsigned int min_i = UINT_MAX;

    avertex = avertices.next;
    while (avertex != &averticesEnd)
    {
        avertexNext = avertex->next;
        if (avertex->vertex->i != -1 && avertex->vertex->i < min_i)
        {
            min_i = avertex->vertex->i;
        }
        avertex = avertexNext;
    }

    avertex = avertices.next;
    while (avertex != &averticesEnd)
    {
        avertexNext = avertex->next;
        if (avertex->vertex->i == min_i)
        {
            forceVSplit(avertex->vertex);
            break;
        }
        avertex = avertexNext;
    }
}
#endif // !NDEBUG

void SRMesh::vsplit(Vertex* vs)
{
    AFace *fn0, *fn1, *fn2, *fn3, *fl_aface, *fr_aface, *aface;
    Vertex *vt, *vu;
    Face *fl, *fr;
    AVertex *vl, *vr;
    VSplit* vsp = &vsplits[vs->i];
    unsigned int vs_vgeom_i = vs->avertex->i;

#ifndef NDEBUG
    assertAFaces();
#endif
    fn0 = (vsp->fn0) ? vsp->fn0->aface : NULL;
    fn1 = (vsp->fn1) ? vsp->fn1->aface : NULL;
    fn2 = (vsp->fn2) ? vsp->fn2->aface : NULL;
    fn3 = (vsp->fn3) ? vsp->fn3->aface : NULL;

    vt = &vertices[baseVCount + vs->i * 2];
    fl = &faces[baseFCount + vs->i * 2];
    vu = vt + 1;
    assert(!vu->avertex);
    fr = fl + 1;

    vt->avertex = vs->avertex;
    vu->avertex = allocator->allocAVertex();
    vs->avertex = NULL;
    vt->avertex->vertex = vt;
    vu->avertex->vertex = vu;
    addAVertex(vu->avertex);
    vt->avertex->i = vsp->vt_i;
    vu->avertex->i = vsp->vu_i;

    // update fn0..fn3 by current active faces
    if (fn0)
    {
        if (!fn1)
        {
            if (vt->avertex == fn0->v0)
            {
                if (fn0->n0)
                    fn1 = fn0->n0;
            }
            else if (vt->avertex == fn0->v1)
            {
                if (fn0->n1)
                    fn1 = fn0->n1;
            }
            else
            {
                if (fn0->n2)
                    fn1 = fn0->n2;
            }
        }
    }
    else if (fn1)
    {
        if (vt->avertex == fn1->v0)
        {
            if (fn1->n2)
                fn0 = fn1->n2;
        }
        else if (vt->avertex == fn1->v1)
        {
            if (fn1->n0)
                fn0 = fn1->n0;
        }
        else
        {
            if (fn1->n1)
                fn0 = fn1->n1;
        }
    }
    if (fn2)
    {
        if (!fn3)
        {
            if (vt->avertex == fn2->v0)
            {
                if (fn2->n2)
                    fn3 = fn2->n2;
            }
            else if (vt->avertex == fn2->v1)
            {
                if (fn2->n0)
                    fn3 = fn2->n0;
            }
            else
            {
                if (fn2->n1)
                    fn3 = fn2->n1;
            }
        }
    }
    else if (fn3)
    {
        if (vt->avertex == fn3->v0)
        {
            if (fn3->n0)
                fn2 = fn3->n0;
        }
        else if (vt->avertex == fn3->v1)
        {
            if (fn3->n1)
                fn2 = fn3->n1;
        }
        else
        {
            if (fn3->n2)
                fn2 = fn3->n2;
        }
    }

    assert(!fn0 || !fn1 || fn0 != fn1);
    assert(!fn0 || !fn3 || fn0 != fn3);
    assert(!fn1 || !fn2 || fn1 != fn2);
    assert(!fn0 || fn0->v0 == vt->avertex || fn0->v1 == vt->avertex || fn0->v2 == vt->avertex);
    assert(!fn1 || fn1->v0 == vt->avertex || fn1->v1 == vt->avertex || fn1->v2 == vt->avertex);
    assert(!fn2 || fn2->v0 == vt->avertex || fn2->v1 == vt->avertex || fn2->v2 == vt->avertex);
    assert(!fn3 || fn3->v0 == vt->avertex || fn3->v1 == vt->avertex || fn3->v2 == vt->avertex);

    if (fn0 || fn1)
    {
        fl->aface = fl_aface = allocator->allocAFace();
        addAFace(fl_aface);

        // find vl
        if (fn0)
        {
            if (vt->avertex == fn0->v0)
                vl = fn0->v1;
            else if (vt->avertex == fn0->v1)
                vl = fn0->v2;
            else
                vl = fn0->v0;
        }
        else
        {
            if (vt->avertex == fn1->v0)
                vl = fn1->v2;
            else if (vt->avertex == fn1->v1)
                vl = fn1->v0;
            else
                vl = fn1->v1;
        }
        // fill in entries of fl.aface
        fl_aface->v0 = vt->avertex;
        fl_aface->v1 = vu->avertex;
        fl_aface->v2 = vl;
        fl_aface->n1 = fn1;
        fl_aface->n2 = fn0;
        fl_aface->tstrip = NULL;
    }
    else
        fl_aface = NULL;

    if (fn2 || fn3)
    {
        fr->aface = fr_aface = allocator->allocAFace();
        addAFace(fr_aface);

        // find vr
        if (fn2)
        {
            if (vt->avertex == fn2->v0)
                vr = fn2->v2;
            else if (vt->avertex == fn2->v1)
                vr = fn2->v0;
            else
                vr = fn2->v1;
        }
        else
        {
            if (vt->avertex == fn3->v0)
                vr = fn3->v1;
            else if (vt->avertex == fn3->v1)
                vr = fn3->v2;
            else
                vr = fn3->v0;
        }
        // fill in entries of fr.aface
        fr_aface->v0 = vt->avertex;
        fr_aface->v1 = vr;
        fr_aface->v2 = vu->avertex;
        fr_aface->n0 = fn2;
        fr_aface->n1 = fn3;
        fr_aface->tstrip = NULL;
    }
    else
        fr_aface = NULL;

    // update fn0..3.neighbors[..] to point to fl, fr
    if (fl_aface)
    {
        fl_aface->n0 = fr_aface;

        if (fn0)
        {
            if (fn0->v0 == vl)
                fn0->n2 = fl_aface;
            else if (fn0->v1 == vl)
                fn0->n0 = fl_aface;
            else
                fn0->n1 = fl_aface;

            assert(!fn0->n0 || (fn0->n0 != fn0->n1 && fn0->n0 != fn0->n2));
            assert(!fn0->n1 || (fn0->n1 != fn0->n0 && fn0->n1 != fn0->n2));
            assert(!fn0->n2 || (fn0->n2 != fn0->n0 && fn0->n2 != fn0->n1));
        }
        if (fn1)
        {
            if (fn1->v0 == vl)
                fn1->n0 = fl_aface;
            else if (fn1->v1 == vl)
                fn1->n1 = fl_aface;
            else
                fn1->n2 = fl_aface;

            assert(!fn1->n0 || (fn1->n0 != fn1->n1 && fn1->n0 != fn1->n2));
            assert(!fn1->n1 || (fn1->n1 != fn1->n0 && fn1->n1 != fn1->n2));
            assert(!fn1->n2 || (fn1->n2 != fn1->n0 && fn1->n2 != fn1->n1));
        }
        assert(!fn0 || !fn1 || (fn0->n0 != fn1 && fn0->n1 != fn1 && fn0->n2 != fn1));
        assert(!fn0 || !fn1 || (fn1->n0 != fn0 && fn1->n1 != fn0 && fn1->n2 != fn0));
    }

    if (fr_aface)
    {
        fr_aface->n2 = fl_aface;

        if (fn2)
        {
            if (fn2->v0 == vr)
                fn2->n0 = fr_aface;
            else if (fn2->v1 == vr)
                fn2->n1 = fr_aface;
            else
                fn2->n2 = fr_aface;

            assert(!fn2->n0 || (fn2->n0 != fn2->n1 && fn2->n0 != fn2->n2));
            assert(!fn2->n1 || (fn2->n1 != fn2->n0 && fn2->n1 != fn2->n2));
            assert(!fn2->n2 || (fn2->n2 != fn2->n0 && fn2->n2 != fn2->n1));
        }
        if (fn3)
        {
            if (fn3->v0 == vr)
                fn3->n2 = fr_aface;
            else if (fn3->v1 == vr)
                fn3->n0 = fr_aface;
            else
                fn3->n1 = fr_aface;

            assert(!fn3->n0 || (fn3->n0 != fn3->n1 && fn3->n0 != fn3->n2));
            assert(!fn3->n1 || (fn3->n1 != fn3->n0 && fn3->n1 != fn3->n2));
            assert(!fn3->n2 || (fn3->n2 != fn3->n0 && fn3->n2 != fn3->n1));
        }
        assert(!fn2 || !fn3 || (fn2->n0 != fn3 && fn2->n1 != fn3 && fn2->n2 != fn3));
        assert(!fn2 || !fn3 || (fn3->n0 != fn2 && fn3->n1 != fn2 && fn3->n2 != fn2));
    }

    assert(!fl_aface || !fr_aface || fl_aface->n0 == fr_aface);
    assert(!fl_aface || !fr_aface || fr_aface->n2 == fl_aface);
    assert(!fn0 || !fn2 || fn0 == fn2 || !fr_aface || (fn0->n0 != fr_aface && fn0->n1 != fr_aface && fn0->n2 != fr_aface));
    assert(!fn1 || !fn3 || fn1 == fn3 || !fr_aface || (fn1->n0 != fr_aface && fn1->n1 != fr_aface && fn1->n2 != fr_aface));
    assert(!fn0 || !fn2 || fn0 == fn2 || !fl_aface || (fn2->n0 != fl_aface && fn2->n1 != fl_aface && fn2->n2 != fl_aface));
    assert(!fn1 || !fn3 || fn1 == fn3 || !fl_aface || (fn3->n0 != fl_aface && fn3->n1 != fl_aface && fn3->n2 != fl_aface));
#ifndef NDEBUG
    assertAFaceNeighbors(fl_aface);
    assertAFaceNeighbors(fr_aface);
    assertAFaces();
#endif

    // update vertices of each face around vs to point to vu
    // update tirangles strip of each face around vs
    aface = NULL;
    if (fl_aface)
    {
        aface = fn1;

        while (aface && aface != fr_aface)
        {
        #ifndef VDPM_TSTRIP_RESTRIP_ALL
            if (aface->tstrip)
            {
                allocator->freeTStrip(aface->tstrip, afaces);
                --tstripCount;
            }
        #endif
            if (aface->v0 == vt->avertex)
            {
                aface->v0 = vu->avertex;
                aface = aface->n0;
            }
            else if (aface->v1 == vt->avertex)
            {
                aface->v1 = vu->avertex;
                aface = aface->n1;
            }
            else if (aface->v2 == vt->avertex)
            {
                aface->v2 = vu->avertex;
                aface = aface->n2;
            }
            else
            {
                assert(0);
                break;
            }
        }
    }

    if (fr_aface && aface != fr_aface)
    {
        assert(!aface);
        aface = fn3;

        while (aface && aface)
        {
        #ifndef VDPM_TSTRIP_RESTRIP_ALL
            if (aface->tstrip)
            {
                allocator->freeTStrip(aface->tstrip, afaces);
                --tstripCount;
            }
        #endif
            if (aface->v0 == vt->avertex)
            {
                aface->v0 = vu->avertex;
                aface = aface->n2;
            }
            else if (aface->v1 == vt->avertex)
            {
                aface->v1 = vu->avertex;
                aface = aface->n0;
            }
            else if (aface->v2 == vt->avertex)
            {
                aface->v2 = vu->avertex;
                aface = aface->n1;
            }
            else
            {
                assert(aface->v0 == vu->avertex || aface->v1 == vu->avertex || aface->v2 == vu->avertex);
                break;
            }
        }
    }

#ifndef VDPM_TSTRIP_RESTRIP_ALL
    aface = NULL;
    if (fl_aface)
    {
        aface = fn0;

        while (aface && aface != fr_aface)
        {
            if (aface->tstrip)
            {
                allocator->freeTStrip(aface->tstrip, afaces);
                --tstripCount;
            }

            if (aface->v0 == vt->avertex)
                aface = aface->n2;
            else if (aface->v1 == vt->avertex)
                aface = aface->n0;
            else if (aface->v2 == vt->avertex)
                aface = aface->n1;
            else
            {
                assert(0);
                break;
            }
        }
    }

    if (fr_aface && aface != fr_aface)
    {
        assert(!aface);
        aface = fn2;

        while (aface)
        {
            if (aface->tstrip)
            {
                allocator->freeTStrip(aface->tstrip, afaces);
                --tstripCount;
            }

            if (aface->v0 == vt->avertex)
                aface = aface->n0;
            else if (aface->v1 == vt->avertex)
                aface = aface->n1;
            else if (aface->v2 == vt->avertex)
                aface = aface->n2;
            else
            {
                assert(0);
                break;
            }
        }
    }
#endif // !VDPM_TSTRIP_RESTRIP_ALL
    assert(!fn0 || !fl_aface || fn0->v0 != vt->avertex || fn0->n0 == fl_aface);
    assert(!fn0 || !fl_aface || fn0->v1 != vt->avertex || fn0->n1 == fl_aface);
    assert(!fn0 || !fl_aface || fn0->v2 != vt->avertex || fn0->n2 == fl_aface);
    assert(!fn1 || !fl_aface || fn1->v0 != vu->avertex || fn1->n2 == fl_aface);
    assert(!fn1 || !fl_aface || fn1->v1 != vu->avertex || fn1->n0 == fl_aface);
    assert(!fn1 || !fl_aface || fn1->v2 != vu->avertex || fn1->n1 == fl_aface);
    assert(!fn2 || !fr_aface || fn2->v0 != vt->avertex || fn2->n2 == fr_aface);
    assert(!fn2 || !fr_aface || fn2->v1 != vt->avertex || fn2->n0 == fr_aface);
    assert(!fn2 || !fr_aface || fn2->v2 != vt->avertex || fn2->n1 == fr_aface);
    assert(!fn3 || !fr_aface || fn3->v0 != vu->avertex || fn3->n0 == fr_aface);
    assert(!fn3 || !fr_aface || fn3->v1 != vu->avertex || fn3->n1 == fr_aface);
    assert(!fn3 || !fr_aface || fn3->v2 != vu->avertex || fn3->n2 == fr_aface);
    assert(!fn0 || fn0->v0 == vt->avertex || fn0->v1 == vt->avertex || fn0->v2 == vt->avertex);
    assert(!fn1 || fn1->v0 == vu->avertex || fn1->v1 == vu->avertex || fn1->v2 == vu->avertex);
    assert(!fn2 || fn2->v0 == vt->avertex || fn2->v1 == vt->avertex || fn2->v2 == vt->avertex);
    assert(!fn3 || fn3->v0 == vu->avertex || fn3->v1 == vu->avertex || fn3->v2 == vu->avertex);
#ifndef NDEBUG
    assertAFaceNeighbors(fl_aface);
    assertAFaceNeighbors(fr_aface);
    assertAFaces();
#endif

#ifdef VDPM_GEOMORPHS
    // geomorphs of vt, vu
    VMorph* vm_t = vt->avertex->vmorph;

    if (outsideViewFrustum(vs)
    #ifdef VDPM_ORIENTED_AWAY
        || orientedAway(vs)
    #endif
        )
    {
        if (vm_t)
        {
            assert(vm_t->avertex == vt->avertex);

            if (vm_t->coarsening)
            {
                Vertex* v = (vs->parent == (vs + 1)->parent) ? vs + 1 : vs - 1;
                if (v->avertex)
                {
                    VMorph* vmorph = v->avertex->vmorph;
                    if (vmorph)
                    {
                        assert(vmorph->avertex == v->avertex);

                        if (outsideViewFrustum(vs->parent)
                        #ifdef VDPM_ORIENTED_AWAY
                            || orientedAway(vs->parent)
                        #endif
                            )
                        {
                            removeVMorph(vmorph);
                        }
                        else
                        {
                            VGeom* vgRefined = getVGeom(v->avertex->i);
                            VGeom* vgeom = getVMorphVGeom(vmorph->vgIndex);
                            vmorph->coarsening = false;
                            vmorph->gtime = gtime - vmorph->gtime;
                            vmorph->vgInc.point = (vgRefined->point - vgeom->point) / vmorph->gtime;
                            vmorph->vgInc.normal = (vgRefined->normal - vgeom->normal) / vmorph->gtime;

                            if (hasColor())
                            {
                                float* vgIncColor = geometry.getVGeomColor(&vmorph->vgInc);
                                float* vgRefinedColor = geometry.getVGeomColor(vgRefined);
                                float* vgeomColor = geometry.getVGeomColor(vgeom);

                                for (int i = 0; i < 3; ++i)
                                    vgIncColor[i] = (vgRefinedColor[i] - vgeomColor[i]) / vmorph->gtime;
                            }

                            if (hasTexCoord())
                            {
                                float* vgIncTexCoord = geometry.getVGeomTexCoord(&vmorph->vgInc);
                                float* vgRefinedTexCoord = geometry.getVGeomTexCoord(vgRefined);
                                float* vgeomTexCoord = geometry.getVGeomTexCoord(vgeom);

                                for (int i = 0; i < 2; ++i)
                                    vgIncTexCoord[i] = (vgRefinedTexCoord[i] - vgeomTexCoord[i]) / vmorph->gtime;
                            }
                        }
                    }
                }
            }
            removeVMorph(vm_t);
        }
        assert(!vu->avertex->vmorph);
        assert(!vt->avertex->vmorph);
    }
    else
    {
        VMorph* vm_u;
        VGeom *vt_vgeom, *vu_vgeom;
        bool pass = false;

        if (vm_t)
        {
            if (vm_t->coarsening)
            {
                Vertex* v = (vs->parent == (vs + 1)->parent) ? vs + 1 : vs - 1;
                if (v->avertex)
                {
                    VMorph* vmorph = v->avertex->vmorph;
                    if (vmorph)
                    {
                        VGeom*  vgRefined = getVGeom(v->avertex->i);
                        VGeom* vgeom = getVMorphVGeom(vmorph->vgIndex);

                        vmorph->coarsening = false;
                        vmorph->gtime = gtime - vmorph->gtime;
                        vmorph->vgInc.point = (vgRefined->point - vgeom->point) / vmorph->gtime;
                        vmorph->vgInc.normal = (vgRefined->normal - vgeom->normal) / vmorph->gtime;

                        if (hasColor())
                        {
                            float* vgIncColor = geometry.getVGeomColor(&vmorph->vgInc);
                            float* vgRefinedColor = geometry.getVGeomColor(vgRefined);
                            float* vgeomColor = geometry.getVGeomColor(vgeom);

                            for (int i = 0; i < 3; ++i)
                                vgIncColor[i] = (vgRefinedColor[i] - vgeomColor[i]) / vmorph->gtime;
                        }

                        if (hasTexCoord())
                        {
                            float* vgIncTexCoord = geometry.getVGeomTexCoord(&vmorph->vgInc);
                            float* vgRefinedTexCoord = geometry.getVGeomTexCoord(vgRefined);
                            float* vgeomTexCoord = geometry.getVGeomTexCoord(vgeom);

                            for (int i = 0; i < 2; ++i)
                                vgIncTexCoord[i] = (vgRefinedTexCoord[i] - vgeomTexCoord[i]) / vmorph->gtime;
                        }
                    }
                }
            }
        }
        else
        {
            vt->avertex->vmorph = vm_t = createVMorph();
            vt_vgeom = getVMorphVGeom(vm_t->vgIndex);
            ::memcpy(vt_vgeom, getVGeom(vs_vgeom_i), geometry.vgeomSize);
            vm_t->avertex = vt->avertex;
        }
        vu->avertex->vmorph = vm_u = createVMorph();
        vu_vgeom = getVMorphVGeom(vm_u->vgIndex);
        ::memcpy(vu_vgeom, getVGeom(vs_vgeom_i), geometry.vgeomSize);
        vm_u->avertex = vu->avertex;

        vt_vgeom = getVMorphVGeom(vm_t->vgIndex); // get pointer again after possible realloc

        VGeom* vtRefined = getVGeom(vt->avertex->i);
        VGeom* vuRefined = getVGeom(vu->avertex->i);

#ifdef VDPM_GEOMORPHS_PLUS
        if (!fr)
        {
            if (!fn0)
                calcVGeomOnLine(vtRefined, vu_vgeom, getVGeom(vl->i), vt_vgeom);
            else if (!fn1)
                calcVGeomOnLine(vuRefined, vt_vgeom, getVGeom(vl->i), vu_vgeom);

            pass = true;
        }
        else if (!fl)
        {
            if (!fn2)
                calcVGeomOnLine(vtRefined, vu_vgeom, getVGeom(vr->i), vt_vgeom);
            else if (!fn3)
                calcVGeomOnLine(vuRefined, vt_vgeom, getVGeom(vr->i), vu_vgeom);

            pass = true;
        }
        else if (fn1)
        {
            if (fn1 == fn3)
            {
                calcVGeomOnFace(vuRefined, vt_vgeom, getVGeom(vl->i), getVGeom(vr->i), vu_vgeom);
                pass = true;
            }
            else if (fn3)
            {
                if (fn1->n0 == fn3)
                {
                    calcVGeomOnLine(vuRefined, vt_vgeom, getVGeom(fn1->v1->i), vu_vgeom);
                    pass = true;
                }
                else if (fn1->n1 == fn3)
                {
                    calcVGeomOnLine(vuRefined, vt_vgeom, getVGeom(fn1->v2->i), vu_vgeom);
                    pass = true;
                }
                else if (fn1->n2 == fn3)
                {
                    calcVGeomOnLine(vuRefined, vt_vgeom, getVGeom(fn1->v0->i), vu_vgeom);
                    pass = true;
                }
            }
        }
        if (!pass && fn0)
        {
            if (fn0 == fn2)
            {
                calcVGeomOnFace(vtRefined, vu_vgeom, getVGeom(vr->i), getVGeom(vl->i), vt_vgeom);
            }
            else if (fn2)
            {
                if (fn0->n0 == fn2)
                    calcVGeomOnLine(vtRefined, vu_vgeom, getVGeom(fn0->v2->i), vt_vgeom);
                else if (fn0->n1 == fn2)
                    calcVGeomOnLine(vtRefined, vu_vgeom, getVGeom(fn0->v0->i), vt_vgeom);
                else if (fn0->n2 == fn2)
                    calcVGeomOnLine(vtRefined, vu_vgeom, getVGeom(fn0->v1->i), vt_vgeom);
            }
        }

#endif // VDPM_GEOMORPHS_PLUS

        vm_t->coarsening = false;
        vm_t->gtime = gtime;
        vm_t->vgInc.point = (vtRefined->point - vt_vgeom->point) / gtime;
        vm_t->vgInc.normal = (vtRefined->normal - vt_vgeom->normal) / gtime;

        vm_u->coarsening = false;
        vm_u->gtime = gtime;
        vm_u->vgInc.point = (vuRefined->point - vu_vgeom->point) / gtime;
        vm_u->vgInc.normal = (vuRefined->normal - vu_vgeom->normal) / gtime;

        if (hasColor())
        {
            float* vgIncColor = geometry.getVGeomColor(&vm_t->vgInc);
            float* vgRefinedColor = geometry.getVGeomColor(vtRefined);
            float* vgeomColor = geometry.getVGeomColor(vt_vgeom);

            for (int i = 0; i < 3; ++i)
                vgIncColor[i] = (vgRefinedColor[i] - vgeomColor[i]) / gtime;

            vgIncColor = geometry.getVGeomColor(&vm_u->vgInc);
            vgRefinedColor = geometry.getVGeomColor(vuRefined);
            vgeomColor = geometry.getVGeomColor(vu_vgeom);

            for (int i = 0; i < 3; ++i)
                vgIncColor[i] = (vgRefinedColor[i] - vgeomColor[i]) / gtime;
        }

        if (hasTexCoord())
        {
            float* vgIncTexCoord = geometry.getVGeomTexCoord(&vm_t->vgInc);
            float* vgRefinedTexCoord = geometry.getVGeomTexCoord(vtRefined);
            float* vgeomTexCoord = geometry.getVGeomTexCoord(vt_vgeom);

            for (int i = 0; i < 2; ++i)
                vgIncTexCoord[i] = (vgRefinedTexCoord[i] - vgeomTexCoord[i]) / gtime;

            vgIncTexCoord = geometry.getVGeomTexCoord(&vm_u->vgInc);
            vgRefinedTexCoord = geometry.getVGeomTexCoord(vuRefined);
            vgeomTexCoord = geometry.getVGeomTexCoord(vu_vgeom);

            for (int i = 0; i < 2; ++i)
                vgIncTexCoord[i] = (vgRefinedTexCoord[i] - vgeomTexCoord[i]) / gtime;
        }
    }
#endif // VDPM_GEOMORPHS

#ifdef VDPM_TSTRIP_RESTRIP_ALL
    tstripDirty = true;
#endif
}

void SRMesh::ecol(Vertex* vs)
{
    AFace *aface, *fn0, *fn1, *fn2, *fn3, *fl_aface, *fr_aface;
    Vertex *vt, *vu;
    Face *fl, *fr;

    vt = &vertices[baseVCount + vs->i * 2];
    vu = vt + 1;

#ifdef VDPM_GEOMORPHS
    assert(!vu->avertex->vmorph);
#endif
    fl = &faces[baseFCount + vs->i * 2];
    fr = fl + 1;
    fl_aface = fl->aface;
    fr_aface = fr->aface;

    assert(!fl_aface || !fr_aface || fl_aface->n0 == fr_aface);
    assert(!fl_aface || !fr_aface || fr_aface->n2 == fl_aface);

#ifndef NDEBUG
    assertAFaceNeighbors(fl_aface);
    assertAFaceNeighbors(fr_aface);
    assertAFaces();
#endif

    // update vertices of each face around vu to point to vs
    // update tirangles strip of each face around vu
    aface = NULL;
    if (fl_aface)
    {
        if (fl_aface->tstrip)
        {
            allocator->freeTStrip(fl_aface->tstrip, afaces);
            --tstripCount;
        }

        aface = fl_aface->n1;
        while (aface && aface != fr_aface)
        {
        #ifndef VDPM_TSTRIP_RESTRIP_ALL
            if (aface->tstrip)
            {
                allocator->freeTStrip(aface->tstrip, afaces);
                --tstripCount;
            }
        #endif

            if (aface->v0 == vu->avertex)
            {
                aface->v0 = vt->avertex;
                aface = aface->n0;
            }
            else if (aface->v1 == vu->avertex)
            {
                aface->v1 = vt->avertex;
                aface = aface->n1;
            }
            else if (aface->v2 == vu->avertex)
            {
                aface->v2 = vt->avertex;
                aface = aface->n2;
            }
            else
            {
                assert(0);
                break;
            }
        }
    }
    if (fr_aface && aface != fr_aface)
    {
        aface = fr_aface->n1;
        while (aface)
        {
        #ifndef VDPM_TSTRIP_RESTRIP_ALL
            if (aface->tstrip)
            {
                allocator->freeTStrip(aface->tstrip, afaces);
                --tstripCount;
            }
        #endif
            if (aface->v0 == vu->avertex)
            {
                aface->v0 = vt->avertex;
                aface = aface->n2;
            }
            else if (aface->v1 == vu->avertex)
            {
                aface->v1 = vt->avertex;
                aface = aface->n0;
            }
            else if (aface->v2 == vu->avertex)
            {
                aface->v2 = vt->avertex;
                aface = aface->n1;
            }
            else
            {
                assert(aface->v0 == vt->avertex || aface->v1 == vt->avertex || aface->v2 == vt->avertex);
                break;
            }
        }
    }

    // update tirangles strip of each face around vt
    aface = NULL;
    if (fr_aface)
    {
        if (fr_aface->tstrip)
        {
            allocator->freeTStrip(fr_aface->tstrip, afaces);
            --tstripCount;
        }

    #ifndef VDPM_TSTRIP_RESTRIP_ALL
        aface = fr_aface->n0;
        while (aface && aface != fl_aface)
        {
            if (aface->tstrip)
            {
                allocator->freeTStrip(aface->tstrip, afaces);
                --tstripCount;
            }

            if (aface->v0 == vt->avertex)
                aface = aface->n0;
            else if (aface->v1 == vt->avertex)
                aface = aface->n1;
            else if (aface->v2 == vt->avertex)
                aface = aface->n2;
            else
            {
                assert(0);
                break;
            }
        }
    #endif // !VDPM_TSTRIP_RESTRIP_ALL
    }
#ifndef VDPM_TSTRIP_RESTRIP_ALL
    if (fl_aface && aface != fl_aface)
    {
        assert(!aface);
        aface = fl_aface->n2;
        while (aface)
        {
            if (aface->tstrip)
            {
                allocator->freeTStrip(aface->tstrip, afaces);
                --tstripCount;
            }

            if (aface->v0 == vt->avertex)
                aface = aface->n2;
            else if (aface->v1 == vt->avertex)
                aface = aface->n0;
            else if (aface->v2 == vt->avertex)
                aface = aface->n1;
            else
            {
                assert(0);
                break;
            }
        }
    }
#endif // !VDPM_TSTRIP_RESTRIP_ALL

    // update neighbors of fl and fr to point to one another; free aface of fl, fr
#ifndef NDEBUG
    assertAFaces();
#endif
    if (fl_aface)
    {
        fn0 = fl_aface->n2;
        fn1 = fl_aface->n1;

        assert(!fn0 || fn0 != fn1);
        assert(!fn1 || fn1 != fn0);

        if (fn0)
        {
            if (fn0->n0 == fl->aface)
                fn0->n0 = fn1;
            else if (fn0->n1 == fl->aface)
                fn0->n1 = fn1;
            else
                fn0->n2 = fn1;

            assert(!fn0->n0 || (fn0->n0 != fn0->n1 && fn0->n0 != fn0->n2));
            assert(!fn0->n1 || (fn0->n1 != fn0->n0 && fn0->n1 != fn0->n2));
            assert(!fn0->n2 || (fn0->n2 != fn0->n0 && fn0->n2 != fn0->n1));
        }

        if (fn1)
        {
            if (fn1->n0 == fl->aface)
                fn1->n0 = fn0;
            else if (fn1->n1 == fl->aface)
                fn1->n1 = fn0;
            else
                fn1->n2 = fn0;

            assert(!fn1->n0 || (fn1->n0 != fn1->n1 && fn1->n0 != fn1->n2));
            assert(!fn1->n1 || (fn1->n1 != fn1->n0 && fn1->n1 != fn1->n2));
            assert(!fn1->n2 || (fn1->n2 != fn1->n0 && fn1->n2 != fn1->n1));
        }
        --afaceCount;
        allocator->freeAFace(fl_aface);
        fl->aface = NULL;

        assert(!fn0 || fn0->v0 == vt->avertex || fn0->v1 == vt->avertex || fn0->v2 == vt->avertex);
        assert(!fn1 || fn1->v0 == vt->avertex || fn1->v1 == vt->avertex || fn1->v2 == vt->avertex);
    }

    if (fr_aface)
    {
        fn2 = fr_aface->n0;
        fn3 = fr_aface->n1;

        assert(!fn2 || fn2 != fn3);
        assert(!fn3 || fn3 != fn2);

        if (fn2)
        {
            if (fn2->n0 == fr->aface)
                fn2->n0 = fn3;
            else if (fn2->n1 == fr->aface)
                fn2->n1 = fn3;
            else
                fn2->n2 = fn3;

            assert(!fn2->n0 || (fn2->n0 != fn2->n1 && fn2->n0 != fn2->n2));
            assert(!fn2->n1 || (fn2->n1 != fn2->n0 && fn2->n1 != fn2->n2));
            assert(!fn2->n2 || (fn2->n2 != fn2->n0 && fn2->n2 != fn2->n1));
        }
        if (fn3)
        {
            if (fn3->n0 == fr->aface)
                fn3->n0 = fn2;
            else if (fn3->n1 == fr->aface)
                fn3->n1 = fn2;
            else
                fn3->n2 = fn2;

            assert(!fn3->n0 || (fn3->n0 != fn3->n1 && fn3->n0 != fn3->n2));
            assert(!fn3->n1 || (fn3->n1 != fn3->n0 && fn3->n1 != fn3->n2));
            assert(!fn3->n2 || (fn3->n2 != fn3->n0 && fn3->n2 != fn3->n1));
        }
        --afaceCount;
        allocator->freeAFace(fr_aface);
        fr->aface = NULL;

        assert(!fn2 || fn2->v0 == vt->avertex || fn2->v1 == vt->avertex || fn2->v2 == vt->avertex);
        assert(!fn3 || fn3->v0 == vt->avertex || fn3->v1 == vt->avertex || fn3->v2 == vt->avertex);
    }
#ifndef NDEBUG
    assertAFaces();
#endif
    vs->avertex = vt->avertex;
    --avertexCount;

#ifdef VDPM_AMORTIZATION
    if (vu->avertex == amortizeAvertex)
        amortizeAvertex = vu->avertex->next;

#endif // VDPM_AMORTIZATION

    allocator->freeAVertex(vu->avertex);

    vu->avertex = NULL;
    vt->avertex = NULL;
    vs->avertex->vertex = vs;
    vs->avertex->i = getVGeomIndex(vs);

#ifdef VDPM_TSTRIP_RESTRIP_ALL
    tstripDirty = true;
#endif
}

void SRMesh::forceVSplit(Vertex* v)
{
    int vstackTop = 0;

    vstack[vstackTop] = v;

    while (vstackTop >= 0)
    {
        Vertex* vs;
        Face*fl;

        vs = vstack[vstackTop];
        fl = &faces[baseFCount + vs->i * 2];

        if (fl->aface || (fl + 1)->aface)
        {
            --vstackTop;
            continue;
        }

        if (vstackTop >= vstackSize - 4)
        {
            vstackSize += 4;
            vstack = (VertexPointer*)::realloc(vstack, sizeof(VertexPointer) * vstackSize);
        }

        if (!vs->avertex)
        {
            vstack[++vstackTop] = vs->parent;
            assert(vstack[vstackTop]);
        }
        else if (vsplitLegal(vs))
        {
            --vstackTop;
            vsplit(vs);

        #ifdef VDPM_REGULATION_FORCE
            if (afaceCount >= targetAFaceCount)
                return;
        #endif

            continue;
        }
        else
        {
            Face *fn0, *fn1, *fn2, *fn3;

            fn0 = vsplits[vs->i].fn0;
            if (fn0 && !fn0->aface)
            {
                vstack[++vstackTop] = vertices[baseVCount + (fn0 - &faces[baseFCount])].parent;
                assert(vstack[vstackTop]);
            }

            fn1 = vsplits[vs->i].fn1;
            if (fn1 && !fn1->aface)
            {
                vstack[++vstackTop] = vertices[baseVCount + (fn1 - &faces[baseFCount])].parent;
                assert(vstack[vstackTop]);
            }

            fn2 = vsplits[vs->i].fn2;
            if (fn2 && !fn2->aface && fn2 != fn0)
            {
                vstack[++vstackTop] = vertices[baseVCount + (fn2 - &faces[baseFCount])].parent;
                assert(vstack[vstackTop]);
            }

            fn3 = vsplits[vs->i].fn3;
            if (fn3 && !fn3->aface && fn3 != fn1)
            {
                vstack[++vstackTop] = vertices[baseVCount + (fn3 - &faces[baseFCount])].parent;
                assert(vstack[vstackTop]);
            }
        }
    }
}

bool SRMesh::outsideViewFrustum(Vertex* vs)
{
    Point* point;
    unsigned int p;

    assert(vs->i != UINT_MAX);

    if (vs->avertex)
        point = &getVGeom(vs->avertex->i)->point;
    else
        point = &getVGeom(getVGeomIndex(vs))->point;

    for (p = 0; p < 6; ++p)
    {
        float d = viewport->frustum[p][0] * point->x + viewport->frustum[p][1] * point->y + viewport->frustum[p][2] * point->z + viewport->frustum[p][3];
        if (d <= -vsplits[vs->i].radius)
            return true;
    }
    return false;
}

#ifdef VDPM_ORIENTED_AWAY
bool SRMesh::orientedAway(Vertex* vs)
{
    VGeom* vs_geom;
    Point v_e;
    AVertex* avertex = vs->avertex;
    float result;

    if (avertex)
        vs_geom = getVGeom(avertex->i);
    else
        vs_geom = getVGeom(getVGeomIndex(vs));

#ifdef VDPM_PREDICT_VIEW_POSITION
    v_e = vs_geom->point - viewport->predictViewPos;
#else
    v_e = vs_geom->point - viewport->viewPos;
#endif

    result = dotProduct(v_e, vs_geom->normal);
    if (result > 0.0f)
    {
        result *= result;
        if (result > dotProduct(v_e, v_e) * vsplits[vs->i].sin2alpha)
            return true;
    }
    return false;
}
#endif // VDPM_ORIENTED_AWAY

bool SRMesh::screenErrorIllegal(Vertex* vs)
{
    VGeom* vs_geom;
    Point v_e;
    AVertex* avertex = vs->avertex;
    VSplit& vsp = vsplits[vs->i];
    float lv2, ve_n;

    if (avertex)
    {
        vs_geom = getVGeom(avertex->i);
    }
    else
    {
        vs_geom = getVGeom(getVGeomIndex(vs));
    }
#ifdef VDPM_PREDICT_VIEW_POSITION
    v_e = vs_geom->point - viewport->predictViewPos;
#else
    v_e = vs_geom->point - viewport->viewPos;
#endif

#ifdef VDPM_SCREEN_ERROR_STRICT
    lv2 = dotProduct(v_e, v_e) - vsp.radius;    // more strict
#else
    lv2 = dotProduct(v_e, v_e);
#endif

    if (vsp.uni_error >= kappa2 * lv2)
        return true;

    ve_n = dotProduct(v_e, vs_geom->normal);

    if (vsp.dir_error * (lv2 - ve_n * ve_n) >= kappa2 * lv2 * lv2)
        return true;

    return false;
}

bool SRMesh::vsplitLegal(Vertex* vs)
{
    VSplit* vsp = &vsplits[vs->i];

    if ((vsp->fn0 && !vsp->fn0->aface) || (vsp->fn1 && !vsp->fn1->aface) ||
        (vsp->fn2 && !vsp->fn2->aface) || (vsp->fn3 && !vsp->fn3->aface))
        return false;

    return true;
}

bool SRMesh::ecolLegal(Vertex* vs)
{
    Vertex* vt = &vertices[baseVCount + vs->i * 2];

    if (!vt->avertex || !(vt + 1)->avertex)
        return false;

    Face* fl = &faces[baseFCount + vs->i * 2];
    VSplit* vsp = &vsplits[vs->i];

    if (fl->aface)
    {
        AFace *fn0, *fn1;

        if (vsp->fn0)
        {
            fn0 = vsp->fn0->aface;
            if (!fn0 || fl->aface->n2 != fn0)
                return false;
        }
        if (vsp->fn1)
        {
            fn1 = vsp->fn1->aface;
            if (!fn1 || fl->aface->n1 != fn1)
                return false;
        }
    }

    AFace* afr = (fl + 1)->aface;

    if (afr)
    {
        AFace *fn2, *fn3;

        if (vsp->fn2)
        {
            fn2 = vsp->fn2->aface;
            if (!fn2 || afr->n0 != fn2)
                return false;
        }
        if (vsp->fn3)
        {
            fn3 = vsp->fn3->aface;
            if (!fn3 || afr->n1 != fn3)
                return false;
        }
    }
    return true;
}

#ifdef VDPM_GEOMORPHS

void SRMesh::startCoarsening(Vertex* vs)
{
    Vertex *vt, *vu;
    VGeom *vt_vgeom, *vu_vgeom, *vt_goalVGeom, *vu_goalVGeom;
    VMorph *vm_t, *vm_u;

    vt = &vertices[baseVCount + vs->i * 2];
    vu = vt + 1;
    vm_t = vt->avertex->vmorph;
    vt_vgeom = getVGeom(vt->avertex->i);
    if (!vm_t)
    {
        VGeom* vmorphVgeom;
        vt->avertex->vmorph = vm_t = createVMorph();
        vmorphVgeom = getVMorphVGeom(vm_t->vgIndex);
        ::memcpy(vmorphVgeom, vt_vgeom, geometry.vgeomSize);
        vm_t->avertex = vt->avertex;
    }

    vm_u = vu->avertex->vmorph;
    vu_vgeom = getVGeom(vu->avertex->i);
    if (!vm_u)
    {
        VGeom* vmorphVgeom;
        vu->avertex->vmorph = vm_u = createVMorph();
        vmorphVgeom = getVMorphVGeom(vm_u->vgIndex);
        ::memcpy(vmorphVgeom, vu_vgeom, geometry.vgeomSize);
        vm_u->avertex = vu->avertex;
    }
    vt_goalVGeom = vu_goalVGeom = getVGeom(getVGeomIndex(vs));

#ifdef VDPM_GEOMORPHS_PLUS
    {
        AFace *fn0, *fn1, *fn2, *fn3, *aface;
        bool vt_notBound, vu_notBound;

        fn0 = (vsplits[vs->i].fn0) ? vsplits[vs->i].fn0->aface : NULL;
        fn1 = (vsplits[vs->i].fn1) ? vsplits[vs->i].fn1->aface : NULL;
        fn2 = (vsplits[vs->i].fn2) ? vsplits[vs->i].fn2->aface : NULL;
        fn3 = (vsplits[vs->i].fn3) ? vsplits[vs->i].fn3->aface : NULL;

        // update fn0..fn3 by current active faces
        if (fn0)
        {
            if (!fn1)
            {
                if (vt->avertex == fn0->v0)
                {
                    if (fn0->n1)
                        fn1 = fn0->n1;
                }
                else if (vt->avertex == fn0->v1)
                {
                    if (fn0->n2)
                        fn1 = fn0->n2;
                }
                else
                {
                    if (fn0->n0)
                        fn1 = fn0->n0;
                }
            }
        }
        else if (fn1)
        {
            if (vt->avertex == fn1->v0)
            {
                if (fn1->n2)
                    fn0 = fn1->n2;
            }
            else if (vt->avertex == fn1->v1)
            {
                if (fn1->n0)
                    fn0 = fn1->n0;
            }
            else
            {
                if (fn1->n1)
                    fn0 = fn1->n1;
            }
        }
        if (fn2)
        {
            if (!fn3)
            {
                if (vt->avertex == fn2->v0)
                {
                    if (fn2->n2)
                        fn3 = fn2->n2;
                }
                else if (vt->avertex == fn2->v1)
                {
                    if (fn2->n0)
                        fn3 = fn2->n0;
                }
                else
                {
                    if (fn2->n1)
                        fn3 = fn2->n1;
                }
            }
        }
        else if (fn3)
        {
            if (vt->avertex == fn3->v0)
            {
                if (fn3->n1)
                    fn2 = fn3->n1;
            }
            else if (vt->avertex == fn3->v1)
            {
                if (fn3->n2)
                    fn2 = fn3->n2;
            }
            else
            {
                if (fn3->n0)
                    fn2 = fn3->n0;
            }
        }

        // determine vt, vu are boundary vertices or not
        if (!fn0 || !fn2)
            vt_notBound = false;
        else
        {
            aface = fn0;
            while (aface && aface != fn2)
            {
                if (aface->v0 == vt->avertex)
                    aface = aface->n2;
                else if (aface->v1 == vt->avertex)
                    aface = aface->n0;
                else if (aface->v2 == vt->avertex)
                    aface = aface->n1;
                else
                    break;
            }
            vt_notBound = (aface == fn2) ? true : false;
        }
        if (!fn1 || !fn3)
            vu_notBound = false;
        else
        {
            aface = fn1;
            while (aface && aface != fn3)
            {
                if (aface->v0 == vu->avertex)
                    aface = aface->n1;
                else if (aface->v1 == vu->avertex)
                    aface = aface->n2;
                else if (aface->v2 == vu->avertex)
                    aface = aface->n0;
                else
                    break;
            }
            vu_notBound = (aface == fn3) ? true : false;
        }

        if (vt_notBound || vu_notBound)
        {
            AVertex *vl, *vr;
            bool pass = false;

            // find vl, vr
            if (fn0 || fn1)
            {
                if (fn0)
                {
                    if (vt->avertex == fn0->v0)
                        vl = fn0->v2;
                    else if (vt->avertex == fn0->v1)
                        vl = fn0->v0;
                    else
                        vl = fn0->v1;
                }
                else
                {
                    if (vt->avertex == fn1->v0)
                        vl = fn1->v1;
                    else if (vt->avertex == fn1->v1)
                        vl = fn1->v2;
                    else
                        vl = fn1->v0;
                }
            }
            if (fn2 || fn3)
            {
                if (fn2)
                {
                    if (vt->avertex == fn2->v0)
                        vr = fn2->v1;
                    else if (vt->avertex == fn2->v1)
                        vr = fn2->v2;
                    else
                        vr = fn2->v0;
                }
                else
                {
                    if (vt->avertex == fn3->v0)
                        vr = fn3->v2;
                    else if (vt->avertex == fn3->v1)
                        vr = fn3->v0;
                    else
                        vr = fn3->v1;
                }
            }

            if (vl->vmorph || vr->vmorph)
                pass = true;
            else if (!fn2 && !fn3)
            {
                if (!fn1)
                    calcVGeomOnLine(vt_vgeom, vu_goalVGeom, getVGeom(vl->i), vt_goalVGeom);
                else if (!fn0)
                    calcVGeomOnLine(vu_vgeom, vt_goalVGeom, getVGeom(vl->i), vu_goalVGeom);

                pass = true;
            }
            else if (!fn0 && !fn1)
            {
                if (!fn2)
                    calcVGeomOnLine(vt_vgeom, vu_goalVGeom, getVGeom(vr->i), vt_goalVGeom);
                else if (!fn3)
                    calcVGeomOnLine(vu_vgeom, vt_goalVGeom, getVGeom(vr->i), vu_goalVGeom);

                pass = true;
            }
            else if (fn1 && vu_notBound)
            {
                if (fn1 == fn3)
                {
                    calcVGeomOnFace(vu_vgeom, vt_goalVGeom, getVGeom(vl->i), getVGeom(vr->i), vu_goalVGeom);
                    pass = true;
                }
                else if (fn3)
                {
                    if (fn1->n0 == fn3)
                    {
                        if (!fn1->v1->vmorph)
                            calcVGeomOnLine(vu_vgeom, vt_goalVGeom, getVGeom(fn1->v1->i), vu_goalVGeom);

                        pass = true;
                    }
                    else if (fn1->n1 == fn3)
                    {
                        if (!fn1->v2->vmorph)
                            calcVGeomOnLine(vu_vgeom, vt_goalVGeom, getVGeom(fn1->v2->i), vu_goalVGeom);

                        pass = true;
                    }
                    else if (fn1->n2 == fn3)
                    {
                        if (!fn1->v0->vmorph)
                            calcVGeomOnLine(vu_vgeom, vt_goalVGeom, getVGeom(fn1->v0->i), vu_goalVGeom);

                        pass = true;
                    }
                }
            }
            if (!pass && fn0 && vt_notBound)
            {
                if (fn0 == fn2)
                {
                    calcVGeomOnFace(vt_vgeom, vu_goalVGeom, getVGeom(vr->i), getVGeom(vl->i), vt_goalVGeom);
                }
                else if (fn2)
                {
                    if (fn0->n0 == fn2 && !fn0->v2->vmorph)
                        calcVGeomOnLine(vt_vgeom, vu_goalVGeom, getVGeom(fn0->v2->i), vt_goalVGeom);
                    else if (fn0->n1 == fn2 && !fn0->v0->vmorph)
                        calcVGeomOnLine(vt_vgeom, vu_goalVGeom, getVGeom(fn0->v0->i), vt_goalVGeom);
                    else if (fn0->n2 == fn2 && !fn0->v1->vmorph)
                        calcVGeomOnLine(vt_vgeom, vu_goalVGeom, getVGeom(fn0->v1->i), vt_goalVGeom);
                }
            }
        }
    }
#endif // VDPM_GEOMORPHS_PLUS

    vm_t->gtime = gtime >> 1;    // gtime / 2
    vm_t->coarsening = true;
    vm_t->vgInc.point = (vt_goalVGeom->point - vt_vgeom->point) / vm_t->gtime;
    vm_t->vgInc.normal = (vt_goalVGeom->normal - vt_vgeom->normal) / vm_t->gtime;

    vm_u->gtime = vm_t->gtime;
    vm_u->coarsening = true;
    vm_u->vgInc.point = (vu_goalVGeom->point - vu_vgeom->point) / vm_u->gtime;
    vm_u->vgInc.normal = (vu_goalVGeom->normal - vu_vgeom->normal) / vm_u->gtime;

    if (hasColor())
    {
        float* vgIncColor = geometry.getVGeomColor(&vm_t->vgInc);
        float* goalVGeomColor = geometry.getVGeomColor(vt_goalVGeom);
        float* vgeomColor = geometry.getVGeomColor(vt_vgeom);

        for (int i = 0; i < 3; ++i)
            vgIncColor[i] = (goalVGeomColor[i] - vgeomColor[i]) / vm_t->gtime;

        vgIncColor = geometry.getVGeomColor(&vm_u->vgInc);
        goalVGeomColor = geometry.getVGeomColor(vu_goalVGeom);
        vgeomColor = geometry.getVGeomColor(vu_vgeom);

        for (int i = 0; i < 3; ++i)
            vgIncColor[i] = (goalVGeomColor[i] - vgeomColor[i]) / vm_t->gtime;
    }

    if (hasTexCoord())
    {
        float* vgIncTexCoord = geometry.getVGeomTexCoord(&vm_t->vgInc);
        float* goalVGeomTexCoord = geometry.getVGeomTexCoord(vt_goalVGeom);
        float* vgeomTexCoord = geometry.getVGeomTexCoord(vt_vgeom);

        for (int i = 0; i < 2; ++i)
            vgIncTexCoord[i] = (goalVGeomTexCoord[i] - vgeomTexCoord[i]) / vm_t->gtime;

        vgIncTexCoord = geometry.getVGeomTexCoord(&vm_u->vgInc);
        goalVGeomTexCoord = geometry.getVGeomTexCoord(vu_goalVGeom);
        vgeomTexCoord = geometry.getVGeomTexCoord(vu_vgeom);

        for (int i = 0; i < 2; ++i)
            vgIncTexCoord[i] = (goalVGeomTexCoord[i] - vgeomTexCoord[i]) / vm_t->gtime;
    }

#ifdef VDPM_TSTRIP_RESTRIP_ALL
    tstripDirty = true;

#else
    {
        AFace *aface, *fl_aface, *fr_aface;
        Face *fl, *fr;

        fl = &faces[baseFCount + vs->i * 2];
        fr = fl + 1;
        fl_aface = fl->aface;
        fr_aface = fr->aface;

        aface = NULL;
        if (fl_aface)
        {
            if (fl_aface->tstrip)
            {
                allocator->freeTStrip(fl_aface->tstrip, afaces);
                --tstripCount;
            }

            aface = fl_aface->n1;
            while (aface && aface != fr_aface)
            {
                if (aface->tstrip)
                {
                    allocator->freeTStrip(aface->tstrip, afaces);
                    --tstripCount;
                }

                if (aface->v0 == vu->avertex)
                {
                    aface = aface->n0;
                }
                else if (aface->v1 == vu->avertex)
                {
                    aface = aface->n1;
                }
                else if (aface->v2 == vu->avertex)
                {
                    aface = aface->n2;
                }
                else
                {
                    assert(0);
                    break;
                }
            }
        }
        if (fr_aface && aface != fr_aface)
        {
            aface = fr_aface->n1;
            while (aface)
            {
                if (aface->tstrip)
                {
                    allocator->freeTStrip(aface->tstrip, afaces);
                    --tstripCount;
                }

                if (aface->v0 == vu->avertex)
                {
                    aface = aface->n2;
                }
                else if (aface->v1 == vu->avertex)
                {
                    aface = aface->n0;
                }
                else if (aface->v2 == vu->avertex)
                {
                    aface = aface->n1;
                }
                else
                {
                    assert(aface->v0 == vt->avertex || aface->v1 == vt->avertex || aface->v2 == vt->avertex);
                    break;
                }
            }
        }

        aface = NULL;
        if (fr_aface)
        {
            if (fr_aface->tstrip)
            {
                allocator->freeTStrip(fr_aface->tstrip, afaces);
                --tstripCount;
            }

            aface = fr_aface->n0;
            while (aface && aface != fl_aface)
            {
                if (aface->tstrip)
                {
                    allocator->freeTStrip(aface->tstrip, afaces);
                    --tstripCount;
                }

                if (aface->v0 == vt->avertex)
                    aface = aface->n0;
                else if (aface->v1 == vt->avertex)
                    aface = aface->n1;
                else if (aface->v2 == vt->avertex)
                    aface = aface->n2;
                else
                {
                    assert(0);
                    break;
                }
            }
        }

        if (fl_aface && aface != fl_aface)
        {
            assert(!aface);
            aface = fl_aface->n2;
            while (aface)
            {
                if (aface->tstrip)
                {
                    allocator->freeTStrip(aface->tstrip, afaces);
                    --tstripCount;
                }

                if (aface->v0 == vt->avertex)
                    aface = aface->n2;
                else if (aface->v1 == vt->avertex)
                    aface = aface->n0;
                else if (aface->v2 == vt->avertex)
                    aface = aface->n1;
                else
                {
                    assert(0);
                    break;
                }
            }
        }
    }
#endif // VDPM_TSTRIP_RESTRIP_ALL
}

bool SRMesh::finishCoarsening(AVertex* avertex)
{
    Vertex* vt = avertex->vertex;
    VMorph* vmorph;
    Vertex* vu;

    if (vt->parent == (vt + 1)->parent)
        vu = vt + 1;
    else
        vu = vt - 1;

    vmorph = vu->avertex->vmorph;
    if (vmorph)
    {
        assert(vmorph->avertex == vu->avertex);

        if (vmorph->gtime > 0)
            return false;

        removeVMorph(vmorph);
        assert(!vu->avertex->vmorph);
    }

    vmorph = avertex->vmorph;
    if (vmorph)
    {
        assert(vmorph->avertex == avertex);

        removeVMorph(vmorph);
        assert(!avertex->vmorph);
    }
    return true;
}

void SRMesh::abortCoarsening(AVertex* avertex)
{
    VMorph* vmorph = avertex->vmorph;
    VGeom* vgRefined = getVGeom(avertex->i);
    VGeom* vgeom = getVMorphVGeom(vmorph->vgIndex);
    Vertex* v = avertex->vertex;

    vmorph->coarsening = false;
    vmorph->gtime = gtime - vmorph->gtime;
    vmorph->vgInc.point = (vgRefined->point - vgeom->point) / vmorph->gtime;
    vmorph->vgInc.normal = (vgRefined->normal - vgeom->normal) / vmorph->gtime;

    if (hasColor())
    {
        float* vgIncColor = geometry.getVGeomColor(&vmorph->vgInc);
        float* vgRefinedColor = geometry.getVGeomColor(vgRefined);
        float* vgeomColor = geometry.getVGeomColor(vgeom);

        for (int i = 0; i < 3; ++i)
            vgIncColor[i] = (vgRefinedColor[i] - vgeomColor[i]) / vmorph->gtime;
    }

    if (hasTexCoord())
    {
        float* vgIncTexCoord = geometry.getVGeomTexCoord(&vmorph->vgInc);
        float* vgRefinedTexCoord = geometry.getVGeomTexCoord(vgRefined);
        float* vgeomTexCoord = geometry.getVGeomTexCoord(vgeom);

        for (int i = 0; i < 2; ++i)
            vgIncTexCoord[i] = (vgRefinedTexCoord[i] - vgeomTexCoord[i]) / vmorph->gtime;
    }

    if (v->parent == (v + 1)->parent)
        ++v;
    else
        --v;

    if (v->avertex && (vmorph = v->avertex->vmorph))
    {
        vgRefined = getVGeom(v->avertex->i);
        vgeom = getVMorphVGeom(vmorph->vgIndex);
        vmorph->coarsening = false;
        vmorph->gtime = gtime - vmorph->gtime;
        vmorph->vgInc.point = (vgRefined->point - vgeom->point) / vmorph->gtime;
        vmorph->vgInc.normal = (vgRefined->normal - vgeom->normal) / vmorph->gtime;

        if (hasColor())
        {
            float* vgIncColor = geometry.getVGeomColor(&vmorph->vgInc);
            float* vgRefinedColor = geometry.getVGeomColor(vgRefined);
            float* vgeomColor = geometry.getVGeomColor(vgeom);

            for (int i = 0; i < 3; ++i)
                vgIncColor[i] = (vgRefinedColor[i] - vgeomColor[i]) / vmorph->gtime;
        }

        if (hasTexCoord())
        {
            float* vgIncTexCoord = geometry.getVGeomTexCoord(&vmorph->vgInc);
            float* vgRefinedTexCoord = geometry.getVGeomTexCoord(vgRefined);
            float* vgeomTexCoord = geometry.getVGeomTexCoord(vgeom);

            for (int i = 0; i < 2; ++i)
                vgIncTexCoord[i] = (vgRefinedTexCoord[i] - vgeomTexCoord[i]) / vmorph->gtime;
        }
    }
}
#endif // VDPM_GEOMORPHS

#ifndef NDEBUG

void SRMesh::assertAVertices()
{
    AVertex* avertex = avertices.next;

#ifdef VDPM_AMORTIZATION
    bool amortizeAFaceFound = false;
#endif

    while (avertex != &averticesEnd)
    {
        assert(avertex->prev != NULL);
        assert(avertex->next != NULL);
        assert(avertex->next != (void*)0xCDCDCDCD);

#ifdef VDPM_AMORTIZATION
        if (avertex == amortizeAvertex)
            amortizeAFaceFound = true;
#endif
        avertex = avertex->next;
    }
#ifdef VDPM_AMORTIZATION
    assert(amortizeAFaceFound);
#endif
}

void SRMesh::assertAFaces()
{
    AFace* aface = afaces.next;

    while (aface != &afacesEnd)
    {
    #ifndef VDPM_TSTRIP_RESTRIP_ALL
        assert(aface->tstrip == NULL);
    #endif
        assert(aface->prev != NULL);
        assert(aface->next != NULL);
        assert(aface->next != (void*)0xCDCDCDCD);
        assert(!aface->n0 || aface->n0->prev != NULL);
        assert(!aface->n1 || aface->n1->prev != NULL);
        assert(!aface->n2 || aface->n2->prev != NULL);

        aface = aface->next;
    }

    TStrip* tstrip = tstrips.next;
    while (tstrip != &tstripsEnd)
    {
        AFace* aface;
        for (aface = tstrip->afaces; aface->next; aface = aface->next)
        {
            assert(aface->tstrip == tstrip);
            assert(aface->next != (void*)0xCDCDCDCD);
            assert(!aface->n0 || aface->n0->prev != NULL);
            assert(!aface->n1 || aface->n1->prev != NULL);
            assert(!aface->n2 || aface->n2->prev != NULL);
        }
        assert(aface->tstrip);
        tstrip = tstrip->next;
    }
}

void SRMesh::assertAFaceNeighbors(AFace* aface)
{
    if (!aface)
        return;

    if (aface->n0)
    {
        unsigned int count = 0;

        if (aface->n0->n0 == aface)
            count++;
        if (aface->n0->n1 == aface)
            count++;
        if (aface->n0->n2 == aface)
            count++;

        assert(count == 1);
    }

    if (aface->n1)
    {
        unsigned int count = 0;

        if (aface->n1->n0 == aface)
            count++;
        if (aface->n1->n1 == aface)
            count++;
        if (aface->n1->n2 == aface)
            count++;

        assert(count == 1);
    }

    if (aface->n2)
    {
        unsigned int count = 0;

        if (aface->n2->n0 == aface)
            count++;
        if (aface->n2->n1 == aface)
            count++;
        if (aface->n2->n2 == aface)
            count++;

        assert(count == 1);
    }

    AFace* af = afaces.next;
    while (af != &afacesEnd)
    {
        unsigned int count = 0;

        assert(!af->n0 || (af->n0 != af->n1 && af->n0 != af->n2));
        assert(!af->n1 || (af->n1 != af->n0 && af->n1 != af->n2));
        assert(!af->n2 || (af->n2 != af->n0 && af->n2 != af->n1));

        if (af->n0 == aface)
            count++;
        if (af->n1 == aface)
            count++;
        if (af->n2 == aface)
            count++;

        assert(count <= 3);

        af = af->next;
    }
}

#ifdef VDPM_GEOMORPHS

void SRMesh::assertVMorphs()
{
    VMorph* vmorph = vmorphs.next;

    while (vmorph != &vmorphsEnd)
    {
        assert(vmorph->avertex != NULL);
        assert(vmorph->prev != NULL);
        assert(vmorph->next != NULL);
        assert(vmorph->next != (void*)0xCDCDCDCD);
        assert(vmorph->avertex->prev);
        assert(vmorph->avertex->vmorph == vmorph);

        vmorph = vmorph->next;
    }
}
#endif // VDPM_GEOMORPHS

#endif // !NDEBUG

void SRMesh::addAVertex(AVertex* avertex)
{
    ++avertexCount;
    avertex->next = avertices.next;
    avertices.next->prev = avertex;
    avertex->prev = &avertices;
    avertices.next = avertex;
}

void SRMesh::addAFace(AFace* aface)
{
    ++afaceCount;
    aface->next = afaces.next;
    afaces.next->prev = aface;
    aface->prev = &afaces;
    afaces.next = aface;
}

void SRMesh::addTStrip(TStrip* tstrip)
{
    tstrip->next = tstrips.next;
    tstrips.next->prev = tstrip;
    tstrip->prev = &tstrips;
    tstrips.next = tstrip;
}

unsigned int SRMesh::getVGeomIndex(Vertex* vs)
{
    div_t result;

    if (vs->parent)
    {
        result = div(vs - &vertices[baseVCount], 2);
        if (result.rem)
            return vsplits[result.quot].vu_i;
        else
            return vsplits[result.quot].vt_i;
    }
    else
        return vs - vertices;
}

inline unsigned int SRMesh::getVertexIndex(AVertex* av, TStrip* tstrip)
{
#ifdef VDPM_GEOMORPHS
    VMorph* vmorph = av->vmorph;
    if (vmorph)
    {
    #ifndef VDPM_TSTRIP_RESTRIP_ALL
        if (tstrip->gtime > vmorph->gtime)
            tstrip->gtime = vmorph->gtime;
    #endif // !VDPM_TSTRIP_RESTRIP_ALL
        return vmorph->vgIndex;
    }
#endif // VDPM_GEOMORPHS
    return av->i;
}

#ifdef VDPM_GEOMORPHS

VMorph* SRMesh::createVMorph()
{
    VMorph* vmorph = allocator->allocVMorph();
    vmorph->vgIndex = getFreeVMorphIndex();
    vmorph->next = vmorphs.next;
    vmorphs.next->prev = vmorph;
    vmorph->prev = &vmorphs;
    vmorphs.next = vmorph;
    ++vmorphCount;
    return vmorph;
}

void SRMesh::removeVMorph(VMorph* vmorph)
{
    freeVMorphIndex(vmorph->vgIndex - vcount);
    vmorph->avertex->vmorph = NULL;
    allocator->freeVMorph(vmorph);
    --vmorphCount;

#ifdef VDPM_TSTRIP_RESTRIP_ALL
    tstripDirty = true;
#endif
}

unsigned int SRMesh::getFreeVMorphIndex()
{
    unsigned int i, size;

#ifndef VDPM_RENDERER_OPENGL_VBO_MAP_VGEOM
    if (!vmorphVgeoms)
        vmorphVgeoms = (VGeom*)renderer->mapBuffer(RENDERER_VERTEX_BUFFER, geometry.vbo, geometry.vgeomSize * vcount, geometry.vgeomSize * vmorphSize, RENDERER_WRITE_ONLY);
#endif // !VDPM_RENDERER_OPENGL_VBO_MAP_VGEOM

    for (i = 0; i < vmorphSize; ++i)
    {
        if (isVMorphIndexFree(i))
            return vcount + i;
    }

    renderer->flushBuffer(RENDERER_VERTEX_BUFFER, geometry.vbo, geometry.vgeomSize * vcount, vmorphSize);
    renderer->unmapBuffer(RENDERER_VERTEX_BUFFER, geometry.vbo);
    size = vmorphSize;
    vmorphSize *= 2;
    geometry.resize(geometry.vgeomCount + vmorphSize);
#ifdef VDPM_RENDERER_OPENGL_VBO_MAP_VGEOM
    vmorphVgeoms = getVGeom(vcount);
#else
    vmorphVgeoms = (VGeom*)renderer->mapBuffer(RENDERER_VERTEX_BUFFER, geometry.vbo, geometry.vgeomSize * vcount, vmorphSize, RENDERER_WRITE_ONLY);
#endif // VDPM_RENDERER_OPENGL_VBO_MAP_VGEOM

    for (i = size; i < vmorphSize; ++i)
    {
        freeVMorphIndex(i);
    }
    return vcount + size;
}

void SRMesh::freeVMorphIndex(unsigned int index)
{
    VGeom* vmorphVgeom = (VGeom*)((uint8_t*)vmorphVgeoms + geometry.vgeomSize * index);
    *(unsigned int*)&vmorphVgeom->point.x = UINT32_MAX;
}

bool SRMesh::isVMorphIndexFree(unsigned int index)
{
    VGeom* vmorphVgeom = (VGeom*)((uint8_t*)vmorphVgeoms + geometry.vgeomSize * index);
    return *(unsigned int*)&vmorphVgeom->point.x == UINT32_MAX;
}

VGeom* SRMesh::getVMorphVGeom(unsigned int index)
{
    VGeom* vmorphVgeom = (VGeom*)((uint8_t*)vmorphVgeoms + geometry.vgeomSize * (index - vcount));
    return vmorphVgeom;
}

void SRMesh::addGMorphTStrip(TStrip* tstrip)
{
    tstrip->next = gmorphTstrips.next;
    gmorphTstrips.next->prev = tstrip;
    tstrip->prev = &gmorphTstrips;
    gmorphTstrips.next = tstrip;
}

#ifdef VDPM_GEOMORPHS_PLUS

void SRMesh::calcVGeomOnLine(const VGeom* vu_vgeom, const VGeom* vl_vgeom, const VGeom* vr_vgeom, VGeom* vgeom)
{
    Point vec_lu, vec_lr;
    float lambda, result;

    vec_lu = vu_vgeom->point - vl_vgeom->point;
    vec_lr = vr_vgeom->point - vl_vgeom->point;
    result = dotProduct(vec_lr, vec_lr);
    lambda = (result) ? dotProduct(vec_lu, vec_lr) / result : 0.0f;

    if (lambda <= 0.0f)
    {
        ::memcpy(vgeom, vl_vgeom, geometry.vgeomSize);
    }
    else if (lambda >= 1.0f)
    {
        ::memcpy(vgeom, vr_vgeom, geometry.vgeomSize);
    }
    else
    {
        vgeom->point = vl_vgeom->point + lambda * vec_lr;
        vgeom->normal = vl_vgeom->normal + lambda * (vr_vgeom->normal - vl_vgeom->normal);

        if (hasColor())
        {
            float* vgeomColor = geometry.getVGeomColor(vgeom);
            float* vlVgeomColor = geometry.getVGeomColor(vl_vgeom);
            float* vrVgeomColor = geometry.getVGeomColor(vr_vgeom);

            for (int i = 0; i < 3; ++i)
                vgeomColor[i] = vlVgeomColor[i] + lambda * (vrVgeomColor[i] - vlVgeomColor[i]);
        }

        if (hasTexCoord())
        {
            float* vgeomTexCoord = geometry.getVGeomTexCoord(vgeom);
            float* vlVgeomTexCoord = geometry.getVGeomTexCoord(vl_vgeom);
            float* vrVgeomTexCoord = geometry.getVGeomTexCoord(vr_vgeom);

            for (int i = 0; i < 2; ++i)
                vgeomTexCoord[i] = vlVgeomTexCoord[i] + lambda * (vrVgeomTexCoord[i] - vlVgeomTexCoord[i]);
        }
    }
}

void SRMesh::calcVGeomOnFace(const VGeom* vu_vgeom, const VGeom* vs_vgeom, const VGeom* vl_vgeom, const VGeom* vr_vgeom, VGeom* vgeom)
{
    Point vec_sl, vec_sr, vec_su, faceNormal;
    float lambda, result, dot00, dot01, dot02, dot11, dot12, invDenom, u, v;

    vec_sl = vl_vgeom->point - vs_vgeom->point;
    vec_sr = vr_vgeom->point - vs_vgeom->point;
    faceNormal = crossProduct(vec_sl, vec_sr);
    result = dotProduct(faceNormal, faceNormal);
    lambda = (result) ? (dotProduct(faceNormal, vs_vgeom->point) -
        dotProduct(faceNormal, vu_vgeom->point)) / result : 0.0f;
    vgeom->point = vu_vgeom->point + lambda * faceNormal;
    vec_su = vgeom->point - vs_vgeom->point;

    // compute dot products
    dot00 = dotProduct(vec_sr, vec_sr);
    dot01 = dotProduct(vec_sr, vec_sl);
    dot02 = dotProduct(vec_sr, vec_su);
    dot11 = dotProduct(vec_sl, vec_sl);
    dot12 = dotProduct(vec_sl, vec_su);

    // compute barycentric coordinates
    invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    if (u >= 0.0f)
    {
        if (v >= 0.0f)
        {
            if (u + v <= 1.0f)
            {
                // inside triangle
                vgeom->normal = vs_vgeom->normal + u * (vr_vgeom->normal - vs_vgeom->normal) + v * (vl_vgeom->normal - vs_vgeom->normal);

                if (hasColor())
                {
                    float* vgeomColor = geometry.getVGeomColor(vgeom);
                    float* vsVgeomColor = geometry.getVGeomColor(vs_vgeom);
                    float* vrVgeomColor = geometry.getVGeomColor(vr_vgeom);
                    float* vlVgeomColor = geometry.getVGeomColor(vl_vgeom);

                    for (int i = 0; i < 3; ++i)
                        vgeomColor[i] = vsVgeomColor[i] + u * (vrVgeomColor[i] - vsVgeomColor[i]) + v * (vlVgeomColor[i] - vsVgeomColor[i]);
                }

                if (hasTexCoord())
                {
                    float* vgeomTexCoord = geometry.getVGeomTexCoord(vgeom);
                    float* vsVgeomTexCoord = geometry.getVGeomTexCoord(vs_vgeom);
                    float* vrVgeomTexCoord = geometry.getVGeomTexCoord(vr_vgeom);
                    float* vlVgeomTexCoord = geometry.getVGeomTexCoord(vl_vgeom);

                    for (int i = 0; i < 2; ++i)
                        vgeomTexCoord[i] = vsVgeomTexCoord[i] + u * (vrVgeomTexCoord[i] - vsVgeomTexCoord[i]) + v * (vlVgeomTexCoord[i] - vsVgeomTexCoord[i]);
                }
            }
            else
            {
                // outside line vl_vr
                calcVGeomOnLine(vu_vgeom, vl_vgeom, vr_vgeom, vgeom);
            }
        }
        else
        {
            if (u + v <= 1.0f)
            {
                // outside line vs_vr
                calcVGeomOnLine(vu_vgeom, vs_vgeom, vr_vgeom, vgeom);
            }
            else
            {
                // outside vr
                ::memcpy(vgeom, vr_vgeom, geometry.vgeomSize);
            }
        }
    }
    else
    {
        if (v >= 0.0f)
        {
            if (u + v <= 1.0f)
            {
                // outside line vl_vs
                calcVGeomOnLine(vu_vgeom, vl_vgeom, vs_vgeom, vgeom);
            }
            else
            {
                // outside vl
                ::memcpy(vgeom, vl_vgeom, geometry.vgeomSize);
            }
        }
        else
        {
            // outside vs
            ::memcpy(vgeom, vs_vgeom, geometry.vgeomSize);
        }
    }
}
#endif // VDPM_GEOMORPHS_PLUS
#endif // VDPM_GEOMORPHS
