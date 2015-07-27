///////////////////////////////////////////////////////////////////////////////
// ModelGL.h
// =========
// Model component of OpenGL
// 
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2006-07-10
// UPDATED: 2006-07-10
///////////////////////////////////////////////////////////////////////////////

#ifndef MODEL_GL_H
#define MODEL_GL_H

class ModelGL
{
public:
    ModelGL();                                      // ctor
    ~ModelGL();                                     // dtor

    void init();                                    // initialize OpenGL states
    void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ);
    void setViewport(int width, int height);
    void draw();

    void setMouseLeft(bool flag) { mouseLeftDown = flag; };
    void setMouseRight(bool flag) { mouseRightDown = flag; };
    void setMousePosition(int x, int y) { mouseX = x; mouseY = y; };

    void rotateCamera(int x, int y);
    void zoomCamera(int dist);


protected:

private:
    // member functions
    void initLights();                              // add a white light ti scene

    // members
    bool mouseLeftDown;
    bool mouseRightDown;
    int mouseX;
    int mouseY;
    float cameraAngleX;
    float cameraAngleY;
    float cameraDistance;
};
#endif
