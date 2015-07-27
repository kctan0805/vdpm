#ifndef _SRMESH_CONVERTER_H
#define _SRMESH_CONVERTER_H

#include "stdmix.h"
#include "MxVdpmSlim.h"
#include "vdpm/SRMesh.h"
#include "osgVdpm/SRMeshDrawable"
#include "osg/NodeVisitor"

class SRMeshConverter : public osg::NodeVisitor
{
public:
    SRMeshConverter(unsigned int faceTarget, std::string& fileName);
    ~SRMeshConverter();

    virtual void apply( osg::Geode & geode );
    virtual void apply( osg::Node & node )
    {
        traverse( node );
    }

private:
    void read(MxStdModel& m, osg::Geometry& geometry);
    void write(osgVdpm::SRMeshDrawable& srmeshdrawable, MxVdpmSlim& slim);

    unsigned int faceTarget;
};
#endif
