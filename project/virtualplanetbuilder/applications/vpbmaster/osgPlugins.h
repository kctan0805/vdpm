#pragma once

//This file is used to force the linking of the osg and hogbox plugins
//as we our doing a static build we can't depend on the loading of the
//dynamic libs to add the plugins to the registries

#include <osgViewer/GraphicsWindow>
#include <osgDB/Registry>

//windowing system
#ifndef ANDROID
USE_GRAPHICSWINDOW()
#endif


//osg plugins

USE_OSGPLUGIN(OpenFlight)
USE_OSGPLUGIN(obj)
USE_OSGPLUGIN(shp)
USE_OSGPLUGIN(ive)
USE_OSGPLUGIN(nvtt)

//depreceated osg format
USE_OSGPLUGIN(osg)
USE_DOTOSGWRAPPER_LIBRARY(osg)

USE_OSGPLUGIN(osg2)
USE_SERIALIZER_WRAPPER_LIBRARY(osg)

//image files
USE_OSGPLUGIN(tiff)
USE_OSGPLUGIN(png)
USE_OSGPLUGIN(jpeg)
USE_OSGPLUGIN(dds)

USE_OSGPLUGIN(terrain)
USE_SERIALIZER_WRAPPER_LIBRARY(osgTerrain)
USE_DOTOSGWRAPPER_LIBRARY(osgTerrain)

USE_OSGPLUGIN(vpb)
USE_SERIALIZER_WRAPPER(BuildOptions)
USE_SERIALIZER_WRAPPER(ImageOptions)
