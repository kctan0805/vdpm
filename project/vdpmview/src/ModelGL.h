///////////////////////////////////////////////////////////////////////////////
// ModelGL.h
// =========
// Model component of OpenGL
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2008-10-02
// UPDATED: 2013-03-01
///////////////////////////////////////////////////////////////////////////////

#ifndef MODEL_GL_H
#define MODEL_GL_H

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "Matrices.h"
#include "Vectors.h"
#include "Bmp.h"
#include "vdpm/Types.h"

using namespace vdpm;

#define MAX_MESH_COUNT 8

class ModelGL
{
public:
    ModelGL();
    ~ModelGL();

    void init();                                    // initialize OpenGL states
    void quit();                                    // clean up OpenGL objects
    void draw();

    void setMouseLeft(bool flag)        { mouseLeftDown = flag; };
    void setMouseRight(bool flag)       { mouseRightDown = flag; };
    void setMousePosition(int x, int y) { mouseX = x; mouseY = y; };
    void setDrawMode(int mode);
    void setWindowSize(int width, int height);
    void setProjection(float l, float r, float b, float t, float n, float f);
    void setProjectionMode(int mode)    { projectionMode = mode; updateProjectionMatrix(); };
    int  getProjectionMode()            { return projectionMode; };

    const float* getProjectionMatrixElements()  { return matrixProjection.get(); }

    void rotateCamera(int x, int y);
    void zoomCamera(int dist);
    void zoomCameraDelta(int delta);        // for mousewheel

    void loadSRMesh(const char fileLine[]);
    void loadTexture(const char fileLine[]);
    void unload();

    void setTau(float tau);
    void setAFaces(unsigned int count);
    void setGTime(int gtime);
    void setAmortize(int step);
    float getTau();
    unsigned int getAFaces();
    const Point& getViewPosition();
    int getFps() { return fps; }
    unsigned int getTStripCount();
    unsigned int getVMorphCount();
    void enableColor();
    void disableColor();
    void enableTexture();
    void disableTexture();
    void enableLighting();
    void disableLighting();
    void testVsplit();
    void testEcol();

protected:

private:
    // member functions
    void initLights();                              // add a white light ti scene
    void setViewport(int x, int y, int width, int height);
    void setViewportSub(int left, int bottom, int width, int height, float nearPlane, float farPlane);
    void setViewportSub(int left, int bottom, int width, int height, float l, float r, float b, float t, float n, float f);
    void drawSub1();                                // draw upper window
    void drawSub2();                                // draw bottom window
    void setFrustum(float l, float r, float b, float t, float n, float f);
    void setFrustum(float fovy, float ratio, float n, float f);
    void setOrthoFrustum(float l, float r, float b, float t, float n=-1, float f=1);
    void updateProjectionMatrix();

    // members
    int windowWidth;
    int windowHeight;
    bool windowSizeChanged;
    bool drawModeChanged;
    int drawMode;
    int projectionMode;
    bool mouseLeftDown;
    bool mouseRightDown;
    int mouseX;
    int mouseY;
    float projectionLeft;
    float projectionRight;
    float projectionBottom;
    float projectionTop;
    float projectionNear;
    float projectionFar;
    Vector3 frustumVertices[8];         // 8 vertices of frustum
    Vector3 frustumNormals[6];          // 6 face normals of frustum
    float cameraPosition[3];

    // these are for 3rd person view
    float cameraAngleX;
    float cameraAngleY;
    float cameraDistance;
    float bgColor[4];

    // 4x4 transform matrices
    Matrix4 matrixView;
    Matrix4 matrixModel;
    Matrix4 matrixModelView;
    Matrix4 matrixProjection;

    SRMesh* meshes[MAX_MESH_COUNT];
    GLuint texture;
    Image::Bmp* bmp;
    bool viewChanged, textureChanged, paramChanged, initialized, colorEnabled, textureEnabled, lightEnabled, fillChanged, fileLoading, fileUnloading, testingVsplit, testingEcol;
    float tau, radius;
    unsigned int targetAfaceCount;
    int gtime, amortizeStep, fps, meshCount;
};
#endif
