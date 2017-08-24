/* vdpm - View-dependent progressive meshes library
* Copyright 2015 Jim Tan
* https://github.com/kctan0805/vdpm
*
* vdpm is free software; you can redistribute it and/or modify
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
