//
//  render.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "render.h"

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
    bsp    = new BSP();
    camera = new Camera();
    ready  = false;
    
    // Shaders
    load_shaders();
}

// Load a map
void ERenderer::setMap(ProtonMap *map) {
    this->map = map;
    
    uint16_t scenarioTag = map->principal_tag;
    if (scenarioTag != NULLED_TAG_ID) {
        
        ProtonTag *scenarioTag = map->tags.at(map->principal_tag).get();
        HaloScenarioTag *scenario = (HaloScenarioTag *)(scenarioTag->Data());
        this->bsp->setup(map, scenarioTag);
        
        // Load Objects
        int i;
        for (i=0; i < scenario->scenRef.count; i++) {
        }
        
        printf("This map has %d types of scenery\n", scenario->scenRef.count);
        printf("This map has %d types of vehicles\n", scenario->vehiRef.count);
        printf("This map has %d types of equipment\n", scenario->eqipRef.count);
        
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

// Main rendering loop
void ERenderer::render() {
    printf("render\n");
    if (!ready) {
        return;
    }
    
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
        
        for (int pass = shader_SENV; pass != shader_SENV; pass++ )
        {
            ShaderType type = static_cast<ShaderType>(pass);
            shader *shader = get_shader(type);
            shader->start();
            this->bsp->render(map, scenarioTag);
            shader->stop();
        }
        
        // Render Objects
        int i;
        for (i=0; i < scenario->scen.count; i++) {
            
        }
    }
    
    // Print any errors
    GLenum error = glGetError();
    if( error != GL_NO_ERROR )
    {
        fprintf(stderr, "Opengl error %s", gluErrorString( error ));
    }
    
    glFlush();
}