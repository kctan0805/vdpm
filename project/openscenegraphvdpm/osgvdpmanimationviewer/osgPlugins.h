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
USE_OSGPLUGIN(ive)
USE_OSGPLUGIN(osg)
USE_OSGPLUGIN(osg2)
USE_OSGPLUGIN(rgb)
USE_OSGPLUGIN(OpenFlight)
USE_OSGPLUGIN(png)
USE_OSGPLUGIN(freetype)
USE_OSGPLUGIN(vdpm)

USE_DOTOSGWRAPPER_LIBRARY(osg)
USE_DOTOSGWRAPPER_LIBRARY(osgFX)
USE_DOTOSGWRAPPER_LIBRARY(osgParticle)
USE_DOTOSGWRAPPER_LIBRARY(osgShadow)
USE_DOTOSGWRAPPER_LIBRARY(osgSim)
USE_DOTOSGWRAPPER_LIBRARY(osgTerrain)
USE_DOTOSGWRAPPER_LIBRARY(osgText)
USE_DOTOSGWRAPPER_LIBRARY(osgViewer)
USE_DOTOSGWRAPPER_LIBRARY(osgVolume)
USE_DOTOSGWRAPPER_LIBRARY(osgWidget)

USE_SERIALIZER_WRAPPER_LIBRARY(osg)
USE_SERIALIZER_WRAPPER_LIBRARY(osgAnimation)
USE_SERIALIZER_WRAPPER_LIBRARY(osgFX)
USE_SERIALIZER_WRAPPER_LIBRARY(osgManipulator)
USE_SERIALIZER_WRAPPER_LIBRARY(osgParticle)
USE_SERIALIZER_WRAPPER_LIBRARY(osgShadow)
USE_SERIALIZER_WRAPPER_LIBRARY(osgSim)
USE_SERIALIZER_WRAPPER_LIBRARY(osgTerrain)
USE_SERIALIZER_WRAPPER_LIBRARY(osgText)
USE_SERIALIZER_WRAPPER_LIBRARY(osgVolume)
USE_SERIALIZER_WRAPPER_LIBRARY(osgVdpm)

//osg animation
struct RegisterDotOsgWrapperProxyProxy
{
    RegisterDotOsgWrapperProxyProxy(osgDB::RegisterDotOsgWrapperProxy& proxy) : _proxy(proxy) {}
    osgDB::RegisterDotOsgWrapperProxy& _proxy;
};

extern osgDB::RegisterDotOsgWrapperProxy g_BoneProxy;
RegisterDotOsgWrapperProxyProxy g_BoneProxyProxy(g_BoneProxy);

extern osgDB::RegisterDotOsgWrapperProxy g_StackedTranslateElementProxy;
RegisterDotOsgWrapperProxyProxy g_StackedTranslateElementProxyProxy(g_StackedTranslateElementProxy);

extern osgDB::RegisterDotOsgWrapperProxy g_UpdateMaterialProxy;
RegisterDotOsgWrapperProxyProxy g_UpdateMaterialProxyProxy(g_UpdateMaterialProxy);

extern osgDB::RegisterDotOsgWrapperProxy g_UpdateMatrixTransformProxy;
RegisterDotOsgWrapperProxyProxy g_UpdateMatrixTransformProxyProxy(g_UpdateMatrixTransformProxy);
