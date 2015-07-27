///////////////////////////////////////////////////////////////////////////////
// ModelGL.cpp
// ===========
// Model component of OpenGL
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2008-10-02
// UPDATED: 2013-03-01
///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include <windows.h>    // include windows.h to avoid thousands of compile errors even though this class is not depending on Windows
#endif

#include <GL/glew.h>
#ifdef __APPLE__
#include <GL/glxew.h>
#else
#include <GL/wglew.h>
#endif

#define _USE_MATH_DEFINES
#include <cmath>
#include "ModelGL.h"
#include "cameraSimple.h"   // 3D mesh of camera
#include "Log.h"
#include "vdpm/Log.h"
#include "vdpm/OpenGLRenderer.h"
#include "vdpm/Serializer.h"
#include "vdpm/SRMesh.h"
#include "vdpm/Viewport.h"

// constants
const float DEG2RAD = 3.141593f / 180;
const float FOV_Y = 60.0f;              // vertical FOV in degree
const float CAMERA_ANGLE_X = 0.0f;     // pitch in degree
const float CAMERA_ANGLE_Y = 0.0f;    // heading in degree
const float CAMERA_DISTANCE = 25.0f;    // camera distance
// default projection matrix values
const float DEFAULT_LEFT = -0.5f;
const float DEFAULT_RIGHT = 0.5f;
const float DEFAULT_BOTTOM = -0.5f;
const float DEFAULT_TOP = 0.5f;
const float DEFAULT_NEAR = 1.0f;
const float DEFAULT_FAR = 100.0f;

///////////////////////////////////////////////////////////////////////////////
// default ctor
///////////////////////////////////////////////////////////////////////////////
ModelGL::ModelGL() : windowWidth(0), windowHeight(0), mouseLeftDown(false), mouseX(0), mouseY(0),
                     mouseRightDown(false), drawModeChanged(false), drawMode(0),
                     cameraAngleX(CAMERA_ANGLE_X), cameraAngleY(CAMERA_ANGLE_Y),
                     cameraDistance(CAMERA_DISTANCE), windowSizeChanged(false),
                     projectionLeft(DEFAULT_LEFT),
                     projectionRight(DEFAULT_RIGHT),
                     projectionBottom(DEFAULT_BOTTOM),
                     projectionTop(DEFAULT_TOP),
                     projectionNear(DEFAULT_NEAR),
                     projectionFar(DEFAULT_FAR),
                     projectionMode(0),
                     texture(GL_INVALID_VALUE),
                     bmp(NULL),
                     viewChanged(false),
                     textureChanged(false),
                     tau(0.01f),
                     targetAfaceCount(10000),
                     gtime(8),
                     amortizeStep(4),
                     paramChanged(false),
                     initialized(false),
                     fps(0),
                     meshCount(0),
                     colorEnabled(false),
                     textureEnabled(false),
                     lightEnabled(false),
                     fillChanged(false),
                     fileLoading(false),
                     fileUnloading(false),
                     testingVsplit(false), 
                     testingEcol(false)
{
    cameraPosition[0] = cameraPosition[1] = cameraPosition[2] = 0;
    bgColor[0] = bgColor[1] = bgColor[2] = bgColor[3] = 0;

    // init projection matrix
    setFrustum(projectionLeft, projectionRight,
               projectionBottom, projectionTop,
               projectionNear, projectionFar);

    // model, view, modelview matrices are fixed in this app
    matrixModel.identity();
    matrixView.identity();
    matrixModelView.identity();
}



///////////////////////////////////////////////////////////////////////////////
// destructor
///////////////////////////////////////////////////////////////////////////////
ModelGL::~ModelGL()
{
}


///////////////////////////////////////////////////////////////////////////////
// initialize OpenGL states and scene
///////////////////////////////////////////////////////////////////////////////
void ModelGL::init()
{
    glShadeModel(GL_SMOOTH);                        // shading mathod: GL_SMOOTH or GL_FLAT
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);          // 4-byte pixel alignment

    // enable /disable features
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glEnable(GL_SCISSOR_TEST);

     // track material ambient and diffuse from surface color, call it before glEnable(GL_COLOR_MATERIAL)
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);   // background color
    glClearStencil(0);                              // clear stencil buffer
    glClearDepth(1.0f);                             // 0 is near, 1 is far
    glDepthFunc(GL_LEQUAL);

    initLights();

    Log::println = Win::log;
}



