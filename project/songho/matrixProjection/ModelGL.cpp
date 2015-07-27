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

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <cmath>
#include "ModelGL.h"
#include "cameraSimple.h"   // 3D mesh of camera


// constants
const float DEG2RAD = 3.141593f / 180;
const float FOV_Y = 60.0f;
const float NEAR_PLANE = 1.0f;
const float FAR_PLANE = 100.0f;
const float CAMERA_ANGLE_X = 45.0f;     // pitch in degree
const float CAMERA_ANGLE_Y = -45.0f;    // heading in degree
const float CAMERA_DISTANCE = 25.0f;    // camera distance
// default projection matrix values
const float DEFAULT_LEFT = -0.5f;
const float DEFAULT_RIGHT = 0.5f;
const float DEFAULT_BOTTOM = -0.5f;
const float DEFAULT_TOP = 0.5f;
const float DEFAULT_NEAR = 1.0f;
const float DEFAULT_FAR = 10.0f;



///////////////////////////////////////////////////////////////////////////////
// default ctor
///////////////////////////////////////////////////////////////////////////////
ModelGL::ModelGL() : windowWidth(0), windowHeight(0), mouseLeftDown(false),
                     mouseRightDown(false), drawModeChanged(false), drawMode(0),
                     cameraAngleX(CAMERA_ANGLE_X), cameraAngleY(CAMERA_ANGLE_Y),
                     cameraDistance(CAMERA_DISTANCE), windowSizeChanged(false),
                     projectionLeft(DEFAULT_LEFT),
                     projectionRight(DEFAULT_RIGHT),
                     projectionBottom(DEFAULT_BOTTOM),
                     projectionTop(DEFAULT_TOP),
                     projectionNear(DEFAULT_NEAR),
                     projectionFar(DEFAULT_FAR),
                     quadricId(0), projectionMode(0)
{
    bgColor[0] = bgColor[1] = bgColor[2] = bgColor[3] = 0;

    // init projection matrix
    setFrustum(projectionLeft, projectionRight,
               projectionBottom, projectionTop,
               projectionNear, projectionFar);

    // model, view, modelview matrices are fixed in this app
    matrixModel.identity();
    matrixView.identity();
    matrixView.translate(0, 0, -7); // move the camera away from 3D objects
    matrixModelView = matrixView * matrixModel;
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
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glEnable(GL_SCISSOR_TEST);

     // track material ambient and diffuse from surface color, call it before glEnable(GL_COLOR_MATERIAL)
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);   // background color
    glClearStencil(0);                              // clear stencil buffer
    glClearDepth(1.0f);                             // 0 is near, 1 is far
    glDepthFunc(GL_LEQUAL);

    initLights();

    // create a GLU quadric object
    quadricId = gluNewQuadric();
    gluQuadricDrawStyle(quadricId, GLU_FILL);
}



///////////////////////////////////////////////////////////////////////////////
// clean up OpenGL objects
///////////////////////////////////////////////////////////////////////////////
void ModelGL::quit()
{
    gluDeleteQuadric(quadricId);
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
    float lightPos[4] = {0, 0, 1, 0};               // directional light
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_LIGHT0);                            // MUST enable each light source after configuration
}



