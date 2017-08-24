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
#ifndef VDPM_VIEWPORT_H
#define VDPM_VIEWPORT_H

#include "vdpm/Types.h"

namespace vdpm
{
    class Viewport
    {
        friend class SRMesh;

    public:
        Viewport();
        ~Viewport();

        static Viewport& getDefaultInstance();

        void setViewClipPlane(int plane, float a, float b, float c, float d);
        void setViewPosition(float x, float y, float z);
        const Point& getViewPosition() { return viewPos; }

    protected:
        Point viewPos;
        float frustum[6][4];

#ifdef VDPM_PREDICT_VIEW_POSITION
        Point predictViewPos, delta_e;
#endif
    };
} // namespace vdpm

#endif // VDPM_VIEWPORT_H
