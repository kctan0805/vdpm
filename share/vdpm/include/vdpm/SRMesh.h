#ifndef VDPM_SRMESH_H
#define VDPM_SRMESH_H

#include <cstdint>
#include "vdpm/Types.h"
#include "vdpm/Geometry.h"

namespace vdpm
{
    class SRMesh
    {
        friend class Serializer;

    public:
        ~SRMesh();

        int realize(Renderer* renderer);
        void setViewport(Viewport* viewport);
        void setViewAngle(float fov);
        void setTau(float tau);

    #ifdef VDPM_REGULATION
        void setTargetAFaceCount(unsigned int count);
    #endif

    #ifdef VDPM_AMORTIZATION
        void setAmortizeStep(unsigned int step);
    #endif

    #ifdef VDPM_GEOMORPHS
        void setGTime(unsigned int gtime);
        void updateVMorphs();
        unsigned int getVMorphCount() { return vmorphCount; };
    #endif
        void updateViewport();
        void adaptRefine();
        void updateScene();
        void draw();
        void printStatus();
        void printAVertex(AVertex* avertex);

    #ifdef VDPM_BOUNDS
        const Bounds& getBounds() { return bounds; }
    #endif
        float getTau() { return tau; };
        unsigned int getVertexCount() { return vcount; };
        unsigned int getAFaceCount() { return afaceCount; };
        unsigned int getTStripCount() { return tstripCount; };

        void* getArrayBuffer() { return geometry.vbo; }

    #ifdef VDPM_RENDERER_OPENGL_IBO
        void* getElementArrayBuffer() { return ibo; }
    #endif
        float* getVertexPointer() { return geometry.getVertexPointer(); }
        float* getNormalPointer() { return geometry.getNormalPointer(); }
        float* getColorPointer() { return geometry.getColorPointer(); }
        float* getTexCoordPointer() { return geometry.getTexCoordPointer(); }
        unsigned int** getIndicesPointer() { return indicesArray; }
        unsigned int* getIndicesCountPointer() { return indicesCountArray; }
        unsigned int getVGeomSize() { return geometry.vgeomSize; }
        unsigned int getColorOffset() { return geometry.colorOffset; }
        unsigned int getTexCoordOffset() { return geometry.texCoordOffset; }

        const char* getTextureName() { return texname; }
        bool hasColor() { return geometry.hasColor; }
        bool hasTexCoord() { return geometry.hasTexCoord; }

    #ifndef NDEBUG
        void testEcol();
        void testVsplit();
    #endif // !NDEBUG

    protected:
        SRMesh();
        void vsplit(Vertex* vs);
        void ecol(Vertex* vs);
        void forceVSplit(Vertex* v);
        bool outsideViewFrustum(Vertex* vs);
    #ifdef VDPM_ORIENTED_AWAY
        bool orientedAway(Vertex* vs);
    #endif
        bool screenErrorIllegal(Vertex* vs);
        bool vsplitLegal(Vertex* vs);
        bool ecolLegal(Vertex* vs);
    #ifdef VDPM_GEOMORPHS
        void startCoarsening(Vertex* vs);
        bool finishCoarsening(AVertex* avertex);
        void abortCoarsening(AVertex* avertex);
    #endif // VDPM_GEOMORPHS

        void addAVertex(AVertex* avertex);
        void addAFace(AFace* aface);

    #ifndef NDEBUG
        void assertAVertices();
        void assertAFaces();
        void assertAFaceNeighbors(AFace* aface);
    #ifdef VDPM_GEOMORPHS
        void assertVMorphs();
    #endif // VDPM_GEOMORPHS
    #endif // !NDEBUG

        Vertex** vstack;
        unsigned int* indicesBuffer;
        unsigned int** indicesArray;
        unsigned int* indicesCountArray;

#ifdef VDPM_RENDERER_OPENGL_IBO
        void* ibo;
        unsigned int iboSize;
#endif

        Vertex* vertices;
        Face* faces;
        VSplit* vsplits;
        AVertex avertices, averticesEnd;
        AFace afaces, afacesEnd;
        TStrip tstrips, tstripsEnd;

#ifdef VDPM_TSTRIP_RESTRIP_ALL
        bool tstripDirty;
#endif

#ifdef VDPM_GEOMORPHS
        VMorph  vmorphs, vmorphsEnd;
        unsigned short gtime;
        unsigned int vmorphCount, vmorphSize;
        TStrip gmorphTstrips, gmorphTstripsEnd;
        VGeom* vmorphVgeoms;
#endif
        unsigned int vcount, fcount, baseVCount, baseFCount, vsplitCount, avertexCount, tstripCount, afaceCount, indicesArraySize, indicesBufferSize;
        int vstackSize;
        float tanPhi, kappa2, tau;

#ifdef VDPM_REGULATION
        float targetTau;
        unsigned int targetAFaceCount;
#endif

#ifdef VDPM_AMORTIZATION
        AVertex* amortizeAvertex;
        unsigned int amortizeBudget, amortizeCount, amortizeStep;
#endif

#ifdef VDPM_BOUNDS
        Bounds bounds;
#endif
        char* texname;
        Geometry geometry;
        Allocator* allocator;
        Renderer* renderer;
        Viewport* viewport;

    private:
        VGeom* getVGeom(unsigned int i) { return geometry.getVGeom(i); }
        void addTStrip(TStrip* tstrip);
        unsigned int getVGeomIndex(Vertex* vs);
        inline unsigned int getVertexIndex(AVertex* av, TStrip* tstrip);
    #ifdef VDPM_GEOMORPHS
        VMorph* createVMorph();
        void removeVMorph(VMorph* vmorph);
        unsigned int getFreeVMorphIndex();
        void freeVMorphIndex(unsigned int index);
        bool isVMorphIndexFree(unsigned int index);
        VGeom* getVMorphVGeom(unsigned int index);
        void addGMorphTStrip(TStrip* tstrip);

    #ifdef VDPM_GEOMORPHS_PLUS
        void calcVGeomOnLine(const VGeom* vu_vgeom, const VGeom* vl_vgeom, const VGeom* vr_vgeom, VGeom* vgeom);
        void calcVGeomOnFace(const VGeom* vu_vgeom, const VGeom* vs_vgeom, const VGeom* vl_vgeom, const VGeom* vr_vgeom, VGeom* vgeom);
    #endif // VDPM_GEOMORPHS_PLUS
    #endif // VDPM_GEOMORPHS
    };
} // namespace vdpm

#endif // VDPM_SRMESH_H