///////////////////////////////////////////////////////////////////////////////
// set camera position and lookat direction
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ)
{
    float forward[4];
    float up[4];
    float left[4];
    float position[4];
    float invLength;

    // determine forward vector (direction reversed because it is camera)
    forward[0] = posX - targetX;    // x
    forward[1] = posY - targetY;    // y
    forward[2] = posZ - targetZ;    // z
    forward[3] = 0.0f;              // w
    // normalize it without w-component
    invLength = 1.0f / sqrtf(forward[0]*forward[0] + forward[1]*forward[1] + forward[2]*forward[2]);
    forward[0] *= invLength;
    forward[1] *= invLength;
    forward[2] *= invLength;

    // assume up direction is straight up
    up[0] = 0.0f;   // x
    up[1] = 1.0f;   // y
    up[2] = 0.0f;   // z
    up[3] = 0.0f;   // w

    // compute left vector with cross product
    left[0] = up[1]*forward[2] - up[2]*forward[1];  // x
    left[1] = up[2]*forward[0] - up[0]*forward[2];  // y
    left[2] = up[0]*forward[1] - up[1]*forward[0];  // z
    left[3] = 1.0f;                                 // w

    // re-compute orthogonal up vector
    up[0] = forward[1]*left[2] - forward[2]*left[1];    // x
    up[1] = forward[2]*left[0] - forward[0]*left[2];    // y
    up[2] = forward[0]*left[1] - forward[1]*left[0];    // z
    up[3] = 0.0f;                                       // w

    // camera position
    position[0] = -posX;
    position[1] = -posY;
    position[2] = -posZ;
    position[3] = 1.0f;

    // copy axis vectors to matrix
    matrixView.identity();
    matrixView.setColumn(0, left);
    matrixView.setColumn(1, up);
    matrixView.setColumn(2, forward);
    matrixView.setColumn(3, position);
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
    setFrustum(FOV_Y, (float)(w)/h, NEAR_PLANE, FAR_PLANE); // FOV, AspectRatio, NearClip, FarClip

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
            //glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        else if(drawMode == 2)      // point mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            //glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        drawModeChanged = false;
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

    // copy  ModelView matrix to OpenGL after transpose
    // This matrix is fixed and pre-computed in constructor
    glLoadMatrixf(matrixModelView.getTranspose());

    drawSpheres();

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
    gluPerspective(FOV_Y, windowWidth/(windowHeight/2.0f), 1, 1000);

    // switch to modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // clear buffer
    glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);   // background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glPushMatrix();

    // First, transform the camera (viewing matrix) from world space to eye space
    glTranslatef(0, 0, -cameraDistance);
    glRotatef(cameraAngleX, 1, 0, 0); // pitch
    glRotatef(cameraAngleY, 0, 1, 0); // heading

    // draw grid
    drawGrid(10, 1);

    // draw balls
    drawSpheres();

    // draw the camera
    glPushMatrix();
    glTranslatef(0, 0, 7);
    drawCamera();
    drawFrustum(projectionLeft, projectionRight, projectionBottom, projectionTop, projectionNear, projectionFar);
    glPopMatrix();

    glPopMatrix();
}



///////////////////////////////////////////////////////////////////////////////
// draw a grid on the xz plane
///////////////////////////////////////////////////////////////////////////////
void ModelGL::drawGrid(float size, float step)
{
    // disable lighting
    glDisable(GL_LIGHTING);

    glBegin(GL_LINES);

    glColor3f(0.3f, 0.3f, 0.3f);
    for(float i=step; i <= size; i+= step)
    {
        glVertex3f(-size, 0,  i);   // lines parallel to X-axis
        glVertex3f( size, 0,  i);
        glVertex3f(-size, 0, -i);   // lines parallel to X-axis
        glVertex3f( size, 0, -i);

        glVertex3f( i, 0, -size);   // lines parallel to Z-axis
        glVertex3f( i, 0,  size);
        glVertex3f(-i, 0, -size);   // lines parallel to Z-axis
        glVertex3f(-i, 0,  size);
    }

    // x-axis
    glColor3f(0.5f, 0, 0);
    glVertex3f(-size, 0, 0);
    glVertex3f( size, 0, 0);

    // z-axis
    glColor3f(0,0,0.5f);
    glVertex3f(0, 0, -size);
    glVertex3f(0, 0,  size);

    glEnd();

    // enable lighting back
    glEnable(GL_LIGHTING);
}