///////////////////////////////////////////////////////////////////////////////
// clean up OpenGL objects
///////////////////////////////////////////////////////////////////////////////
void ModelGL::quit()
{
}



///////////////////////////////////////////////////////////////////////////////
// initialize lights
///////////////////////////////////////////////////////////////////////////////
void ModelGL::initLights()
{
    // set up light colors (ambient, diffuse, specular)
    GLfloat lightKa[] = {.0f, .0f, .0f, 1.0f};      // ambient light
    GLfloat lightKd[] = {.9f, .9f, .9f, 1.0f};      // diffuse light
    GLfloat lightKs[] = {1, 1, 1, 1};               // specular light
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

    // position the light in eye space
    float lightPos[4] = {0, 1, 1, 0};               // directional light
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_LIGHT0);                            // MUST enable each light source after configuration
}



///////////////////////////////////////////////////////////////////////////////
// set rendering window size
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setWindowSize(int width, int height)
{
    // assign the width/height of viewport
    windowWidth = width;
    windowHeight = height;
    windowSizeChanged = true;
}



///////////////////////////////////////////////////////////////////////////////
// configure projection and viewport
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setViewport(int x, int y, int w, int h)
{
    // set viewport to be the entire window
    glViewport((GLsizei)x, (GLsizei)y, (GLsizei)w, (GLsizei)h);

    // set perspective viewing frustum
    setFrustum(FOV_Y, (float)(w)/h, projectionNear, projectionFar); // FOV, AspectRatio, NearClip, FarClip

    // copy projection matrix to OpenGL
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(matrixProjection.getTranspose());
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}



///////////////////////////////////////////////////////////////////////////////
// configure projection and viewport of sub window
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setViewportSub(int x, int y, int width, int height, float nearPlane, float farPlane)
{
    // set viewport
    glViewport(x, y, width, height);
    glScissor(x, y, width, height);

    // set perspective viewing frustum
    setFrustum(FOV_Y, (float)(width)/height, nearPlane, farPlane); // FOV, AspectRatio, NearClip, FarClip

    // copy projection matrix to OpenGL
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(matrixProjection.getTranspose());
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void ModelGL::setViewportSub(int x, int y, int width, int height, float left, float right, float bottom, float top, float front, float back)
{
    // set viewport
    glViewport(x, y, width, height);
    glScissor(x, y, width, height);

    // set viewing frustum
    if(projectionMode == 0) // perspective
        setFrustum(left, right, bottom, top, front, back);
    else                    // orthographic
        setOrthoFrustum(left, right, bottom, top, front, back);

    // copy projection matrix to OpenGL
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(matrixProjection.getTranspose());
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}



///////////////////////////////////////////////////////////////////////////////
// draw 2D/3D scene
///////////////////////////////////////////////////////////////////////////////
void ModelGL::draw()
{
    static int frames = 0;
    static DWORD lastTick = 0;
    DWORD tick = ::GetTickCount();

    frames++;
    if (tick - lastTick >= 1000)
    {
        fps = frames;
        frames = 0;
        lastTick = tick;
    }

    drawSub1();
    drawSub2();

    if(windowSizeChanged)
    {
        setViewport(0, 0, windowWidth, windowHeight);
        windowSizeChanged = false;
    }

    if(drawModeChanged)
    {
        if(drawMode == 0)           // fill mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
        }
        else if(drawMode == 1)      // wireframe mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        else if(drawMode == 2)      // point mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        drawModeChanged = false;
    }

    if (fillChanged)
    {
        if (colorEnabled)
            glEnable(GL_COLOR_MATERIAL);
        else
            glDisable(GL_COLOR_MATERIAL);
        
        if (textureEnabled)
            glEnable(GL_TEXTURE_2D);
        else
            glDisable(GL_TEXTURE_2D);

        if (lightEnabled)
            glEnable(GL_LIGHTING);
        else
            glDisable(GL_LIGHTING);
    }

    if (textureChanged)
    {
        glDeleteTextures(1, &texture);

        if (bmp)
        {
            int chans, x, y;
            void* buf;

            x = bmp->getWidth();
            y = bmp->getHeight();
            chans = bmp->getBitCount() / 8;
            buf = (void*)bmp->getDataRGB();

            // gen texture ID
            glGenTextures(1, &texture);

            // set active texture and configure it
            glBindTexture(GL_TEXTURE_2D, texture);

            // select modulate to mix texture with color for shading
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // if wrap is true, the texture wraps over at the edges (repeat)
            //       ... false, the texture ends at the edges (clamp)
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // build our texture mipmaps
            switch (chans)
            {
            case 1:
                gluBuild2DMipmaps(GL_TEXTURE_2D, chans, x, y, GL_LUMINANCE, GL_UNSIGNED_BYTE, buf);
                break;
            case 3:
                gluBuild2DMipmaps(GL_TEXTURE_2D, chans, x, y, GL_RGB, GL_UNSIGNED_BYTE, buf);
                break;
            case 4:
                gluBuild2DMipmaps(GL_TEXTURE_2D, chans, x, y, GL_RGBA, GL_UNSIGNED_BYTE, buf);
                break;
            }
            delete bmp;
            bmp = NULL;
        }
        textureChanged = false;
    }

    if (fileLoading)
    {
        SRMesh* srmesh;

        if (meshCount >= MAX_MESH_COUNT)
            return;

        srmesh = meshes[meshCount];
        srmesh->realize(&OpenGLRenderer::getInstance());

        viewChanged = paramChanged = initialized = true;
        ++meshCount;
        fileLoading = false;
    }
    else if (fileUnloading)
    {
        initialized = false;
        textureChanged = true;

        for (int i = 0; i < meshCount; ++i)
        {
            SRMesh* srmesh = meshes[i];
            delete srmesh;
        }
        meshCount = 0;

        fileUnloading = false;
    }
}



