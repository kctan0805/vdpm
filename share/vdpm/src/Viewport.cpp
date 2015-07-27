#include <cassert>
#include "vdpm/Viewport.h"

using namespace std;
using namespace vdpm;

Viewport::Viewport()
{
}

Viewport::~Viewport()
{
    // do nothing
}

Viewport& Viewport::getDefaultInstance()
{
    static Viewport self;
    return self;
}

void Viewport::setViewClipPlane(int plane, float a, float b, float c, float d)
{
    float t;
    assert(plane >= 0);
    assert(plane < 6);

    t = sqrt(a * a + b * b + c * c);
    frustum[plane][0] = a / t;
    frustum[plane][1] = b / t;
    frustum[plane][2] = c / t;
    frustum[plane][3] = d / t;
}

void Viewport::setViewPosition(float x, float y, float z)
{
    viewPos.x = x;
    viewPos.y = y;
    viewPos.z = z;
}
