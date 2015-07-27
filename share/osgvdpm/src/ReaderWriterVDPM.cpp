#include <stdlib.h>
#include <string>

#include <osg/Notify>
#include <osg/Node>
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osg/Texture2D>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include "vdpm/Serializer.h"
#include "vdpm/SRMesh.h"
#include "vdpm/StdInStream.h"
#include "osgVdpm/SRMeshDrawable"

class ReaderWriterVDPM : public osgDB::ReaderWriter
{
public:
    ReaderWriterVDPM()
    {
        supportsExtension("vdpm","View-dependent Progressive Meshes format");
    }

    virtual const char* className() const { return "View-dependent Progressive Meshes Reader"; }

    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const;

    virtual ReadResult readNode(std::istream& fin, const Options* options) const;

protected:

    osg::Node* convertSRMeshToSceneGraph(vdpm::SRMesh& srmesh, const Options* options) const;
};

osg::Node* ReaderWriterVDPM::convertSRMeshToSceneGraph(vdpm::SRMesh& srmesh, const Options* options) const
{
    osg::Group* group = new osg::Group;

    osgVdpm::SRMeshDrawable* srmeshdrawable = new osgVdpm::SRMeshDrawable;

    if (srmeshdrawable)
    {
        srmeshdrawable->setSRMesh(&srmesh);

        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(srmeshdrawable);

        group->addChild(geode);

        if (srmesh.getTextureName())
        {
            osg::ref_ptr< osg::Image > image;
            if (!options->getDatabasePathList().empty())
            {
                // first try with database path of parent.
                image = osgDB::readRefImageFile(options->getDatabasePathList().front() + '/' + srmesh.getTextureName(), options);
            }

            if (!image.valid())
            {
                // if not already set then try the filename as is.
                image = osgDB::readRefImageFile(srmesh.getTextureName(), options);
            }

            if (image.valid())
            {
                osg::Texture2D* texture = new osg::Texture2D(image.get());
                osg::Texture::WrapMode textureWrapMode = osg::Texture::REPEAT;

                texture->setWrap(osg::Texture2D::WRAP_R, textureWrapMode);
                texture->setWrap(osg::Texture2D::WRAP_S, textureWrapMode);
                texture->setWrap(osg::Texture2D::WRAP_T, textureWrapMode);
                srmeshdrawable->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
            }
        }
    }
    return group;
}

// register with Registry to instantiate the above reader/writer.
REGISTER_OSGPLUGIN(vdpm, ReaderWriterVDPM)

// read file and convert to OSG.
osgDB::ReaderWriter::ReadResult ReaderWriterVDPM::readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
{
    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    std::string fileName = osgDB::findDataFile( file, options );
    if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

    osgDB::ifstream fin(fileName.c_str(), std::ios_base::in | std::ios_base::binary);
    if (fin)
    {
        vdpm::StdInStream ins(&fin);
        vdpm::SRMesh* srmesh = vdpm::Serializer::getInstance().loadSRMesh(ins);

        // code for setting up the database path so that internally referenced file are searched for on relative paths.
        osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
        local_opt->setDatabasePath(osgDB::getFilePath(fileName));

        osg::Node* node = convertSRMeshToSceneGraph(*srmesh, local_opt);
        return node;
    }

    return ReadResult::FILE_NOT_HANDLED;
}

osgDB::ReaderWriter::ReadResult ReaderWriterVDPM::readNode(std::istream& fin, const Options* options) const
{
    if (fin)
    {
        fin.imbue(std::locale::classic());

        vdpm::StdInStream ins(&fin);
        vdpm::SRMesh* srmesh = vdpm::Serializer::getInstance().loadSRMesh(ins);

        osg::Node* node = convertSRMeshToSceneGraph(*srmesh, options);
        return node;
    }

    return ReadResult::FILE_NOT_HANDLED;
}
