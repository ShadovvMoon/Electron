//
//  render.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "render.h"

// Definitions
#ifdef _WIN32
    #define _WINDOWS 1
#elif _WIN64
    #define _WINDOWS 1
#endif

// OpenGL includes
#ifdef _WINDOWS
    #include "glew/glew.h"
#elif __APPLE__
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
    #include <GLUT/glut.h>
#endif

// Setup
void ERenderer::setup() {
#ifdef _WINDOWS
    GLenum error = glewInit();
    if (error != GLEW_OK)
    {
        printf ("An error occurred with glew %d: %s \n", error, (char *) glewGetErrorString(error));
    }
#endif
    
    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);
}

// Load a map
void ERenderer::setMap(ProtonMap *map) {
    uint16_t scenarioTag = map->principal_tag;
    if (scenarioTag != NULLED_TAG_ID) {
        
        
        /*
         void OffsetData(uint32_t offset, uint32_t size);
         [11:19am] Zero2:     void AppendData(uint32_t offset, uint32_t size);
         [11:19am] Zero2:     void DeleteData(uint32_t offset, uint32_t size);
         [11:19am] Zero2:     void InsertData(uint32_t offset, const char *data, uint32_t size);
         */
        
    }
}

// Main rendering loop
void ERenderer::render() {
    
}