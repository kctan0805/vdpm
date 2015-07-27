include_directories(
    ${OPENSCENEGRAPH_INCLUDE_DIR}
    ${VDPM_INCLUDE_DIR}
    include
)

add_library(osgVdpm STATIC
    include/osgVdpm/PreDrawCallback
    include/osgVdpm/SRMeshDrawable
    include/osgVdpm/SRMeshUserData
    include/osgVdpm/SRMeshUserStats
    src/PreDrawCallback.cpp
    src/ReaderWriterVDPM.cpp
    src/SRMeshDrawable.cpp
    src/SRMeshUserStats.cpp
    src/serializer/LibraryWrapper.cpp
    src/serializer/SRMeshDrawable.cpp
)