///////////////////////////////////////////////////////////////////////////////
// draw the local axis of an object
///////////////////////////////////////////////////////////////////////////////
void ModelGL::drawAxis(float size)
{
    glDepthFunc(GL_ALWAYS);     // to avoid visual artifacts with grid lines
    glDisable(GL_LIGHTING);

    // draw axis
    glLineWidth(3);
    glBegin(GL_LINES);
        glColor3f(1, 0, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(size, 0, 0);
        glColor3f(0, 1, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(0, size, 0);
        glColor3f(0, 0, 1);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, size);
    glEnd();
    glLineWidth(1);

    // draw arrows(actually big square dots)
    glPointSize(5);
    glBegin(GL_POINTS);
        glColor3f(1, 0, 0);
        glVertex3f(size, 0, 0);
        glColor3f(0, 1, 0);
        glVertex3f(0, size, 0);
        glColor3f(0, 0, 1);
        glVertex3f(0, 0, size);
    glEnd();
    glPointSize(1);

    // restore default settings
    glEnable(GL_LIGHTING);
    glDepthFunc(GL_LEQUAL);
}



///////////////////////////////////////////////////////////////////////////////
// draw spheres
///////////////////////////////////////////////////////////////////////////////
void ModelGL::drawSpheres()
{
    const int SLICES = 36;
    const int STACKS = 24;
    const float RADIUS = 0.5f;

    float color1[3] = {1, 0, 0};
    float color2[3] = {1, 0.5f, 0};
    float color3[3] = {1, 1, 0};
    float color4[3] = {0, 1, 0};
    float color5[3] = {0, 1, 1};
    float color6[3] = {0, 0, 1};
    float color7[3] = {1, 0, 1};

    // set default specular and shiniess using glMaterial
    float shininess = 64.0f;
    float specularColor[] = {1.0, 1.0f, 1.0f, 1.0f};
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess); // range 0 ~ 128
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularColor);

    glPushMatrix();
    glTranslatef(0, 0, 3);
    glColor3fv(color1);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1, 0, 2);
    glColor3fv(color2);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-1, 0, 2);
    glColor3fv(color2);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, 1, 2);
    glColor3fv(color2);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, -1, 2);
    glColor3fv(color2);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(2, 0, 1);
    glColor3fv(color3);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-2, 0, 1);
    glColor3fv(color3);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, 2, 1);
    glColor3fv(color3);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, -2, 1);
    glColor3fv(color3);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(3, 0, 0);
    glColor3fv(color4);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-3, 0, 0);
    glColor3fv(color4);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, 3, 0);
    glColor3fv(color4);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, -3, 0);
    glColor3fv(color4);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(4, 0, -1);
    glColor3fv(color5);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-4, 0, -1);
    glColor3fv(color5);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, 4, -1);
    glColor3fv(color5);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, -4, -1);
    glColor3fv(color5);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(5, 0, -2);
    glColor3fv(color6);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-5, 0, -2);
    glColor3fv(color6);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, 5, -2);
    glColor3fv(color6);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, -5, -2);
    glColor3fv(color6);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(6, 0, -3);
    glColor3fv(color7);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-6, 0, -3);
    glColor3fv(color7);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, 6, -3);
    glColor3fv(color7);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, -6, -3);
    glColor3fv(color7);
    gluSphere(quadricId, RADIUS, SLICES, STACKS);
    glPopMatrix();
}



///////////////////////////////////////////////////////////////////////////////
// rotate the camera for subWin2
///////////////////////////////////////////////////////////////////////////////
void ModelGL::rotateCamera(int x, int y)
{
    cameraAngleY += (x - mouseX);
    cameraAngleX += (y - mouseY);
    mouseX = x;
    mouseY = y;
}



