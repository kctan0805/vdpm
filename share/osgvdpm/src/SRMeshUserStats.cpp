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
