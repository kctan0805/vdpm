///////////////////////////////////////////////////////////////////////////////
// ModelGL.cpp
// ===========
// Model component of OpenGL
// 
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2006-07-10
// UPDATED: 2013-11-27
///////////////////////////////////////////////////////////////////////////////

#if _WIN32
    #include <windows.h>    // include windows.h to avoid thousands of compile errors even though this class is not depending on Windows
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include "ModelGL.h"
#include "Bmp.h"



///////////////////////////////////////////////////////////////////////////////
// default ctor
///////////////////////////////////////////////////////////////////////////////
ModelGL::ModelGL() : windowWidth(0), windowHeight(0), mouseLeftDown(false),
                     mouseRightDown(false), changeDrawMode(false), drawMode(0),
                     cameraAngleX(0), cameraAngleY(0), cameraDistance(5),
                     animateFlag(false), bgFlag(0), frameBuffer(0), bufferSize(0),
                     windowResized(false)
{
    bgColor[0] = bgColor[1] = bgColor[2] = bgColor[3] = 0;
}



///////////////////////////////////////////////////////////////////////////////
// destructor
///////////////////////////////////////////////////////////////////////////////
ModelGL::~ModelGL()
{
    // deallocate framebuffer
    //delete [] frameBuffer;
    //frameBuffer = 0;
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
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

     // track material ambient and diffuse from surface color, call it before glEnable(GL_COLOR_MATERIAL)
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    //glEnable(GL_COLOR_MATERIAL);

    glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);   // background color
    glClearStencil(0);                              // clear stencil buffer
    glClearDepth(1.0f);                             // 0 is near, 1 is far
    glDepthFunc(GL_LEQUAL);

    initLights();
    setCamera(0, 0, 8, 0, 0, 0);
    listId = createEarthDL();
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

    // position the light
    float lightPos[4] = {-10, 0, 5, 0};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_LIGHT0);                            // MUST enable each light source after configuration
}



///////////////////////////////////////////////////////////////////////////////
// set camera position and lookat direction
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(posX, posY, posZ, targetX, targetY, targetZ, 0, 1, 0); // eye(x,y,z), focal(x,y,z), up(x,y,z)
}



///////////////////////////////////////////////////////////////////////////////
// configure projection and viewport
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setViewport(int w, int h)
{
    // assign the width/height of viewport
    windowWidth = w;
    windowHeight = h;
    //bufferSize = w * h * 4; // rgba
    //frameBuffer = new unsigned char [bufferSize];

    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    // set perspective viewing frustum
    float aspectRatio = (float)w / h;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)(w)/h, 0.1f, 20.0f); // FOV, AspectRatio, NearClip, FarClip

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
}



///////////////////////////////////////////////////////////////////////////////
// toggle to resize window
///////////////////////////////////////////////////////////////////////////////
void ModelGL::resizeWindow(int w, int h)
{
    // assign the width/height of viewport
    windowWidth = w;
    windowHeight = h;
    windowResized = true;
}



///////////////////////////////////////////////////////////////////////////////
// draw 2D/3D scene
///////////////////////////////////////////////////////////////////////////////
void ModelGL::draw()
{
    static float angle = 0;

    if(windowResized)
    {
        setViewport(windowWidth, windowHeight);
        windowResized = false;
    }

    // clear buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // save the initial ModelView matrix before modifying ModelView matrix
    glPushMatrix();

    // tramsform camera
    glTranslatef(0, 0, cameraDistance);
    glRotatef(cameraAngleX, 1, 0, 0);   // pitch
    glRotatef(cameraAngleY, 0, 1, 0);   // heading

    glPushMatrix(); // draw sphere
        if(animateFlag) angle += 0.5f;
        //glRotatef(angle, 0.0f, 1.0f, 0.0f);
        glRotatef(angle, 0.39875f, 0.91706f, 0.0f);
        glCallList(listId);     // render with display list
    glPopMatrix();

    glPopMatrix();

    // read color framebuffer
    //glReadBuffer(GL_BACK);
    //glReadPixels(0, 0, windowWidth, windowHeight, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)frameBuffer);

    //for (int i = 0; i < bufferSize; i+=4)
    //{
    //    frameBuffer[i] = (unsigned char)255;
    //    frameBuffer[i+1] = 16;
    //    frameBuffer[i+2] = 16;
    //}
    //glDrawPixels(windowWidth, windowHeight, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)frameBuffer);

    if(changeDrawMode)
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
    }

    // check if background colour was changed
    if(bgFlag)
    {
        glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
        bgFlag = false;
    }
}



