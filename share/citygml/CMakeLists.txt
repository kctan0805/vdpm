set(CMAKE_CURRENT_SOURCE_DIR ${CITYGML_SOURCE_DIR})

set(META_VERSION_MAJOR       "1")
set(META_VERSION_MINOR       "4")
set(META_VERSION_PATCH       "0")
set(META_VERSION             "${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}")

SET(LIBCITYGML_USER_DEFINED_DYNAMIC_OR_STATIC "STATIC")

# gdal library
ADD_DEFINITIONS( -DUSE_GDAL )

# core
SET( target citygml )

ADD_DEFINITIONS( -DCITYGML_LIBRARY )

ADD_DEFINITIONS( -DLIBCITYGML_BUILD )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/citygml_api.h.in ${CMAKE_BINARY_DIR}/share/citygml/include/citygml/citygml_api.h)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/sources/include ${XERCESC_INCLUDE_DIR}  ${ICONV_INCLUDE_DIR} ${GDAL_INCLUDE_DIR} ${CMAKE_BINARY_DIR}/share/citygml/include)

SET(SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/citymodel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/tesselator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/object.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/featureobject.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/appearance.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/texture.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/material.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/georeferencedtexture.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/envelope.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/appearancemanager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/cityobject.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/geometry.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/implictgeometry.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/linearring.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/polygon.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/transformmatrix.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/texturetargetdefinition.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/materialtargetdefinition.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/texturecoordinates.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/appearancetarget.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/citygmlfactory.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/polygonmanager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/geometrymanager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/citygml/linestring.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/parser/nodetypes.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/parser/attributes.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/parser/geocoordinatetransformer.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/parser/citygmldocumentparser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/parser/parserxercesc.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/parser/citygmlelementparser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/parser/elementparser.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/parser/delayedchoiceelementparser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/parser/skipelementparser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/parser/sequenceparser.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/parser/gmlobjectparser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/parser/gmlfeaturecollectionparser.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/parser/citymodelelementparser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/parser/cityobjectelementparser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/parser/appearanceelementparser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/parser/materialelementparser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/parser/textureelementparser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/parser/georeferencedtextureelementparser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/parser/geometryelementparser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/parser/polygonelementparser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/parser/linestringelementparser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/parser/linearringelementparser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/src/parser/implicitgeometryelementparser.cpp
)

SET(PUBLIC_HEADER
  ${CMAKE_BINARY_DIR}/share/citygml/include/citygml/citygml_api.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/citygmllogger.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/polygon.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/material.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/geometry.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/object.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/featureobject.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/georeferencedtexture.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/cityobject.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/envelope.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/appearance.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/vecs.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/citymodel.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/linearring.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/citygml.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/transformmatrix.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/implictgeometry.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/tesselator.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/texture.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/appearancetargetdefinition.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/texturetargetdefinition.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/materialtargetdefinition.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/texturecoordinates.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/appearancetarget.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/citygmlfactory.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/linestring.h
)

SET(HEADERS
  ${PUBLIC_HEADER}

  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/citygml_api.h.in

  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/tesselator.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/utils.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/appearancemanager.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/polygonmanager.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/citygml/geometrymanager.h

  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/nodetypes.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/attributes.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/documentlocation.h

  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/parserutils.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/geocoordinatetransformer.h

  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/citygmldocumentparser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/citygmlelementparser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/elementparser.h

  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/delayedchoiceelementparser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/skipelementparser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/sequenceparser.h

  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/gmlobjectparser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/gmlfeaturecollectionparser.h

  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/citymodelelementparser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/cityobjectelementparser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/appearanceelementparser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/materialelementparser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/textureelementparser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/georeferencedtextureelementparser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/geometryelementparser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/polygonelementparser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/linestringelementparser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/linearringelementparser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/include/parser/implicitgeometryelementparser.h
)

ADD_LIBRARY( ${target} ${LIBCITYGML_USER_DEFINED_DYNAMIC_OR_STATIC} ${SOURCES} ${HEADERS} )


# osg plugin
set(target osgdb_citygml)

include_directories(
  ${OPENSCENEGRAPH_INCLUDE_DIR}
)

set( sources ${CMAKE_CURRENT_SOURCE_DIR}/osgplugin/ReaderWriterCityGML.cpp )

add_library(${target} ${LIBCITYGML_USER_DEFINED_DYNAMIC_OR_STATIC} ${sources})
