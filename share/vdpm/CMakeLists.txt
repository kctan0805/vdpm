include_directories(include)

add_library(vdpm STATIC
    include/vdpm/Allocator.h
    include/vdpm/Config.h
    include/vdpm/Geometry.h
    include/vdpm/InStream.h
    include/vdpm/Log.h
    include/vdpm/OpenGLRenderer.h
    include/vdpm/OutStream.h
    include/vdpm/Renderer.h
    include/vdpm/Serializer.h
    include/vdpm/SRMesh.h
    include/vdpm/StdInStream.h
    include/vdpm/Types.h
    include/vdpm/Utility.h
    include/vdpm/Viewport.h
    src/Allocator.cpp
    src/Geometry.cpp
    src/Log.cpp
    src/OpenGLRenderer.cpp
    src/Renderer.cpp
    src/Serializer.cpp
    src/StdInStream.cpp
    src/SRMesh.cpp
    src/Utility.cpp
    src/Viewport.cpp
)
