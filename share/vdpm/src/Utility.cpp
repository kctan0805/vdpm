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
#include <algorithm>
#include "vdpm/Utility.h"

using namespace vdpm;

Vector vdpm::crossProduct(const Vector& v1, const Vector& v2)
{
	Vector result;

	result[0] = v1[1] * v2[2] - v1[2] * v2[1];
	result[1] = v1[2] * v2[0] - v1[0] * v2[2];
	result[2] = v1[0] * v2[1] - v1[1] * v2[0];

	return result;
}

Bounds::Bounds()
{
    min[0] = min[1] = min[2] = 0.0f;
    max[0] = max[1] = max[2] = 0.0f;
    center[0] = center[1] = center[2] = 0.0f;
    radius = 0.0f;

    points = 0;
    initialized = false;
}

void Bounds::addPoint(const Vector& v, bool will_update)
{
    if (!initialized)
    {
        min[0] = max[0] = v[0];
        min[1] = max[1] = v[1];
        min[2] = max[2] = v[2];
        initialized = true;
    }
    else
    {
        if (v[0] < min[0]) min[0] = v[0];
        if (v[1] < min[1]) min[1] = v[1];
        if (v[2] < min[2]) min[2] = v[2];

        if (v[0] > max[0]) max[0] = v[0];
        if (v[1] > max[1]) max[1] = v[1];
        if (v[2] > max[2]) max[2] = v[2];
    }

    if (will_update)
    {
        center += Vector(v);
        points++;
    }
}

void Bounds::complete()
{
    center /= (float)points;

    Vector R1 = max - center;
    Vector R2 = min - center;

    radius = std::max(magnitude(R1), magnitude(R2));
}

void Bounds::merge(const Bounds& b)
{
    addPoint(b.min, false);
    addPoint(b.max, false);
    points += b.points;

    Vector dC = b.center - center;
    float dist = magnitude(dC);

    if (dist + b.radius > radius)
    {
        // New sphere does not lie within old sphere
        center += b.center;
        center /= 2;

        dist /= 2;
        radius = std::max(dist + radius, dist + b.radius);
    }
}
