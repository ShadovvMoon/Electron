//
//  defines.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef _defines_h
#define _defines_h

#define RENDER_VAO
#define RENDER_GPU
//#define RENDER_FAST // Render as fast as possible

// Definitions
#ifdef _WIN32
#define _WINDOWS 1
#elif _WIN64
#define _WINDOWS 1
#endif

#include <stdio.h>
#include <stdint.h>

#include "tags/ZZTHaloScenarioTag.h"
#include "tags/ZZTHaloObjectTag.h"
#include "ProtonMap.h"

// Custom includes
#ifdef _WINDOWS
#include "glew/GL/glew.h"
#elif __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#endif

#ifdef RENDER_FAST
#define BITS_PER_PIXEL 16.0
#define DEPTH_SIZE 16.0
#else
#define BITS_PER_PIXEL 32.0
#define DEPTH_SIZE 32.0
#endif

#define MAX_SCENARIO_OBJECTS 100000

#define ShaderCount 7
typedef enum {
    shader_NULL = 0,
    shader_SENV = 1,
    shader_SCHI = 2,
    shader_SOSO = 3,
    shader_SGLA = 4,
    shader_SWAT = 5,
    shader_SCEX = 6,
} ShaderType;

// VBO
#define INDEX_BUFFER 0
#define POS_VB 1
#define NORMAL_VB 2
#define TEXCOORD_VB 3
#define LIGHT_VB 4
#define BINORMAL_VB 5
#define TANGENT_VB 6

// Functions
void errorCheck();

#endif