///////////////////////////////////////////////////////////////////////////////
// compile a sphere with texturemap in a DL
///////////////////////////////////////////////////////////////////////////////
GLuint ModelGL::createEarthDL()
{
    // set material
    GLfloat matAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat matDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat matSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat matShininess[] = { 10.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

    // create a sphere which is a quadric object
    GLUquadricObj *sphere = gluNewQuadric();
    GLuint sphereTex = loadTextureBmp("earth.bmp");
    GLuint envTex = loadTextureBmp("envmap01_512.bmp");
    gluQuadricDrawStyle(sphere, GLU_FILL); // GLU_FILL, GLU_LINE, GLU_SILHOUETTE, GLU_POINT
    gluQuadricTexture(sphere, GL_TRUE);
    gluQuadricNormals(sphere, GL_SMOOTH);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

    GLuint id = glGenLists(1);
    glNewList(id, GL_COMPILE);  // create display list for the sphere
        //glDisable(GL_DEPTH_TEST);
        //glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glBindTexture(GL_TEXTURE_2D, sphereTex);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPushMatrix();

        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(23.5f, 0.0f, 1.0f, 0.0f);
        gluSphere(sphere, 1.0, 50, 50); // radius, slice, stack

/*
        glColor4f(1.0f, 1.0f, 1.0f, 0.3f);
        glBindTexture(GL_TEXTURE_2D, envTex);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
        gluSphere(sphere, 1.0, 50, 50); // radius, slice, stack
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
*/
        glPopMatrix();
        //glEnable(GL_DEPTH_TEST);
    glEndList();

    return id;
}



///////////////////////////////////////////////////////////////////////////////
// load a BMP as texture
///////////////////////////////////////////////////////////////////////////////
unsigned int ModelGL::loadTextureBmp(const char* fileName)
{
    int chans, x, y;
    void* buf;
    Image::Bmp bmp;

    bmp.read(fileName);
    x = bmp.getWidth();
    y = bmp.getHeight();
    chans = bmp.getBitCount() / 8;
    buf = (void*)bmp.getDataRGB();

    // gen texture ID
    GLuint texture;
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
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // build our texture mipmaps
    switch(chans)
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

    return texture;
}



///////////////////////////////////////////////////////////////////////////////
// rotate the camera
///////////////////////////////////////////////////////////////////////////////
void ModelGL::rotateCamera(int x, int y)
{
    if(mouseLeftDown)
    {
        cameraAngleY += (x - mouseX);
        cameraAngleX += (y - mouseY);
        mouseX = x;
        mouseY = y;
    }
}



///////////////////////////////////////////////////////////////////////////////
// zoom the camera
///////////////////////////////////////////////////////////////////////////////
void ModelGL::zoomCamera(int delta)
{
    if(mouseRightDown)
    {
        cameraDistance += (delta - mouseY) * 0.05f;
        mouseY = delta;
    }
}



///////////////////////////////////////////////////////////////////////////////
// change drawing mode
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setDrawMode(int mode)
{
    if(drawMode != mode)
    {
        changeDrawMode = true;
        drawMode = mode;
    }
}



///////////////////////////////////////////////////////////////////////////////
// change background colour, the value should be between 0 and 1
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setBackgroundRed(float value)
{
    if(bgColor[0] != value)
    {
        bgColor[0] = value;
        bgFlag = true;
    }
}
void ModelGL::setBackgroundGreen(float value)
{
    if(bgColor[1] != value)
    {
        bgColor[1] = value;
        bgFlag = true;
    }
}
void ModelGL::setBackgroundBlue(float value)
{
    if(bgColor[2] != value)
    {
        bgColor[2] = value;
        bgFlag = true;
    }
}
