//
//  defines.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef _defines_h
#define _defines_h

// Rendering flags
#define SHADOW_MAP_SIZE 1024
#define RENDER_SWAT_REFLECTION
#define RENDER_SENV_REFLECTION

// Force hardware acceleration
#define RENDER_PIPELINE
#define RENDER_GPU
#ifdef RENDER_PIPELINE
    //#define RENDER_CORE_32
#endif

//#define GL_VALIDATE
// Render using vertex array objects (if false, use arrays or vbos)
#define RENDER_VAO
#define RENDER_VBO

// Use standard VAOS (non APPLE)
#ifdef RENDER_CORE_32
    #define RENDER_VAO
    #define RENDER_VBO
    #define RENDER_VAO_NORMAL
#endif

// Don't remember what this does
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
#define _USE_MATH_DEFINES
#include "glew/GL/glew.h"
#define RENDER_VAO_NORMAL
#elif __APPLE__
#include <OpenGL/gl3.h>
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

#define ShaderCount 11
#define ShaderStart 0
typedef enum {
    shader_NULL = 0,
    shader_SENV = 1,
    shader_SCHI = 2,
    shader_SGLA = 3,
    shader_SWAT = 4,
    shader_SCEX = 5,
    shader_SOSO = 6,
    shader_DEFF = 7,
    shader_SSAO = 8,
    shader_BLUR = 9,
    shader_SENV_REFLECT = 10
} ShaderType;
#define ShaderEnd 6

int NextHighestPowerOf2(int n);
#define Reflections 2
typedef enum {
    reflection_z = 0,
    reflection_sky = 1,
} Reflection;

// Camera
typedef struct {
    float fogr, fogg, fogb;
    float fogdist, fogcut, foglegacy;
    float perspective[16];
    float modelview[16];
    float position[3];
    float rotation[3];
    float camera[3];
} shader_options;

// VBO
#define INDEX_BUFFER 0
#define POS_VB 1
#define NORMAL_VB 2
#define TEXCOORD_VB 3
#define LIGHT_VB 4
#define BINORMAL_VB 5
#define TANGENT_VB 6

#define texCoord_buffer 1
#define texCoord_buffer_light 3
#define normals_buffer 2
#define binormals_buffer 5
#define tangents_buffer 6

// Functions
void errorCheck();

#endif