///////////////////////////////////////////////////////////////////////////////
// draw upper window (view from the camera)
///////////////////////////////////////////////////////////////////////////////
void ModelGL::drawSub1()
{
    // set upper viewport (perspective or ortho depending on projectionMode)
    setViewportSub(0, windowHeight/2, windowWidth, windowHeight/2, projectionLeft, projectionRight, projectionBottom, projectionTop, projectionNear, projectionFar);

    // clear buffer
    glClearColor(0.1f, 0.1f, 0.1f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glPushMatrix();

    // First, transform the camera (viewing matrix) from world space to eye space
    glTranslatef(-cameraPosition[0], -cameraPosition[1], -cameraPosition[2] - cameraDistance);
    glRotatef(cameraAngleX, 1, 0, 0); // pitch
    glRotatef(cameraAngleY, 0, 1, 0); // heading

    if (initialized)
    {
        for (int i = 0; i < meshCount; ++i)
        {
            SRMesh* srmesh = meshes[i];

            if (viewChanged)
            {
                srmesh->updateViewport();
                viewChanged = false;
            }

            if (paramChanged)
            {
                srmesh->setTau(tau);
            #ifdef VDPM_REGULATION
                srmesh->setTargetAFaceCount(targetAfaceCount);
            #endif
            #ifdef VDPM_GEOMORPHS
                srmesh->setGTime(gtime);
            #endif
            #ifdef VDPM_AMORTIZATION
                srmesh->setAmortizeStep(amortizeStep);
            #endif
                paramChanged = false;
            }

            if (testingVsplit)
            {
                for (int i = 0; i < meshCount; ++i)
                {
                #ifndef NDEBUG
                    meshes[i]->testVsplit();
                #endif
                    //meshes[i]->printStatus();
                }
                testingVsplit = false;
            }
            else if (testingEcol)
            {
                for (int i = 0; i < meshCount; ++i)
                {
                #ifndef NDEBUG
                    meshes[i]->testEcol();
                #endif
                    //meshes[i]->printStatus();
                }
                testingEcol = false;
            }

        #ifdef VDPM_GEOMORPHS
            srmesh->updateVMorphs();
        #endif
            srmesh->adaptRefine();
            srmesh->updateScene();
            srmesh->draw();
        }
    }
    glPopMatrix();
}

    

///////////////////////////////////////////////////////////////////////////////
// draw bottom window (3rd person view)
// This function does not use own matrices and use OpenGL matrix directly
///////////////////////////////////////////////////////////////////////////////
void ModelGL::drawSub2()
{
    // set bottom viewport (perspective)
    glViewport(0, 0, windowWidth, windowHeight/2);
    glScissor(0, 0, windowWidth, windowHeight/2);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(FOV_Y, windowWidth / (windowHeight / 2.0f), projectionNear, projectionFar);

    // switch to modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // clear buffer
    glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);   // background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glPushMatrix();

    // First, transform the camera (viewing matrix) from world space to eye space
    glTranslatef(-cameraPosition[0], -cameraPosition[1], -cameraPosition[2]);

    if (initialized)
    {
        for (int i = 0; i < meshCount; ++i)
        {
            SRMesh* srmesh = meshes[i];

            srmesh->draw();
        }
    }

    glPopMatrix();
}



