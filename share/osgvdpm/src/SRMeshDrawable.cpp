/* osgVdpm - View-dependent progressive meshes for OpenSceneGraph
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
#include "vdpm/OpenGLRenderer.h"
#include "vdpm/SRMesh.h"
#include "vdpm/Viewport.h"
#include "osgVdpm/SRMeshDrawable"
#include "osgVdpm/SRMeshUserData"
#include "osgVdpm/SRMeshUserStats"

using namespace osg;
using namespace osgVdpm;

SRMeshDrawable::SRMeshDrawable() : srmesh(NULL), viewport(NULL), status(NOT_REALIZED)
{
    // turn off display lists right now, just incase we want to modify the projection matrix along the way.
    setSupportsDisplayList(false);
}

SRMeshDrawable::SRMeshDrawable(const SRMeshDrawable& srmeshdrawable,const CopyOp& copyop):
Drawable(srmeshdrawable, copyop), srmesh(NULL), viewport(NULL), status(NOT_REALIZED)
{
}

SRMeshDrawable::~SRMeshDrawable()
{
    delete srmesh;
    delete viewport;
}

BoundingSphere SRMeshDrawable::computeBound() const
{
    const vdpm::Bounds& bounds = srmesh->getBounds();
	osg::Vec3f center(bounds.center.x, bounds.center.y, bounds.center.z);
	BoundingSphere bs(center, bounds.radius);

    return bs;
}

void SRMeshDrawable::drawImplementation(RenderInfo& renderInfo) const
{
    SRMeshUserData* userData = (SRMeshUserData*)renderInfo.getView()->getUserData();
    bool pause = false;

    if (status <= REALIZING)
    {
        if (status == REALIZING)
            return;

        status = REALIZING;

        srmesh->realize(&vdpm::OpenGLRenderer::getInstance());

        viewport = new vdpm::Viewport();
        srmesh->setViewport(viewport);

        double fovy, aspect, zNear, zFar;
        osg::Matrix proj = renderInfo.getCurrentCamera()->getProjectionMatrix();
        proj.getPerspective(fovy, aspect, zNear, zFar);

        srmesh->setViewAngle(osg::DegreesToRadians(fovy));

        status = REALIZED;
    }

    if (userData)
    {
        srmesh->setTau(userData->getTau());
        srmesh->setTargetAFaceCount(userData->getTargetAFaceCount());
        srmesh->setAmortizeStep(userData->getAmortizeStep());
        srmesh->setGTime(userData->getGTime());

        if (userData->getStats() && renderInfo.getCurrentCamera()->getStats()->collectStats("rendering"))
        {
            unsigned int framenumber = renderInfo.getView()->getFrameStamp()->getFrameNumber();
            unsigned int afaceCount = srmesh->getAFaceCount();
            unsigned int tstripCount = srmesh->getTStripCount();
            userData->getStats()->setAttribute(framenumber, SRMeshUserStats::tauName, srmesh->getTau());
            userData->getStats()->setAttribute(framenumber, SRMeshUserStats::afaceCountName, afaceCount);
            userData->getStats()->setAttribute(framenumber, SRMeshUserStats::vmorphCountName, srmesh->getVMorphCount());
            userData->getStats()->setAttribute(framenumber, SRMeshUserStats::afaceCountPerTStripName, ((tstripCount > 0) ? ((float)afaceCount / tstripCount) : 0));
        }
        pause = userData->getPause();
    }
    if (!pause)
    {
        srmesh->updateViewport();

    #ifdef VDPM_GEOMORPHS
        srmesh->updateVMorphs();
    #endif
        srmesh->adaptRefine();
        srmesh->updateScene();
    }
    srmesh->draw();
}

