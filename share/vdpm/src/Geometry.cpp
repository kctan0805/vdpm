#include <cassert>
#include <cstdlib>
#include "vdpm/Geometry.h"

using namespace std;
using namespace vdpm;

int Geometry::create(unsigned int count, bool hasColor, bool hasTexCoord)
{
    vgeomSize = sizeof(VGeom);

    if (hasColor)
    {
        colorOffset = vgeomSize;
        vgeomSize += sizeof(float) * 3;
    }

    if (hasTexCoord)
    {
        texCoordOffset = vgeomSize;
        vgeomSize += sizeof(float) * 2;
    }

    vgeoms = (VGeom*)::malloc(vgeomSize * count);
    if (!vgeoms)
        return -1;

    this->hasColor = hasColor;
    this->hasTexCoord = hasTexCoord;
    this->vgeomCount = count;

    return 0;
}

void Geometry::destroy()
{
    if (renderer)
        renderer->destroyBuffer(vbo);

    ::free(vgeoms);
}

int Geometry::realize(Renderer* renderer)
{
    vbo = (VGeom*)renderer->createBuffer(RENDERER_VERTEX_BUFFER, vgeomSize * vgeomCount, vgeoms);
    if (!vbo)
        goto error;

#ifdef VDPM_RENDERER_OPENGL_VBO_MAP_VGEOM
    ::free(vgeoms);
    vgeoms = NULL;
#endif

    this->renderer = renderer;
    return 0;

error:
#ifdef VDPM_RENDERER_OPENGL_VBO_MAP_VGEOM
    if (vbo)
    {
        renderer->destroyBuffer(vbo);
        vbo = NULL;
    }
#endif // VDPM_RENDERER_OPENGL_VBO_MAP_VGEOM

    return -1;
}

int Geometry::resize(unsigned int count)
{
    vbo = renderer->resizeBuffer(RENDERER_VERTEX_BUFFER, vbo, vgeomSize * count);
#ifndef VDPM_RENDERER_OPENGL_VBO
    vgeoms = (VGeom*)vbo;
#endif
#ifdef VDPM_RENDERER_OPENGL_VBO_MAP_VGEOM
    vgeoms = (VGeom*)renderer->mapBuffer(RENDERER_VERTEX_BUFFER, vbo, 0, vgeomSize * count, RENDERER_READ_WRITE);
#endif
    return 0;
}

#ifdef VDPM_RENDERER_OPENGL_VBO_MAP_VGEOM
void Geometry::mapVGeom()
{
    assert(!vgeoms);
    vgeoms = (VGeom*)renderer->mapBuffer(RENDERER_VERTEX_BUFFER, vbo, 0, vgeomSize * vgeomCount, RENDERER_READ_WRITE);
}

void Geometry::unmapVGeom()
{
    renderer->unmapBuffer(RENDERER_VERTEX_BUFFER, vbo);
    vgeoms = NULL;
}
#endif // VDPM_RENDERER_OPENGL_VBO_MAP_VGEOM
