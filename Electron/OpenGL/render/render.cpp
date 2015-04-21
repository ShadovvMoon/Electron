//
//  render.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "render.h"

milliseconds now() {
    return duration_cast< milliseconds >(high_resolution_clock::now().time_since_epoch());
}

// Setup
void ERenderer::setup() {
    printf("setup\n");
#ifdef _WINDOWS
    GLenum error = glewInit();
    if (error != GLEW_OK)
    {
        printf ("An error occurred with glew %d: %s \n", error, (char *) glewGetErrorString(error));
    }
#endif
    
    // Initilisation
    camera  = new Camera;
    shaders = new ShaderManager;
    bsp     = new BSP(shaders);
    objects = new ObjectManager;
    
    // Start the tick
    tick = now();
    forward_tick = now();
    strafe_tick = now();
    
    // Start
    ready = false;
    printf("ready\n");
}

// Load a map
void ERenderer::setMap(ProtonMap *map) {
    printf("set map\n");
    this->map = map;
    
    uint16_t scenarioTag = map->principal_tag;
    if (scenarioTag != NULLED_TAG_ID) {
        
        ProtonTag *scenarioTag = map->tags.at(map->principal_tag).get();
        bsp->setup(map, scenarioTag);
        objects->read(shaders, map, scenarioTag);
    }
    ready = true;
}

void ERenderer::resize(float width, float height) {

    // Viewport
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (width / height), 0.1f, 4000000.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void errorCheck() {
    // Print any errors
    GLenum error = glGetError();
    if( error != GL_NO_ERROR )
    {
        fprintf(stderr, "Opengl error %s\n", gluErrorString( error ));
    }
}

void ERenderer::mouseDrag(float dx, float dy) {
    camera->drag(dx, dy);
}

void ERenderer::applyControl(Control *control){
    if (!ready) {
        return;
    }
    
    milliseconds current = now();
    double seconds = (current.count() - tick.count()) / 1000.0;
    double forward_seconds = (current.count() - forward_tick.count()) / 1000.0;
    double  strafe_seconds = (current.count() -  strafe_tick.count()) / 1000.0;
    
    float speed = 1;
    float fspeed = speed * forward_seconds;
    float sspeed = speed * strafe_seconds;
    if (control->forward && !control->back) {
        camera->move(fspeed);
    } else if (control->back) {
        camera->move(-fspeed);
    } else {
        forward_tick = now();
    }
    
    if (control->left && !control->right) {
        camera->strafe(-sspeed);
    } else if (control->right) {
        camera->strafe(+sspeed);
    } else {
        strafe_tick = now();
    }
    tick = now();
}

// Main rendering loop
void ERenderer::render() {
    if (!ready) {
        return;
    }
    errorCheck();
    
    // Setup the current viewport
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.2,0.2,0.2,1.0);
    
    // Enable GL states
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    camera->look();

    // Render the scenario
    uint16_t scenarioTag = map->principal_tag;
    if (scenarioTag != NULLED_TAG_ID) {
        ProtonTag *scenarioTag = map->tags.at(map->principal_tag).get();
        HaloScenarioTag *scenario = (HaloScenarioTag *)(scenarioTag->Data());
        
        for (int pass = shader_SENV; pass <= shader_SOSO; pass++ )
        {
            ShaderType type = static_cast<ShaderType>(pass);
            shader *shader = shaders->get_shader(type);
            shader->start();
            bsp->render(type);
            objects->render(type);
            shader->stop();
        }
    }
}