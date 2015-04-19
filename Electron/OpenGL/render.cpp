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

typedef struct {
    uint32_t start;
    uint32_t size;
    uint32_t magic;
    uint32_t zero1;
    uint32_t tag;
    uint32_t NamePtr;
    uint32_t unk0;
    uint32_t tagId;
} __attribute__((packed)) BSP_CHUNK;

typedef struct {
    TagDependency lightmaps;
    uint32_t unk0[0x25];
    Reflexive shaders;
    Reflexive collBSP;
    Reflexive nodes;
    uint32_t unk1[0x6];
    Reflexive leaves;
    Reflexive surfaces;
    Reflexive submeshIndices;
    Reflexive submeshHeader;
    Reflexive unk2;
    Reflexive unk3;
    Reflexive unk4;
    Reflexive clusters;
    uint32_t clusterSize;
    uint32_t unk5;
    Reflexive unk6;
    Reflexive clusterPortals;
    Reflexive unk7;
    Reflexive breakableSurfaces;
    Reflexive fogPlanes;
    Reflexive fogRegions;
    Reflexive weatherPalette;
    Reflexive unk8;
    Reflexive unk9;
    Reflexive weather;
    Reflexive weatherPolyhedra;
    Reflexive unk10;
    Reflexive unk11;
    Reflexive pathfinding;
    Reflexive unk12;
    Reflexive backgroundSound;
    Reflexive environmentSound;
    uint32_t soundSize;
    uint32_t unk13;
    Reflexive unk14;
    Reflexive unk15;
    Reflexive unk16;
    Reflexive markers;
    Reflexive dobc;
    Reflexive decals;
    uint32_t unk17[0x9];
} __attribute__((packed)) BSP_MESH;

typedef struct
{
    short LightmapIndex;
    short unk1;
    uint32_t unknown[4];
    Reflexive material;
} __attribute__((packed)) BSP_SUBMESH;

typedef struct
{
    TagDependency ShaderTag;
    uint32_t UnkZero2;
    uint32_t VertIndexOffset;
    uint32_t VertIndexCount;
    float Centroid[3];
    float AmbientColor[3];
    uint32_t DistLightCount;
    float DistLight1[6];
    float DistLight2[6];
    float unkFloat2[3];
    float ReflectTint[4];
    float ShadowVector[3];
    float ShadowColor[3];
    float Plane[4];
    uint32_t UnkFlag2;
    uint32_t UnkCount1;
    uint32_t VertexCount1;
    uint32_t UnkZero4;
    uint32_t VertexOffset;
    uint32_t Vert_Reflexive;
    uint32_t UnkAlways3;
    uint32_t VertexCount2;
    uint32_t UnkZero9;
    uint32_t UnkLightmapOffset;
    uint32_t CompVert_Reflexive;
    uint32_t UnkZero5[2];
    uint32_t SomeOffset1;
    uint32_t PcVertexDataOffset;
    uint32_t UnkZero6;
    uint32_t CompVertBufferSize;
    uint32_t UnkZero7;
    uint32_t SomeOffset2;
    uint32_t VertexDataOffset;
    uint32_t UnkZero8;
} __attribute__((packed)) MATERIAL_SUBMESH_HEADER;

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
    
    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);
    ready = false;
}


// Shaders
class shader {
public:
    void start();
    void stop();
};

class soso : shader {
    
};

// Camera (converted from Archon)
class vector3d {
public:
    float x, y, z;
    vector3d (float sx, float sy, float sz) {
        x = sx;
        y = sy;
        y = sz;
    }
    vector3d* add(vector3d* vector) {
        x += vector->x;
        y += vector->y;
        z += vector->z;
        return this;
    };
};

class camera {
public:
    vector3d *position, *view, *up, *strafe;
    camera () {
        position = new vector3d(0.0, 0.0, 0.0);
        view     = new vector3d(0.0, 0.0, -0.5);
        up       = new vector3d(0.0, 0.0, 1.0);
    }
};

void *ERenderer::map2mem(uint32_t pointer) {
    return map + pointer - map->meta_address;
}

void ERenderer::renderBSP(Reflexive bsp) {
    int i;
    for (i=0; i < bsp.count; i++) {
        BSP_CHUNK *chunk = (BSP_CHUNK *)((long)map2mem(bsp.pointer) + sizeof(BSP_CHUNK) * i);
        BSP_MESH *mesh = (BSP_MESH*)(map2mem(chunk->start));
        
        int m;
        for (m=0; m < mesh->submeshHeader.count; m++) {
            BSP_SUBMESH *submesh = (BSP_SUBMESH *)((long)map2mem(mesh->submeshHeader.pointer) + sizeof(BSP_SUBMESH) * m);
            
            int n;
            for (n=0; n < submesh->material.count; n++) {
                MATERIAL_SUBMESH_HEADER *material = (MATERIAL_SUBMESH_HEADER *)((long)map2mem(submesh->material.pointer) + sizeof(MATERIAL_SUBMESH_HEADER) * n);
                
            }
            glBegin(GL_TRIANGLE_STRIP);
            
            glEnd();
        }
    }
}

// Load a map
void ERenderer::setMap(ProtonMap *map) {
    this->map = map;
    
    uint16_t scenarioTag = map->principal_tag;
    if (scenarioTag != NULLED_TAG_ID) {
        const char *scenarioData = map->tags.at(map->principal_tag).get()->Data();
        HaloScenarioTag *scenario = (HaloScenarioTag *)scenarioData;
        
        // Load the BSP
        renderBSP(scenario->bsp);
        
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
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (width / height), 0.1f, 4000000.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Main rendering loop
void ERenderer::render() {
    if (!ready) {
        return;
    }
    printf("render\n");
    
    // Setup the current viewport
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.2,0.2,0.2,1.0);
    
    // Render the scenario
    uint16_t scenarioTag = map->principal_tag;
    if (scenarioTag != NULLED_TAG_ID) {
        const char *scenarioData = map->tags.at(map->principal_tag).get()->Data();
        HaloScenarioTag *scenario = (HaloScenarioTag *)scenarioData;
        
        // Render BSP
        renderBSP(scenario->bsp);

        // Render Objects
        int i;
        for (i=0; i < scenario->scen.count; i++) {
            
        }
        
    }
}