//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#ifndef __BSP__
#define __BSP__

#include "defines.h"
#include "vector3d.h"
#include "pipeline.hpp"

class BSPRenderBuffer {
public:
    GLuint m_Buffers[7];
    GLuint geometryVAO;
    GLfloat* vertex_array;
    GLfloat* texture_uv;
    GLfloat* light_uv;
    GLfloat* normals;
    GLfloat* binormals;
    GLfloat* tangents;
    GLint* index_array;
    void setup();
};

class BSPRenderSubmesh {
public:
    GLuint m_Buffers[7];
    
    // Rendering
    GLuint geometryVAO;
    int indexCount;
    int vertCount;
    int indexOffset;
    int vertexOffset;
    int lightmap;
    
    shader_object *shader;
    void setup();
};

class BSPRenderMesh {
public:
    texture *lightTexture;
    std::vector<BSPRenderSubmesh*> submeshes;
};

// Helper functions
typedef struct {
    uint32_t plane;
    uint32_t back;
    uint32_t front;
} BSP3DNode; //size = 12
typedef struct {
    float a;
    float b;
    float c;
    float d;
} Plane; //size = 16
typedef struct {
    uint16_t flags;
    uint16_t bsp2dCount;
    uint32_t firstRef;
} Leaf; // size = 8
typedef struct {
    uint32_t plane;
    uint32_t bsp2DNode;
} BSP2DRef; //size = 8
typedef struct {
    float a;
    float b;
    float c;
    uint32_t left;
    uint32_t right;
} BSP2DNode; //size = 20
typedef struct {
    uint32_t plane;
    uint32_t edge;
    uint16_t flags;
    uint16_t material;
} Surface;
typedef struct {
    uint32_t startVert;
    uint32_t endVert;
    uint32_t forwardEdge;
    uint32_t prevEdge;
    uint32_t leftSurface;
    uint32_t rightSurface;
} Edge;
typedef struct {
    float x;
    float y;
    float z;
    uint32_t firstEdge;
} CollVert;
typedef struct {
    HaloTagReflexive BSP3DNodes; //0x00
    HaloTagReflexive Planes;     //0x0C
    HaloTagReflexive Leaves;     //0x18
    HaloTagReflexive BSP2DRef;   //0x24
    HaloTagReflexive BSP2DNodes; //0x30
    HaloTagReflexive Surfaces;   //0x3C
    HaloTagReflexive Edges;      //0x48
    HaloTagReflexive Vertices;   //0x54
} CollisionBSP;

BSP3DNode *node3d(ProtonTag *bspTag, CollisionBSP *collison, int i);
Plane *plane(ProtonTag *bspTag, CollisionBSP *collison, int i);
Leaf *leaf(ProtonTag *bspTag, CollisionBSP *collison, int i);
BSP2DRef *ref2d(ProtonTag *bspTag, CollisionBSP *collison, int i);
BSP2DNode *node2d(ProtonTag *bspTag, CollisionBSP *collison, int i);
Surface *surface(ProtonTag *bspTag, CollisionBSP *collison, int i);
Edge *edge(ProtonTag *bspTag, CollisionBSP *collison, int i);
CollVert *vert(ProtonTag *bspTag, CollisionBSP *collison, int i);
uint8_t* map2mem(ProtonTag *scenario, uint32_t address);

typedef struct intersection {
    vector3d *position = NULL;
    uint32_t surface = -1;
    bool accurate = false;
    struct intersection *next = NULL;
} intersection;

class BSP {
#ifdef RENDER_PIPELINE
    Mesh *mesh;
#else
#endif
    
    BSPRenderBuffer *vao;
    ShaderManager *shaders;
    std::vector<BSPRenderMesh*> renderables;
    std::vector<vector3d*> light_experiment;
    void render_experimental(ShaderType pass);
public:
    BSP(ShaderManager* manager);
    void setup(ProtonMap *map, ProtonTag *scenario, Pipeline *pipeline);
    void render(ShaderType pass);
    void generate_lightmap(vector3d *sun, ProtonMap *map, ProtonTag *scenario);
    
    vector3d *intersect_vision(vector3d *p, vector3d *q, ProtonMap *map, ProtonTag *scenario);
    vector3d *intersect(vector3d *p, vector3d *q, ProtonMap *map, ProtonTag *scenario);
    vector3d *intersect_report(vector3d *p, vector3d *q, ProtonMap *map, ProtonTag *scenario, intersection *output);
};

#endif /* defined(__BSP__) */
