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
#include "camera/camera.h"
#include "shader/shader.h"
#include "renderables/bsp.h"
#include "renderables/object.h"

#include <chrono>
using namespace std::chrono;

typedef struct {
    bool forward;
    bool left;
    bool right;
    bool back;
    bool shift;
    bool control;
} Control;

class ERenderer {
private:
    // movement
    milliseconds tick;
    milliseconds forward_tick;
    milliseconds strafe_tick;
    
    // rendering
    bool ready = false;
    ProtonMap *map;
    ShaderManager *shaders;
    BSP *bsp;
    Camera *camera;
    ObjectManager *objects;
    Control *controller = nullptr;
public:
    ~ERenderer();
    void setup(const char *resources);
    void resize(float width, float height);
    void render();
    
    void read(ProtonMap *map);
    void write();
    
    // Movement
    void applyControl(Control *control);
    
    void mouseDown(float dx, float dy);
    void mouseDrag(float dx, float dy);
    void rightMouseDrag(float dx, float dy);
};

#endif /* defined(____render__) */
