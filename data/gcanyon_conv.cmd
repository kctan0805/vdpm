set PATH=..\build\virtualplanetbuilder\project\virtualplanetbuilder\Release;%PATH%
..\build\virtualplanetbuilder\project\virtualplanetbuilder\Release\vpbmaster -d gcanyon_height.tiff -t gcanyon_color_4k2k.tiff -o gcanyon.osgb --POLYGONAL --RGBA --LOD
..\build\osgvdpmconv\project\osgvdpmconv\Release\osgvdpmconv --vdpm 500 gcanyon.osgb gcanyonvdpm.osgb --smooth
pause