///////////////////////////////////////////////////////////////////////////////
// rotate the camera for subWin2
///////////////////////////////////////////////////////////////////////////////
void ModelGL::rotateCamera(int x, int y)
{
    if (!initialized)
        return;

    cameraAngleY += (x - mouseX);
    cameraAngleX += (y - mouseY);
    mouseX = x;
    mouseY = y;
    viewChanged = true;
}



///////////////////////////////////////////////////////////////////////////////
// zoom the camera for subWin2
///////////////////////////////////////////////////////////////////////////////
void ModelGL::zoomCamera(int y)
{
    if (!initialized)
        return;

    cameraDistance -= (y - mouseY) * bounds.radius * 0.01f;
    mouseY = y;
    viewChanged = true;
}

void ModelGL::zoomCameraDelta(int delta)
{
    if (!initialized)
        return;

    cameraDistance -= delta;
    viewChanged = true;
}



///////////////////////////////////////////////////////////////////////////////
// change drawing mode
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setDrawMode(int mode)
{
    if(drawMode != mode)
    {
        drawModeChanged = true;
        drawMode = mode;
    }
}



///////////////////////////////////////////////////////////////////////////////
// set 6 params of frustum
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setProjection(float l, float r, float b, float t, float n, float f)
{
    projectionLeft = l;
    projectionRight = r;
    projectionBottom = b;
    projectionTop = t;
    projectionNear = n;
    projectionFar = f;

    if(projectionMode == 0)
        setFrustum(l, r, b, t, n, f);
    else
        setOrthoFrustum(l, r, b, t, n, f);
}



///////////////////////////////////////////////////////////////////////////////
// update projection matrix
///////////////////////////////////////////////////////////////////////////////
void ModelGL::updateProjectionMatrix()
{
    if(projectionMode == 0)         // perspective
    {
        setFrustum(projectionLeft, projectionRight,
                   projectionBottom, projectionTop,
                   projectionNear, projectionFar);
    }
    else if(projectionMode == 1)    // orthographic
    {
        setOrthoFrustum(projectionLeft, projectionRight,
                        projectionBottom, projectionTop,
                        projectionNear, projectionFar);
    }
}



///////////////////////////////////////////////////////////////////////////////
// set a perspective frustum with 6 params similar to glFrustum()
// (left, right, bottom, top, near, far)
// Note: this is for row-major notation. OpenGL needs transpose it
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setFrustum(float l, float r, float b, float t, float n, float f)
{
    matrixProjection.identity();
    matrixProjection[0]  =  2 * n / (r - l);
    matrixProjection[2]  =  (r + l) / (r - l);
    matrixProjection[5]  =  2 * n / (t - b);
    matrixProjection[6]  =  (t + b) / (t - b);
    matrixProjection[10] = -(f + n) / (f - n);
    matrixProjection[11] = -(2 * f * n) / (f - n);
    matrixProjection[14] = -1;
    matrixProjection[15] =  0;
}



///////////////////////////////////////////////////////////////////////////////
// set a symmetric perspective frustum with 4 params similar to gluPerspective
// (vertical field of view, aspect ratio, near, far)
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setFrustum(float fovY, float aspectRatio, float front, float back)
{
    float tangent = tanf(fovY/2 * DEG2RAD);   // tangent of half fovY
    float height = front * tangent;           // half height of near plane
    float width = height * aspectRatio;       // half width of near plane

    // params: left, right, bottom, top, near, far
    setFrustum(-width, width, -height, height, front, back);
}



///////////////////////////////////////////////////////////////////////////////
// set a orthographic frustum with 6 params similar to glOrtho()
// (left, right, bottom, top, near, far)
// Note: this is for row-major notation. OpenGL needs transpose it
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setOrthoFrustum(float l, float r, float b, float t, float n, float f)
{
    matrixProjection.identity();
    matrixProjection[0]  =  2 / (r - l);
    matrixProjection[3]  =  -(r + l) / (r - l);
    matrixProjection[5]  =  2 / (t - b);
    matrixProjection[7]  =  -(t + b) / (t - b);
    matrixProjection[10] = -2 / (f - n);
    matrixProjection[11] = -(f + n) / (f - n);
}


