if (DEFINED CFG_USE_CITYGML)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        citygml
        osgdb_citygml
        )
endif()

if (DEFINED CFG_USE_OSGVDPM)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        osgVdpm
        )
endif()

if (DEFINED CFG_USE_OSGEARTH)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        osgdb_osgearth_template_matclass
        osgdb_osgearth_ocean_simple
        osgdb_osgearth_sky_simple
        osgdb_osgearth_sky_gl
        osgdb_osgearth_vdatum_egm2008
        osgdb_osgearth_vdatum_egm96
        osgdb_osgearth_vdatum_egm84
        #osgdb_osgearth_engine_quadtree
        osgdb_osgearth_engine_byo
        osgdb_osgearth_engine_mp
        osgdb_osgearth_label_annotation
        osgdb_osgearth_label_overlay
        osgdb_osgearth_mask_feature
        osgdb_osgearth_model_feature_geom
        osgdb_osgearth_model_feature_stencil
        osgdb_osgearth_feature_tfs
        osgdb_osgearth_feature_wfs
        osgdb_osgearth_feature_ogr
        osgdb_osgearth_gdal
        osgdb_osgearth_colorramp
        osgdb_osgearth_splat_mask
        osgdb_osgearth_noise
        osgdb_osgearth_tileindex
        osgdb_osgearth_bing
        osgdb_osgearth_xyz
        osgdb_osgearth_refresh
        osgdb_osgearth_cache_filesystem
        osgdb_osgearth_debug
        osgdb_osgearth_model_simple
        osgdb_osgearth_agglite
        osgdb_osgearth_osg
        osgdb_osgearth_vpb
        osgdb_osgearth_tms
        osgdb_osgearth_arcgis
        osgdb_osgearth_arcgis_map_cache
        osgdb_osgearth_yahoo
        osgdb_osgearth_tileservice
        osgdb_osgearth_tilecache
        osgdb_osgearth_wms
        osgdb_osgearth_wcs
        osgdb_kml
        osgdb_earth
        osgEarthSymbology
        osgEarthUtil
        osgEarthFeatures
        osgEarthAnnotation
        osgEarth
    )
endif()

if (DEFINED CFG_USE_VIRTUALPLANETBUILDER)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        vpb
    )
endif()

if (DEFINED CFG_USE_OPENSCENEGRAPH)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        osgdb_zip
        osgdb_freetype
        osgdb_nvtt
        osgdb_shp
        osgdb_tgz
        osgdb_osgtgz
        osgdb_obj
        osgdb_openflight
        osgdb_gz
        osgdb_curl
        osgdb_tiff
        osgdb_png
        osgdb_jpeg
        osgdb_ktx
        osgdb_vtf
        osgdb_dot
        osgdb_hdr
        osgdb_tga
        osgdb_dds
        osgdb_pnm
        osgdb_bmp
        osgdb_rgb
        osgdb_glsl
        osgdb_cfg
        osgdb_ive
        osgdb_osg
        osgdb_osgterrain
        osgdb_osgshadow
        osgdb_osgviewer
        osgdb_revisions
        osgdb_normals
        osgdb_trans
        osgdb_scale
        osgdb_rot
        osgdb_osga
        osgdb_deprecated_osgwidget
        osgdb_deprecated_osgvolume
        osgdb_deprecated_osgterrain
        osgdb_deprecated_osgshadow
        osgdb_deprecated_osgviewer
        osgdb_deprecated_osgtext
        osgdb_deprecated_osgsim
        osgdb_deprecated_osgfx
        osgdb_deprecated_osganimation
        osgdb_deprecated_osgparticle
        osgdb_deprecated_osg
        osgdb_serializers_osgviewer
        osgdb_serializers_osgvolume
        osgdb_serializers_osgtext
        osgdb_serializers_osgterrain
        osgdb_serializers_osgga
        osgdb_serializers_osgshadow
        osgdb_serializers_osgsim
        osgdb_serializers_osgparticle
        osgdb_serializers_osgmanipulator
        osgdb_serializers_osgfx
        osgdb_serializers_osganimation
        osgdb_serializers_osg
        osgVolume
        osgWidget
        osgTerrain
        osgSim
        osgShadow
        osgPresentation
        osgParticle
        osgManipulator
        osgFX
        osgAnimation
        osgViewer
        osgText
        osgGA
        osgUtil
        osgDB
        osg
        OpenThreads
    )
endif()

if (DEFINED CFG_USE_MIXKIT)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        mixkit
        )
endif()

if (DEFINED CFG_USE_FLTK)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        fltk
        fltk_forms
        fltk_gl
        fltk_images
    )
endif()

if (DEFINED CFG_USE_FREETYPE)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        freetype
    )
endif()

if (DEFINED CFG_USE_GDAL)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        gdal
    )
endif()

if (DEFINED CFG_USE_CURL)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        curl
    )
endif()

if (DEFINED CFG_USE_GETOPT)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        getopt
    )
endif()

if (DEFINED CFG_USE_GIFLIB)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        gif
    )
endif()

if (DEFINED CFG_USE_GFX)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        gfx
    )
endif()

if (DEFINED CFG_USE_GLEW)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        glew
    )
endif()

if (DEFINED CFG_USE_JPEG)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        jpeg
    )
endif()

if (DEFINED CFG_USE_MESH)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        mesh
        )
endif()

if (DEFINED CFG_USE_NVTT)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        nvtt
        )
endif()

if (DEFINED CFG_USE_PNG)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        png
    )
endif()

if (DEFINED CFG_USE_PROJ)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        proj
    )
endif()

if (DEFINED CFG_USE_TIFF)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        tiff
    )
endif()

if (DEFINED CFG_USE_VDPM)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        vdpm
    )
endif()

if (DEFINED CFG_USE_XERCESC)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        xerces-c
    )
endif()

if (DEFINED CFG_USE_ZLIB)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        zlib
    )
endif()

if(MSVC)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        comctl32
        glu32
        opengl32
        winmm
        wldap32
        ws2_32
    )
endif()
