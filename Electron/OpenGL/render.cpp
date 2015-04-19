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

uint8_t* map2mem(ProtonTag *scenario, uint32_t address) {
    return (uint8_t*)(scenario->Data() + scenario->PointerToOffset(address));
}

void ERenderer::renderBSP(ProtonTag *scenario) {
    HaloTagReflexive bsp = ((HaloScenarioTag*)scenario->Data())->bsp;
    int i;
    for (i=0; i < bsp.count; i++) {
        printf("loading bsp %d\n", i);
               
        BSP_CHUNK *chunk = (BSP_CHUNK *)(map2mem(scenario, bsp.address) + sizeof(BSP_CHUNK) * i); // VERIFIED
        ProtonTag *bspTag = map->tags.at((uint16_t)(chunk->tagId)).get();
        BSP_MESH *mesh = (BSP_MESH *)bspTag->Data();
        printf("mesh map %c%c%c%c\n", mesh->lightmaps.tag_class[0],mesh->lightmaps.tag_class[1],mesh->lightmaps.tag_class[2],mesh->lightmaps.tag_class[3]);
        
        int m;
        for (m=0; m < mesh->submeshHeader.count; m++) {
            printf("loading mesh %d\n", m);
            
            BSP_SUBMESH *submesh = (BSP_SUBMESH *)(map2mem(bspTag, mesh->submeshHeader.address) + sizeof(BSP_SUBMESH) * m);
            
            int n;
            for (n=0; n < submesh->material.count; n++) {
                printf("loading material %d\n", n);
                MATERIAL_SUBMESH_HEADER *material = (MATERIAL_SUBMESH_HEADER *)(map2mem(bspTag, submesh->material.address) + sizeof(MATERIAL_SUBMESH_HEADER) * n);
                
                uint16_t vertCount = material->VertexCount1;
                /*uint64_t vertIndexOffset = ((sizeof(TRI_INDICES) * material->VertIndexOffset) + map2mem(bspTag, mesh->submeshIndices.address));
                
                glBegin(GL_TRIANGLE_STRIP);
                
                int v;
                for (v=0; v < material->VertIndexCount; v++) {
                    TRI_INDICES *index = (TRI_INDICES*)(material->VertIndexOffset + sizeof(TRI_INDICES) * v);
                    index->tri_ind[0];
                    index->tri_ind[1];
                    index->tri_ind[2];
                    
                    
                    
                }
         
                glEnd();
                */
            }
            
        }
    }
}

// Load a map
void ERenderer::setMap(ProtonMap *map) {
    this->map = map;
    
    uint16_t scenarioTag = map->principal_tag;
    if (scenarioTag != NULLED_TAG_ID) {
        
        ProtonTag *scenarioTag = map->tags.at(map->principal_tag).get();
        HaloScenarioTag *scenario = (HaloScenarioTag *)(scenarioTag->Data());
        
        // Load the BSP
        renderBSP(scenarioTag);
        
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

        // Render Objects
        int i;
        for (i=0; i < scenario->scen.count; i++) {
            
        }
        
    }
}