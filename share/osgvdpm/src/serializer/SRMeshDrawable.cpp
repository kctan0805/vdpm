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
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
#include "vdpm/OutStream.h"
#include "vdpm/Serializer.h"
#include "vdpm/SRMesh.h"
#include "osgVdpm/SRMeshDrawable"

class OsgOutStream : public vdpm::OutStream
{
public:
    OsgOutStream(osgDB::OutputStream* os);

    void writeChar(char& value);
    void writeUInt(unsigned int& value);
    void writeFloat(float& value);

private:
    osgDB::OutputStream* os;
};

OsgOutStream::OsgOutStream(osgDB::OutputStream* os)
{
    this->os = os;
}

void OsgOutStream::writeChar(char& value)
{
    *os << value << std::endl;
}

void OsgOutStream::writeUInt(unsigned int& value)
{
    *os << value << std::endl;
}

void OsgOutStream::writeFloat(float& value)
{
    *os << value << std::endl;
}

class OsgInStream : public vdpm::InStream
{
public:
    OsgInStream(osgDB::InputStream* is);

    void readChar(char& value);
    void readUInt(unsigned int& value);
    void readFloat(float& value);

private:
    osgDB::InputStream* is;
};

OsgInStream::OsgInStream(osgDB::InputStream* is)
{
    this->is = is;
}

void OsgInStream::readChar(char& value)
{
    *is >> value;
}

void OsgInStream::readUInt(unsigned int& value)
{
    *is >> value;
}

void OsgInStream::readFloat(float& value)
{
    *is >> value;
}


static bool checkSRMesh( const osgVdpm::SRMeshDrawable& node )
{
    return true;
}

static bool readSRMesh( osgDB::InputStream& is, osgVdpm::SRMeshDrawable& node )
{
    OsgInStream istream(&is);

    is >> is.BEGIN_BRACKET;

    node.setSRMesh(vdpm::Serializer::getInstance().readSRMesh(istream));

    is >> is.END_BRACKET;
    return true;
}

static bool writeSRMesh( osgDB::OutputStream& os, const osgVdpm::SRMeshDrawable& node )
{
    OsgOutStream ostream(&os);

    os << os.BEGIN_BRACKET << std::endl;

    vdpm::Serializer::getInstance().writeSRMesh(ostream, node.getSRMesh());

    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgVdpm_SRMeshDrawable,
                         new osgVdpm::SRMeshDrawable,
                         osgVdpm::SRMeshDrawable,
                         "osg::Object osg::Drawable osgVdpm::SRMeshDrawable" )
{
    ADD_USER_SERIALIZER( SRMesh );  // srmesh
}