///////////////////////////////////////////////////////////////////////////////
// zoom the camera for subWin2
///////////////////////////////////////////////////////////////////////////////
void ModelGL::zoomCamera(int y)
{
    cameraDistance -= (y - mouseY) * 0.1f;
    mouseY = y;
}
void ModelGL::zoomCameraDelta(int delta)
{
    cameraDistance -= delta;
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
// draw frustum
///////////////////////////////////////////////////////////////////////////////
void ModelGL::drawFrustum(float fovY, float aspectRatio, float nearPlane, float farPlane)
{
    float tangent = tanf(fovY/2 * DEG2RAD);
    float nearHeight = nearPlane * tangent;
    float nearWidth = nearHeight * aspectRatio;

    drawFrustum(-nearWidth, nearWidth, -nearHeight, nearHeight, nearPlane, farPlane);
}



///////////////////////////////////////////////////////////////////////////////
// draw frustum with 6 params (left, right, bottom, top, near, far)
///////////////////////////////////////////////////////////////////////////////
void ModelGL::drawFrustum(float l, float r, float b, float t, float n, float f)
{
    computeFrustumVertices(l, r, b, t, n, f);

    float colorLine1[4]  = { 0.7f, 0.7f, 0.7f, 0.7f };
    float colorLine2[4]  = { 0.2f, 0.2f, 0.2f, 0.7f };
    float colorPlane1[4] = { 0.5f, 0.5f, 0.5f, 0.5f };

    // draw lines
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // draw the edges around frustum
    if(projectionMode == 0)
    {
        glBegin(GL_LINES);
        glColor4fv(colorLine2);
        glVertex3f(0, 0, 0);
        glColor4fv(colorLine1);
        glVertex3fv(&frustumVertices[4].x);

        glColor4fv(colorLine2);
        glVertex3f(0, 0, 0);
        glColor4fv(colorLine1);
        glVertex3fv(&frustumVertices[5].x);

        glColor4fv(colorLine2);
        glVertex3f(0, 0, 0);
        glColor4fv(colorLine1);
        glVertex3fv(&frustumVertices[6].x);

        glColor4fv(colorLine2);
        glVertex3f(0, 0, 0);
        glColor4fv(colorLine1);
        glVertex3fv(&frustumVertices[7].x);
        glEnd();
    }
    else
    {
        glColor4fv(colorLine1);
        glBegin(GL_LINES);
        glVertex3fv(&frustumVertices[0].x);
        glVertex3fv(&frustumVertices[4].x);
        glVertex3fv(&frustumVertices[1].x);
        glVertex3fv(&frustumVertices[5].x);
        glVertex3fv(&frustumVertices[2].x);
        glVertex3fv(&frustumVertices[6].x);
        glVertex3fv(&frustumVertices[3].x);
        glVertex3fv(&frustumVertices[7].x);
        glEnd();
    }

    glColor4fv(colorLine1);
    glBegin(GL_LINE_LOOP);
    glVertex3fv(&frustumVertices[4].x);
    glVertex3fv(&frustumVertices[5].x);
    glVertex3fv(&frustumVertices[6].x);
    glVertex3fv(&frustumVertices[7].x);
    glEnd();

    glColor4fv(colorLine1);
    glBegin(GL_LINE_LOOP);
    glVertex3fv(&frustumVertices[0].x);
    glVertex3fv(&frustumVertices[1].x);
    glVertex3fv(&frustumVertices[2].x);
    glVertex3fv(&frustumVertices[3].x);
    glEnd();

    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);

    // frustum is transparent.
    // Draw the frustum faces twice: backfaces first then frontfaces second.
    for(int i = 0; i < 2; ++i)
    {
        if(i == 0)
        {
            // for inside planes
            //glCullFace(GL_FRONT);
            //glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
        }
        else
        {
            // draw outside planes
            glCullFace(GL_BACK);
            glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
        }

        glColor4fv(colorPlane1);
        glBegin(GL_QUADS);
        // left
        glNormal3fv(&frustumNormals[0].x);
        glVertex3fv(&frustumVertices[1].x);
        glVertex3fv(&frustumVertices[5].x);
        glVertex3fv(&frustumVertices[6].x);
        glVertex3fv(&frustumVertices[2].x);
        // right
        glNormal3fv(&frustumNormals[1].x);
        glVertex3fv(&frustumVertices[0].x);
        glVertex3fv(&frustumVertices[3].x);
        glVertex3fv(&frustumVertices[7].x);
        glVertex3fv(&frustumVertices[4].x);
        // bottom
        glNormal3fv(&frustumNormals[2].x);
        glVertex3fv(&frustumVertices[2].x);
        glVertex3fv(&frustumVertices[6].x);
        glVertex3fv(&frustumVertices[7].x);
        glVertex3fv(&frustumVertices[3].x);
        // top
        glNormal3fv(&frustumNormals[3].x);
        glVertex3fv(&frustumVertices[0].x);
        glVertex3fv(&frustumVertices[4].x);
        glVertex3fv(&frustumVertices[5].x);
        glVertex3fv(&frustumVertices[1].x);
        // front
        glNormal3fv(&frustumNormals[4].x);
        glVertex3fv(&frustumVertices[0].x);
        glVertex3fv(&frustumVertices[1].x);
        glVertex3fv(&frustumVertices[2].x);
        glVertex3fv(&frustumVertices[3].x);
        // back
        glNormal3fv(&frustumNormals[5].x);
        glVertex3fv(&frustumVertices[7].x);
        glVertex3fv(&frustumVertices[6].x);
        glVertex3fv(&frustumVertices[5].x);
        glVertex3fv(&frustumVertices[4].x);
        glEnd();
    }
}



