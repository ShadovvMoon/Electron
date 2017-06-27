//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef ____render__
#define ____render__

#include "defines.h"
#include "math.h"
#include "camera.h"
#include "glshader.h"
#include "bsp.h"
#include "object.h"
#include "sky.h"
#include "gui.h"


#include <chrono>
using namespace std::chrono;

typedef struct {
    bool forward;
    bool forwardSlow;
    bool left;
    bool right;
    bool back;
    bool shift;
    bool control;
} Control;

class ERenderer: Pipeline {
private:
    // movement
    milliseconds tick;
    milliseconds forward_tick;
    milliseconds strafe_tick;
    
    // rendering
    bool ready = false;
    
    // deferred rendering
    GLuint mFBO;
    GLuint			mDiffuse; // The diffuse render target
    unsigned int	mDiffuseTexture; // The OpenGL texture for the diffuse render target
    GLuint			mPosition; // The position render target
    unsigned int	mPositionTexture; // The OpenGL texture for the position render target
    GLuint			mNormals; // The normals render target
    unsigned int	mNormalsTexture; // The OpenGL texture for the normals render target
    GLuint			mReflections; // The normals render target
    unsigned int	mReflectionsTexture; // The OpenGL texture for the normals render target
    GLuint			mDepthBuffer; // Depth buffer handle
    unsigned int	mDepthTexture; // The OpenGL texture for the normals render target
    unsigned int	m_width; // FBO width
    unsigned int	m_height; // FBO height
    unsigned int	mSSAOTexture;
    
    // shader options
    shader_options *options;
    
    // private functions
    void start();
    void stop();
public:
    bool useSSAO = false;
    bool render_objects = true;
    
    BSP *bsp;
    ProtonMap *map;
    ShaderManager *shaders;
    Camera *camera;
    ObjectManager *objects;
    SkyManager *skies;
    Control *controller = nullptr;
    GUI *interface;
    
    ~ERenderer();
    void setup(const char *resources);
    void resize(float width, float height);
    void render();
    void renderScene(bool fast, bool reflect);
    
    void read(ProtonMap *map);
    void write();
    
    // Movement
    void applyControl(Control *control);
    
    void mouseDown(float dx, float dy);
    void mouseDrag(float dx, float dy);
    void rightMouseDrag(float dx, float dy);
    
    // Pipeline
    Mesh *createMesh(int count, int verts);
};

#endif /* defined(____render__) */
