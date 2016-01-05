include_directories(${JPEG_INCLUDE_DIR} ${PNG_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
set(FLTK_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

#######################################################################
# basic setup
#######################################################################
# The FLTK version
set(FLTK_VERSION_MAJOR "1")
set(FLTK_VERSION_MINOR "3")
set(FLTK_VERSION_PATCH "3")
set(FLTK_VERSION "${FLTK_VERSION_MAJOR}.${FLTK_VERSION_MINOR}")
set(FLTK_VERSION_FULL "${FLTK_VERSION}.${FLTK_VERSION_PATCH}")

set(FLTK_INCLUDE_DIRS ${FLTK_BINARY_DIR} ${FLTK_SOURCE_DIR})
include_directories(${FLTK_INCLUDE_DIRS})

#######################################################################
# platform dependent information
#######################################################################

# fix no WIN32 defined issue
if(NOT WIN32)
    if(_WIN32)
        set(WIN32 _WIN32)
    elseif(__WIN32__)
        set(WIN32 __WIN32__)
    endif(_WIN32)
endif(NOT WIN32)

# set where config and example files go
if(WIN32 AND NOT CYGWIN)
   set(FLTK_CONFIG_PATH CMake)
elseif(APPLE)
   set(FLTK_CONFIG_PATH FLTK/.framework/Resources/CMake)
else()
   set(FLTK_CONFIG_PATH lib/fltk)
endif(WIN32 AND NOT CYGWIN)

include(TestBigEndian)
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

#######################################################################
# size of ints
include(CheckTypeSize)

CHECK_TYPE_SIZE(short SIZEOF_SHORT)
CHECK_TYPE_SIZE(int   SIZEOF_INT)
CHECK_TYPE_SIZE(long  SIZEOF_LONG)
CHECK_TYPE_SIZE("long long"  HAVE_LONG_LONG)

if(${SIZEOF_SHORT} MATCHES "^2$")
   set(U16 "unsigned short")
endif(${SIZEOF_SHORT} MATCHES "^2$")

if(${SIZEOF_INT} MATCHES "^4$")
   set(U32 "unsigned")
else()
   if(${SIZEOF_LONG} MATCHES "^4$")
      set(U32 "unsigned long")
   endif(${SIZEOF_LONG} MATCHES "^4$")
endif(${SIZEOF_INT} MATCHES "^4$")

if(${SIZEOF_INT} MATCHES "^8$")
   set(U64 "unsigned")
else()
   if(${SIZEOF_LONG} MATCHES "^8$")
      set(U64 "unsigned long")
   endif(${SIZEOF_LONG} MATCHES "^8$")
endif(${SIZEOF_INT} MATCHES "^8$")

#######################################################################
# check for headers, libraries and functions
#######################################################################
# headers
find_file(HAVE_ALSA_ASOUNDLIB_H alsa/asoundlib.h)
find_file(HAVE_DIRENT_H dirent.h)
find_file(HAVE_DLFCN_H dlfcn.h)
find_file(HAVE_FREETYPE_H freetype.h PATH_SUFFIXES freetype2 freetype2/freetype)
find_file(HAVE_GL_GL_H GL/gl.h)
find_file(HAVE_GL_GLU_H GL/glu.h)
find_file(HAVE_LIBPNG_PNG_H libpng/png.h)
find_file(HAVE_LOCALE_H locale.h)
find_file(HAVE_NDIR_H ndir.h)
find_file(HAVE_OPENGL_GLU_H OpenGL/glu.h)
find_file(HAVE_PNG_H png.h)
find_file(HAVE_PTHREAD_H pthread.h)
find_file(HAVE_STDIO_H stdio.h)
find_file(HAVE_STRINGS_H strings.h)
find_file(HAVE_SYS_DIR_H sys/dir.h)
find_file(HAVE_SYS_NDIR_H sys/ndir.h)
find_file(HAVE_SYS_SELECT_H sys/select.h)
find_file(HAVE_SYS_STDTYPES_H sys/stdtypes.h)
find_path(HAVE_XDBE_H Xdbe.h PATH_SUFFIXES X11/extensions extensions)

mark_as_advanced(HAVE_ALSA_ASOUNDLIB_H HAVE_DIRENT_H HAVE_DLFCN_H)
mark_as_advanced(HAVE_FREETYPE_H HAVE_GL_GL_H HAVE_GL_GLU_H)
mark_as_advanced(HAVE_LIBPNG_PNG_H HAVE_LOCALE_H HAVE_NDIR_H)
mark_as_advanced(HAVE_OPENGL_GLU_H HAVE_PNG_H HAVE_PTHREAD_H)
mark_as_advanced(HAVE_STDIO_H HAVE_STRINGS_H HAVE_SYS_DIR_H)
mark_as_advanced(HAVE_SYS_NDIR_H HAVE_SYS_SELECT_H)
mark_as_advanced(HAVE_SYS_STDTYPES_H HAVE_XDBE_H)

# where to find freetype headers
find_path(FREETYPE_PATH freetype.h PATH_SUFFIXES freetype2)
find_path(FREETYPE_PATH freetype/freetype.h PATH_SUFFIXES freetype2)
if(FREETYPE_PATH)
   include_directories(${FREETYPE_PATH})
endif(FREETYPE_PATH)
mark_as_advanced(FREETYPE_PATH)

#######################################################################
# libraries
find_library(LIB_fontconfig fontconfig)
find_library(LIB_freetype freetype)
find_library(LIB_GL GL)
find_library(LIB_MesaGL MesaGL)
find_library(LIB_jpeg jpeg)
find_library(LIB_png png)
find_library(LIB_zlib z)

mark_as_advanced(LIB_CAIRO LIB_dl LIB_fontconfig LIB_freetype)
mark_as_advanced(LIB_GL LIB_MesaGL)
mark_as_advanced(LIB_jpeg LIB_png LIB_zlib)

#######################################################################
# functions
include(CheckFunctionExists)

set(CMAKE_REQUIRED_LIBRARIES png)
CHECK_FUNCTION_EXISTS(png_get_valid          HAVE_PNG_GET_VALID)
CHECK_FUNCTION_EXISTS(png_set_tRNS_to_alpha  HAVE_PNG_SET_TRNS_TO_ALPHA)

CHECK_FUNCTION_EXISTS(scandir                HAVE_SCANDIR)
CHECK_FUNCTION_EXISTS(snprintf               HAVE_SNPRINTF)

# not really true but we convert strcasecmp calls to _stricmp calls in flstring.h
if(MSVC)
   set(HAVE_STRCASECMP 1)
endif(MSVC)

CHECK_FUNCTION_EXISTS(strcasecmp             HAVE_STRCASECMP)

CHECK_FUNCTION_EXISTS(strlcat                HAVE_STRLCAT)
CHECK_FUNCTION_EXISTS(strlcpy                HAVE_STRLCPY)
CHECK_FUNCTION_EXISTS(vsnprintf              HAVE_VSNPRINTF)

set(CMAKE_REQUIRED_LIBRARIES)

mark_as_advanced(HAVE_SCANDIR_POSIX)

#######################################################################
# options
#######################################################################
set(OPTION_OPTIM ""
   CACHE STRING
   "custom optimization flags"
   )
add_definitions(${OPTION_OPTIM})

#######################################################################
set(OPTION_ARCHFLAGS ""
   CACHE STRING
   "custom architecture flags"
   )
add_definitions(${OPTION_ARCHFLAGS})

#######################################################################
option(OPTION_USE_POLL "use poll if available" OFF)

if(OPTION_USE_POLL)
   CHECK_FUNCTION_EXISTS(poll USE_POLL)
endif(OPTION_USE_POLL)

#######################################################################
option(OPTION_BUILD_SHARED_LIBS
    "Build shared libraries(in addition to static libraries)"
    OFF
    )

option(OPTION_CAIRO "use lib Cairo" OFF)
option(OPTION_CAIROEXT
   "use fltk code instrumentation for cairo extended use" OFF
   )

set(FLTK_CAIRO_FOUND FALSE)

#######################################################################
set(HAVE_GL LIB_GL OR LIB_MesaGL)

if(HAVE_GL)
   option(OPTION_USE_GL "use OpenGL" ON)
endif(HAVE_GL)

if(OPTION_USE_GL)
   include(FindOpenGL)
endif(OPTION_USE_GL)

if(OPENGL_FOUND)
   set(CMAKE_REQUIRED_INCLUDES ${OPENGL_INCLUDE_DIR}/GL)
   set(CMAKE_REQUIRED_LIBRARIES -lGLU -lGL)
   CHECK_FUNCTION_EXISTS(glXGetProcAddressARB HAVE_GLXGETPROCADDRESSARB)
   set(FLTK_GL_FOUND TRUE)
else()
   set(FLTK_GL_FOUND FALSE)
endif(OPENGL_FOUND)

#######################################################################
option(OPTION_USE_THREADS "use multi-threading" ON)

if(OPTION_USE_THREADS)
   include(FindThreads)
endif(OPTION_USE_THREADS)

if(OPTION_USE_THREADS AND CMAKE_HAVE_THREADS_LIBRARY)
   add_definitions("-D_THREAD_SAFE -D_REENTRANT")
   set(USE_THREADS 1)
   set(FLTK_THREADS_FOUND TRUE)
else()
   set(FLTK_THREADS_FOUND FALSE)
endif(OPTION_USE_THREADS AND CMAKE_HAVE_THREADS_LIBRARY)

if(OPTION_USE_THREADS AND CMAKE_USE_PTHREADS_INIT)
   set(HAVE_PTHREAD 1)
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
   list(APPEND FLTK_LDLIBS -lpthread)
   list(APPEND FLTK_CFLAGS -D_THREAD_SAFE -D_REENTRANT)
   set(FLTK_PTHREADS_FOUND TRUE)
else()
   set(HAVE_PTHREAD 0)
   set(HAVE_PTHREAD_H 0)
   set(FLTK_PTHREADS_FOUND FALSE)
endif(OPTION_USE_THREADS AND CMAKE_USE_PTHREADS_INIT)

#######################################################################
option(OPTION_LARGE_FILE "enable large file support" ON)

if(OPTION_LARGE_FILE)
   if(NOT MSVC)
      add_definitions(-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE)
      list(APPEND FLTK_CFLAGS -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE)
   endif(NOT MSVC)
endif(OPTION_LARGE_FILE)

#######################################################################
option(OPTION_USE_SYSTEM_ZLIB "Use's system zlib" ON)

set(HAVE_LIBZ 1)

#######################################################################
option(OPTION_USE_SYSTEM_LIBJPEG "Use's system libjpeg" ON)

set(HAVE_LIBJPEG 1)

#######################################################################
option(OPTION_USE_SYSTEM_LIBPNG "Use's system libpng" ON)

set(HAVE_LIBPNG 1)

#######################################################################
# final config and export
#######################################################################

# generate FLTKConfig.cmake for build directory use
set(INCLUDE_DIRS ${FLTK_INCLUDE_DIRS})
set(CONFIG_PATH ${FLTK_BINARY_DIR})

configure_file(
   ${FLTK_SOURCE_DIR}/CMake/FLTKConfig.cmake.in
   ${FLTK_BINARY_DIR}/FLTKConfig.cmake
   @ONLY
)

# generate UseFLTK.cmake for build directory use
configure_file(
   ${FLTK_SOURCE_DIR}/CMake/UseFLTK.cmake.in
   ${FLTK_BINARY_DIR}/UseFLTK.cmake
   @ONLY
)

# generate config.h
configure_file(
   "${FLTK_SOURCE_DIR}/configh.cmake.in"
   "${FLTK_BINARY_DIR}/config.h"
   @ONLY
)

# generate fltk-config
get_filename_component(CC ${CMAKE_C_COMPILER} NAME)
get_filename_component(CXX ${CMAKE_CXX_COMPILER} NAME)

string(REPLACE ";" " " C_FLAGS "${FLTK_CFLAGS}")
string(REPLACE ";" " " LD_LIBS "${FLTK_LDLIBS}")

configure_file(
   "${FLTK_SOURCE_DIR}/fltk-config.cmake.in"
   "${FLTK_BINARY_DIR}/fltk-config"
   @ONLY
)

set(CPPFILES
  ${FLTK_SOURCE_DIR}/src/Fl.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Adjuster.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Bitmap.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Browser.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Browser_.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Browser_load.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Box.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Button.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Chart.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Check_Browser.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Check_Button.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Choice.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Clock.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Color_Chooser.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Copy_Surface.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Counter.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Device.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Dial.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Help_Dialog_Dox.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Double_Window.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_File_Browser.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_File_Chooser.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_File_Chooser2.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_File_Icon.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_File_Input.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Group.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Help_View.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Image.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Image_Surface.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Input.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Input_.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Light_Button.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Menu.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Menu_.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Menu_Bar.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Menu_Button.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Menu_Window.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Menu_add.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Menu_global.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Multi_Label.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Native_File_Chooser.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Overlay_Window.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Pack.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Paged_Device.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Pixmap.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Positioner.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_PostScript.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Printer.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Preferences.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Progress.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Repeat_Button.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Return_Button.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Roller.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Round_Button.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Scroll.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Scrollbar.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Shared_Image.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Single_Window.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Slider.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Table.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Table_Row.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Tabs.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Text_Buffer.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Text_Display.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Text_Editor.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Tile.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Tiled_Image.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Tooltip.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Tree.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Tree_Item_Array.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Tree_Item.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Tree_Prefs.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Valuator.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Value_Input.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Value_Output.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Value_Slider.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Widget.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Window.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Window_fullscreen.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Window_hotspot.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Window_iconize.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Window_shape.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Wizard.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_XBM_Image.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_XPM_Image.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_abort.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_add_idle.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_arg.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_compose.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_display.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_get_key.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_get_system_colors.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_grab.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_lock.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_own_colormap.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_visual.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_x.cxx
  ${FLTK_SOURCE_DIR}/src/filename_absolute.cxx
  ${FLTK_SOURCE_DIR}/src/filename_expand.cxx
  ${FLTK_SOURCE_DIR}/src/filename_ext.cxx
  ${FLTK_SOURCE_DIR}/src/filename_isdir.cxx
  ${FLTK_SOURCE_DIR}/src/filename_list.cxx
  ${FLTK_SOURCE_DIR}/src/filename_match.cxx
  ${FLTK_SOURCE_DIR}/src/filename_setext.cxx
  ${FLTK_SOURCE_DIR}/src/fl_arc.cxx
  ${FLTK_SOURCE_DIR}/src/fl_arci.cxx
  ${FLTK_SOURCE_DIR}/src/fl_ask.cxx
  ${FLTK_SOURCE_DIR}/src/fl_boxtype.cxx
  ${FLTK_SOURCE_DIR}/src/fl_color.cxx
  ${FLTK_SOURCE_DIR}/src/fl_cursor.cxx
  ${FLTK_SOURCE_DIR}/src/fl_curve.cxx
  ${FLTK_SOURCE_DIR}/src/fl_diamond_box.cxx
  ${FLTK_SOURCE_DIR}/src/fl_dnd.cxx
  ${FLTK_SOURCE_DIR}/src/fl_draw.cxx
  ${FLTK_SOURCE_DIR}/src/fl_draw_image.cxx
  ${FLTK_SOURCE_DIR}/src/fl_draw_pixmap.cxx
  ${FLTK_SOURCE_DIR}/src/fl_engraved_label.cxx
  ${FLTK_SOURCE_DIR}/src/fl_file_dir.cxx
  ${FLTK_SOURCE_DIR}/src/fl_font.cxx
  ${FLTK_SOURCE_DIR}/src/fl_gleam.cxx
  ${FLTK_SOURCE_DIR}/src/fl_gtk.cxx
  ${FLTK_SOURCE_DIR}/src/fl_labeltype.cxx
  ${FLTK_SOURCE_DIR}/src/fl_line_style.cxx
  ${FLTK_SOURCE_DIR}/src/fl_open_uri.cxx
  ${FLTK_SOURCE_DIR}/src/fl_oval_box.cxx
  ${FLTK_SOURCE_DIR}/src/fl_overlay.cxx
  ${FLTK_SOURCE_DIR}/src/fl_overlay_visual.cxx
  ${FLTK_SOURCE_DIR}/src/fl_plastic.cxx
  ${FLTK_SOURCE_DIR}/src/fl_read_image.cxx
  ${FLTK_SOURCE_DIR}/src/fl_rect.cxx
  ${FLTK_SOURCE_DIR}/src/fl_round_box.cxx
  ${FLTK_SOURCE_DIR}/src/fl_rounded_box.cxx
  ${FLTK_SOURCE_DIR}/src/fl_set_font.cxx
  ${FLTK_SOURCE_DIR}/src/fl_set_fonts.cxx
  ${FLTK_SOURCE_DIR}/src/fl_scroll_area.cxx
  ${FLTK_SOURCE_DIR}/src/fl_shadow_box.cxx
  ${FLTK_SOURCE_DIR}/src/fl_shortcut.cxx
  ${FLTK_SOURCE_DIR}/src/fl_show_colormap.cxx
  ${FLTK_SOURCE_DIR}/src/fl_symbols.cxx
  ${FLTK_SOURCE_DIR}/src/fl_vertex.cxx
  ${FLTK_SOURCE_DIR}/src/ps_image.cxx
  ${FLTK_SOURCE_DIR}/src/screen_xywh.cxx
  ${FLTK_SOURCE_DIR}/src/fl_utf8.cxx
  ${FLTK_SOURCE_DIR}/src/fl_encoding_latin1.cxx
  ${FLTK_SOURCE_DIR}/src/fl_encoding_mac_roman.cxx
)

set(FLCPPFILES
  ${FLTK_SOURCE_DIR}/src/forms_compatability.cxx
  ${FLTK_SOURCE_DIR}/src/forms_bitmap.cxx
  ${FLTK_SOURCE_DIR}/src/forms_free.cxx
  ${FLTK_SOURCE_DIR}/src/forms_fselect.cxx
  ${FLTK_SOURCE_DIR}/src/forms_pixmap.cxx
  ${FLTK_SOURCE_DIR}/src/forms_timer.cxx
)

set(GLCPPFILES
  ${FLTK_SOURCE_DIR}/src/Fl_Gl_Choice.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Gl_Device_Plugin.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Gl_Overlay.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Gl_Window.cxx
  ${FLTK_SOURCE_DIR}/src/freeglut_geometry.cxx
  ${FLTK_SOURCE_DIR}/src/freeglut_stroke_mono_roman.cxx
  ${FLTK_SOURCE_DIR}/src/freeglut_stroke_roman.cxx
  ${FLTK_SOURCE_DIR}/src/freeglut_teapot.cxx
  ${FLTK_SOURCE_DIR}/src/gl_draw.cxx
  ${FLTK_SOURCE_DIR}/src/gl_start.cxx
  ${FLTK_SOURCE_DIR}/src/glut_compatability.cxx
  ${FLTK_SOURCE_DIR}/src/glut_font.cxx
)

set(IMGCPPFILES
  ${FLTK_SOURCE_DIR}/src/fl_images_core.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_BMP_Image.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_File_Icon2.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_GIF_Image.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_Help_Dialog.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_JPEG_Image.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_PNG_Image.cxx
  ${FLTK_SOURCE_DIR}/src/Fl_PNM_Image.cxx
)

set(CFILES
  ${FLTK_SOURCE_DIR}/src/flstring.c
  ${FLTK_SOURCE_DIR}/src/scandir.c
  ${FLTK_SOURCE_DIR}/src/numericsort.c
  ${FLTK_SOURCE_DIR}/src/vsnprintf.c
  ${FLTK_SOURCE_DIR}/src/xutf8/is_right2left.c
  ${FLTK_SOURCE_DIR}/src/xutf8/is_spacing.c
  ${FLTK_SOURCE_DIR}/src/xutf8/case.c
  ${FLTK_SOURCE_DIR}/src/xutf8/utf8Input.c
  ${FLTK_SOURCE_DIR}/src/xutf8/utf8Utils.c
  ${FLTK_SOURCE_DIR}/src/xutf8/utf8Wrap.c
  ${FLTK_SOURCE_DIR}/src/xutf8/keysym2Ucs.c
  ${FLTK_SOURCE_DIR}/src/fl_utf.c
)

add_definitions(-DFL_LIBRARY)

#######################################################################
add_library(fltk STATIC ${CPPFILES};${CFILES};${FLTK_SOURCE_DIR}/src/fl_call_main.c)

#######################################################################
add_library(fltk_forms STATIC ${FLCPPFILES})

#######################################################################
add_library(fltk_images STATIC ${IMGCPPFILES})

#######################################################################
if(OPENGL_FOUND)
   add_library(fltk_gl STATIC ${GLCPPFILES})
endif(OPENGL_FOUND)

#######################################################################
