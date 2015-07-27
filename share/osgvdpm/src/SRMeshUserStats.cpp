#include <osgViewer/Viewer>

#include "vdpm/SRMesh.h"
#include "osgVdpm/SRMeshUserStats"

using namespace osgVdpm;

const std::string SRMeshUserStats::tauName                  = "vdpmTau";
const std::string SRMeshUserStats::afaceCountName           = "vdpmAFaceCount";
const std::string SRMeshUserStats::vmorphCountName          = "vdpmVmorphCount";
const std::string SRMeshUserStats::afaceCountPerTStripName  = "vdpmAFaceCountPerTStrip";

void SRMeshUserStats::init(osgViewer::StatsHandler* statsHandler)
{
    statsHandler->addUserStatsLine("Tau(%)", osg::Vec4(0.7, 0.7, 0.7, 1), osg::Vec4(0.7, 0.7, 0.7, 0.5),
        tauName, 100.0, true, false, "", "", 1.0);
    statsHandler->addUserStatsLine("Active Faces", osg::Vec4(0.7, 0.7, 0.7, 1), osg::Vec4(0.7, 0.7, 0.7, 0.5),
        afaceCountName, 1.0, true, false, "", "", UINT_MAX);
    statsHandler->addUserStatsLine("VMorphs", osg::Vec4(0.7, 0.7, 0.7, 1), osg::Vec4(0.7, 0.7, 0.7, 0.5),
        vmorphCountName, 1.0, true, false, "", "", UINT_MAX);
    statsHandler->addUserStatsLine("Avg. Faces per TStrip", osg::Vec4(0.7, 0.7, 0.7, 1), osg::Vec4(0.7, 0.7, 0.7, 0.5),
        afaceCountPerTStripName, 1.0, true, false, "", "", 100.0);
}
