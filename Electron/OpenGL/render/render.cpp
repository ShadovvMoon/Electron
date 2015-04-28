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
void ERenderer::setup(const char *resources) {
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
    shaders = new ShaderManager(resources);
    bsp     = new BSP(shaders);
    objects = new ObjectManager;
    skies   = new SkyManager;
    
    // Start the tick
    tick = now();
    forward_tick = now();
    strafe_tick = now();
    
    // Start
    ready = false;
    printf("ready\n");
}

// Load a map
void ERenderer::write() {
    uint16_t scenarioTag = map->principal_tag;
    if (scenarioTag != NULLED_TAG_ID) {
        ProtonTag *scenarioTag = map->tags.at(map->principal_tag).get();
        objects->write(map, scenarioTag);
    }
}

GLuint reflect_texture;
void ERenderer::read(ProtonMap *map) {
    printf("set map\n");
    this->map = map;

    // Load the map
    uint16_t scenarioTag = map->principal_tag;
    if (scenarioTag != NULLED_TAG_ID) {
        
        ProtonTag *scenarioTag = map->tags.at(map->principal_tag).get();
        bsp->setup(map, scenarioTag);
        objects->read(shaders, map, scenarioTag);
        skies->read(objects, map, scenarioTag);
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
    if( error != GL_NO_ERROR ) {
        fprintf(stderr, "Opengl error %s\n", gluErrorString( error ));
    }
}

void ERenderer::rightMouseDrag(float dx, float dy) {
    camera->drag(dx, dy);
}

void ERenderer::mouseDrag(float dx, float dy) {
    if (objects->selection.size() > 0) {
        int i;
        for (i=0; i < objects->selection.size(); i++) {
            ObjectInstance *instance = objects->selection[i];
            vector3d *coord = new vector3d(instance->x, instance->y, instance->z);
            vector3d *move = new vector3d(dx/200, -dy/200, 0);
            vector3d *viewDirection = new vector3d(0,0,0);
            if (controller->control) {
                move->x = 0;
                move->y = 0;
                move->z = dy/10;
            }
            viewDirection->set(camera->position);
            viewDirection->sub(coord);
            float s_acceleration = 1.0;
            float mx;
            float my;
            float mz;
            if (controller->control) {
                mx = 0.0;
                my = 0.0;
                mz = s_acceleration * move->z;
            } else {
                mx = (s_acceleration * move->y * viewDirection->x);
                my = (s_acceleration * move->y * viewDirection->y);
                mz = 0.0;
                mx += (s_acceleration * move->x * ((0 * viewDirection->z) - (1 * viewDirection->y)));
                my += (s_acceleration * move->x * ((1 * viewDirection->x) - (0 * viewDirection->z)));
            }
            instance->x += mx;
            instance->y += my;
            instance->z += mz;
            delete coord;
            delete move;
            delete viewDirection;
        }
    }
}

void ERenderer::mouseDown(float dx, float dy) {
    if (controller == nullptr) {
        return;
    }
    
    objects->select(controller->shift, dx, dy);
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
    controller = control;
    tick = now();
}

void ERenderer::renderScene(bool fast) {
    uint16_t scenarioTag = map->principal_tag;
    if (scenarioTag != NULLED_TAG_ID) {
        ProtonTag *scenarioTag = map->tags.at(map->principal_tag).get();
        HaloScenarioTag *scenario = (HaloScenarioTag *)(scenarioTag->Data());
        
        // Render the sky
        shader *scex = shaders->get_shader(shader_SCEX);
        scex->start();
        skies->render(shader_SCEX);
        scex->stop();
        shader *schi = shaders->get_shader(shader_SCHI);
        schi->start();
        skies->render(shader_SCHI);
        schi->stop();

        // Render everything else
        //glAlphaFunc(GL_GREATER, 0.1);
        for (int pass = ShaderStart; pass <= ShaderEnd; pass++ )
        {
            if (fast && (pass == shader_SCEX || pass == shader_SCHI || pass == shader_SGLA || pass == shader_SWAT)) continue;
            ShaderType type = static_cast<ShaderType>(pass);
            shader *shader = shaders->get_shader(type);
            shader->start();
            bsp->render(type);
            GLuint number = 0;
            objects->render(&number, nullptr, type);
            shader->stop();
        }
    }
}

int NextHighestPowerOf2(int n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
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
    glDepthFunc(GL_LEQUAL);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    camera->look();

    glEnableClientState(GL_VERTEX_ARRAY);
    if (shaders->needs_reflection()) {
        // Render a reflection of the scenario
        GLint anViewport[4];
        glGetIntegerv(GL_VIEWPORT, anViewport);
        
        float reflectionHeight = 0.0;
        glPushMatrix();
        glScalef(1.0, 1.0, -1.0);
        glTranslatef(0.0f, 0.0f, 2*reflectionHeight);
        double plane[4] = {0.0, 0.0, 1.0, -reflectionHeight}; //water at y=0
        glEnable(GL_CLIP_PLANE0);
        glClipPlane(GL_CLIP_PLANE0, plane);
        renderScene(true);
        glPopMatrix();
        glDisable(GL_CLIP_PLANE0);
        
        // Export the reflection into the reflection texture
        glBindTexture(GL_TEXTURE_2D, shaders->get_reflection(reflection_z));
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0,0, NextHighestPowerOf2(anViewport[2]), NextHighestPowerOf2(anViewport[3]), 0);
        
        // Clear previous frame values
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.2,0.2,0.2,1.0);
    }
    
    // Render the scene with reflections
    renderScene(false);
    glDisableClientState(GL_VERTEX_ARRAY);
}

// Cleanup
ERenderer::~ERenderer() {
    printf("deleting renderer\n");
    delete camera;
    delete shaders;
    delete bsp;
    delete objects;
}