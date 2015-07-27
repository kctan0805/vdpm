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
