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

BoundingBox SRMeshDrawable::computeBound() const
{
    const vdpm::Bounds& bounds = srmesh->getBounds();
    BoundingBox bbox(bounds.min.x, bounds.min.y, bounds.min.z, bounds.max.x, bounds.max.y, bounds.max.z);

    return bbox;
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

