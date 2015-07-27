#ifndef VDPM_GEOMETRY_H
#define VDPM_GEOMETRY_H

#include "vdpm/Types.h"
#include "vdpm/Renderer.h"

namespace vdpm
{
    class Geometry
    {
        friend class SRMesh;

    public:
        int create(unsigned int count, bool hasColor, bool hasTexCoord);
        void destroy();
        int realize(Renderer* renderer);
        int resize(unsigned int count);

    #ifdef VDPM_RENDERER_OPENGL_VBO_MAP_VGEOM
        void mapVGeom();
        void unmapVGeom();
    #endif // VDPM_RENDERER_OPENGL_VBO_MAP_VGEOM

        float* getVertexPointer() { return &vgeoms->point.x; }
        float* getNormalPointer() { return &vgeoms->normal.x; }
        float* getColorPointer() { return hasColor ? (float*)((uint8_t*)vgeoms + colorOffset) : 0; }
        float* getTexCoordPointer() { return hasTexCoord ? (float*)((uint8_t*)vgeoms + texCoordOffset) : 0; }
        VGeom* getVGeom(unsigned int index) { return (VGeom*)((uint8_t*)vgeoms + vgeomSize * index); }
        float* getVGeomColor(const VGeom* vgeom) { return (float*)((uint8_t*)vgeom + colorOffset); }
        float* getVGeomTexCoord(const VGeom* vgeom) { return (float*)((uint8_t*)vgeom + texCoordOffset); }

    private:
        Geometry() {}

        VGeom* vgeoms;
        void* vbo;

        unsigned int vgeomCount, vgeomSize, colorOffset, texCoordOffset;
        bool hasColor, hasTexCoord;

        Renderer* renderer;
    };

} // namespace vdpm

#endif // VDPM_GEOMETRY_H