void ModelGL::loadSRMesh(const char fileLine[])
{
    SRMesh* srmesh;

    if (meshCount >= MAX_MESH_COUNT)
        return;

    // load VDPM file
    meshes[meshCount] = Serializer::getInstance().loadSRMesh(fileLine);
    srmesh = meshes[meshCount];

    //srmesh->printStatus();
    if (meshCount == 0)
        bounds = srmesh->getBounds();
    else
        bounds.merge(srmesh->getBounds());

    cameraDistance = 3 * bounds.radius / tan(60 * (float)M_PI / 180.0f);
    projectionNear = cameraDistance / 20;
    projectionFar = 10 * cameraDistance;
    float tangent = tan(FOV_Y / 2 * DEG2RAD);   // tangent of half fovY
    projectionTop = projectionNear * tangent;          // half height of near plane
    projectionRight = projectionTop * windowWidth / (windowHeight / 2);      // half width of near plane
    projectionLeft = -projectionRight;
    projectionBottom = -projectionTop;
    cameraPosition[0] = bounds.center.x;
    cameraPosition[1] = bounds.center.y;
    cameraPosition[2] = bounds.center.z + cameraDistance;

    srmesh->setViewport(&Viewport::getDefaultInstance());
    srmesh->setViewAngle(FOV_Y * DEG2RAD);

    fileLoading = true;
}


void ModelGL::loadTexture(const char fileLine[])
{
    if (textureChanged)
        return;

    bmp = new Image::Bmp();
    if (!bmp)
        return;

    bmp->read(fileLine);
    textureChanged = true;
}


void ModelGL::unload()
{
    fileUnloading = true;
}

void ModelGL::setTau(float tau)
{
    this->tau = tau;
    paramChanged = true;
}

void ModelGL::setAFaces(unsigned int count)
{
#ifdef VDPM_REGULATION
    targetAfaceCount = count;
    paramChanged = true;
#endif
}

void ModelGL::setGTime(int gtime)
{
#ifdef VDPM_GEOMORPHS
    this->gtime = gtime;
    paramChanged = true;
#endif
}

void ModelGL::setAmortize(int step)
{
#ifdef VDPM_AMORTIZATION
    amortizeStep = step;
    paramChanged = true;
#endif
}

float ModelGL::getTau()
{
    float tau = 0.0f;

    if (initialized)
    {
        for (int i = 0; i < meshCount; ++i)
            tau += meshes[i]->getTau();

        tau /= meshCount;
    }
    return tau;
}

unsigned int ModelGL::getAFaces()
{
    unsigned int afaceCount = 0;

    if (initialized)
    {
        for (int i = 0; i < meshCount; ++i)
            afaceCount += meshes[i]->getAFaceCount();
    }
    return afaceCount;
}

const Point& ModelGL::getViewPosition()
{
    static const Point pt;

    if (initialized)
        return Viewport::getDefaultInstance().getViewPosition();

    return pt;
}

const Bounds& ModelGL::getBounds()
{
    static const Bounds b;

    if (initialized)
        return bounds;

    return b;
}

unsigned int ModelGL::getTStripCount()
{
    unsigned int tstripCount = 0;

    if (initialized)
    {
        for (int i = 0; i < meshCount; ++i)
            tstripCount += meshes[i]->getTStripCount();
    }
    return tstripCount;
}

unsigned int ModelGL::getVMorphCount()
{
    unsigned int vmorphCount = 0;
#ifdef VDPM_GEOMORPHS
    if (initialized)
    {
        for (int i = 0; i < meshCount; ++i)
            vmorphCount += meshes[i]->getVMorphCount();
    }
#endif // VDPM_GEOMORPHS
    return vmorphCount;
}

void ModelGL::enableColor()
{
    colorEnabled = true;
    fillChanged = true;
}

void ModelGL::disableColor()
{
    colorEnabled = false;
    fillChanged = true;
}

void ModelGL::enableTexture()
{
    textureEnabled = true;
    fillChanged = true;
}

void ModelGL::disableTexture()
{
    textureEnabled = false;
    fillChanged = true;
}

void ModelGL::enableLighting()
{
    lightEnabled = true;
    fillChanged = true;
}

void ModelGL::disableLighting()
{
    lightEnabled = false;
    fillChanged = true;
}

void ModelGL::testVsplit()
{
    if (initialized)
        testingVsplit = true;
}

void ModelGL::testEcol()
{
    if (initialized)
        testingEcol = true;
}