///////////////////////////////////////////////////////////////////////////////
// compute 8 vertices of frustum
///////////////////////////////////////////////////////////////////////////////
void ModelGL::computeFrustumVertices(float l, float r, float b, float t, float n, float f)
{
    float ratio;
    float farLeft;
    float farRight;
    float farBottom;
    float farTop;

    // perspective mode
    if(projectionMode == 0)
        ratio     = f / n;
    // orthographic mode
    else
        ratio = 1;
    farLeft   = l * ratio;
    farRight  = r * ratio;
    farBottom = b * ratio;
    farTop    = t * ratio;

    // compute 8 vertices of the frustum
    // near top right
    frustumVertices[0].x = r;
    frustumVertices[0].y = t;
    frustumVertices[0].z = -n;

    // near top left
    frustumVertices[1].x = l;
    frustumVertices[1].y = t;
    frustumVertices[1].z = -n;

    // near bottom left
    frustumVertices[2].x = l;
    frustumVertices[2].y = b;
    frustumVertices[2].z = -n;

    // near bottom right
    frustumVertices[3].x = r;
    frustumVertices[3].y = b;
    frustumVertices[3].z = -n;

    // far top right
    frustumVertices[4].x = farRight;
    frustumVertices[4].y = farTop;
    frustumVertices[4].z = -f;

    // far top left
    frustumVertices[5].x = farLeft;
    frustumVertices[5].y = farTop;
    frustumVertices[5].z = -f;

    // far bottom left
    frustumVertices[6].x = farLeft;
    frustumVertices[6].y = farBottom;
    frustumVertices[6].z = -f;

    // far bottom right
    frustumVertices[7].x = farRight;
    frustumVertices[7].y = farBottom;
    frustumVertices[7].z = -f;

    // compute normals
    frustumNormals[0] = (frustumVertices[5] - frustumVertices[1]).cross(frustumVertices[2] - frustumVertices[1]);
    frustumNormals[0].normalize();

    frustumNormals[1] = (frustumVertices[3] - frustumVertices[0]).cross(frustumVertices[4] - frustumVertices[0]);
    frustumNormals[1].normalize();

    frustumNormals[2] = (frustumVertices[6] - frustumVertices[2]).cross(frustumVertices[3] - frustumVertices[2]);
    frustumNormals[2].normalize();

    frustumNormals[3] = (frustumVertices[4] - frustumVertices[0]).cross(frustumVertices[1] - frustumVertices[0]);
    frustumNormals[3].normalize();

    frustumNormals[4] = (frustumVertices[1] - frustumVertices[0]).cross(frustumVertices[3] - frustumVertices[0]);
    frustumNormals[4].normalize();

    frustumNormals[5] = (frustumVertices[7] - frustumVertices[4]).cross(frustumVertices[5] - frustumVertices[4]);
    frustumNormals[5].normalize();
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
