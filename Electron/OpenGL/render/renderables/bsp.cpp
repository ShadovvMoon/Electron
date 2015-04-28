//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "bsp.h"
#include "../shader/shaders/senv.h"

uint8_t* map2mem(ProtonTag *scenario, uint32_t address) {
    return (uint8_t*)(scenario->Data() + scenario->PointerToOffset(address));
}

typedef struct {
    int16_t plane;
    int16_t back;
    int16_t front;
} BSP3DNode;
typedef struct {
    float a;
    float b;
    float c;
    float d;
} Plane;
typedef struct {
    bitmask8 flags;
    uint8_t bsp2dCount;
    uint32_t firstRef;
} Leaf;
typedef struct {
    uint32_t plane;
    uint32_t bsp2DNode;
} BSP2DRef;
typedef struct {
    float a;
    float b;
    float c;
    uint32_t left;
    uint32_t right;
} BSP2DNode;
typedef struct {
    uint32_t plane;
    uint32_t edge;
    bitmask16 flags;
    uint8_t breakableSurface;
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


// Experiment intersection function

BSP3DNode *node3d(ProtonTag *bspTag, CollisionBSP *collison, int i) {
    return (BSP3DNode*)(map2mem(bspTag, collison->BSP3DNodes.address) + i * sizeof(BSP3DNode));
}
Plane *plane(ProtonTag *bspTag, CollisionBSP *collison, int i) {
    return (Plane*)(map2mem(bspTag, collison->Planes.address) + i * sizeof(Plane));
}
Leaf *leaf(ProtonTag *bspTag, CollisionBSP *collison, int i) {
    return (Leaf*)(map2mem(bspTag, collison->Leaves.address) + i * sizeof(Leaf));
}
BSP2DRef *ref2d(ProtonTag *bspTag, CollisionBSP *collison, int i) {
    return (BSP2DRef*)(map2mem(bspTag, collison->BSP2DRef.address) + i * sizeof(BSP2DRef));
}
BSP2DNode *node2d(ProtonTag *bspTag, CollisionBSP *collison, int i) {
    return (BSP2DNode*)(map2mem(bspTag, collison->BSP2DNodes.address) + i * sizeof(BSP2DNode));
}
Surface *surface(ProtonTag *bspTag, CollisionBSP *collison, int i) {
    return (Surface*)(map2mem(bspTag, collison->Surfaces.address) + i * sizeof(Surface));
}
Edge *edge(ProtonTag *bspTag, CollisionBSP *collison, int i) {
    return (Edge*)(map2mem(bspTag, collison->Edges.address) + i * sizeof(Edge));
}
CollVert *vert(ProtonTag *bspTag, CollisionBSP *collison, int i) {
    return (CollVert*)(map2mem(bspTag, collison->Vertices.address) + i * sizeof(CollVert));
}

#include <math.h>
float dot3n(Plane *p, vector3d *q) {
    return p->a * q->x + p->b * q->y + p->c * q->z;
}
float dot2no(BSP2DNode *p, vector3d *q) {
    return p->a * q->x + p->b * q->y;
}
vector3d *project(vector3d *N, vector3d *T) {
    // First find the component of the plane normal that is the most significant
    float x = fabs(N->x);
    float y = fabs(N->y);
    float z = fabs(N->z);
    int projectionAxis = 0;
    int sign = 0; // 0 means the projection axis was negative, 1 means positive
    float letter = 0;
    if (z < y || z < x){
        if (y < x) { // X axis has the greatest contribution
            projectionAxis = 0;
            letter = N->x;
        } else { // Y axis has the greatest contribution
            projectionAxis = 1;
            letter = N->y;
        }
    }
    else { // otherwise Z had the greatest contribution
        projectionAxis = 2;
        letter = N->z;
    }
    if(letter > 0.f) {
        sign = 1;
    }
    
    // Choose the projection plane
    static short planeIndex[3][3][3] = {
        // Negative
        {{2, 1},  //Z,Y
         {0, 2}, //X,Z
         {1, 0}}, //Y,X
        // Positive
        {{1, 2},  // Y, Z
         {2, 0}, // Z, X
         {0, 1}}};// X, Y
    int xyz = planeIndex[sign][projectionAxis][0];
    int xyz2 = planeIndex[sign][projectionAxis][1];
    return new vector3d(T->get(xyz), T->get(xyz2), 0);
}

bool surfaceTest(ProtonTag *b, CollisionBSP *c, vector3d *T, uint32_t s) {
    CollVert *P, *Q;
    float angleSum = 0;
    
    int currentEdge;
    Surface *surf = surface(b,c,s);
    int nextEdge = surf->edge;
    do {
        currentEdge = nextEdge;
        Edge *curr = edge(b,c,currentEdge);
        if (s == curr->leftSurface) {
            P = vert(b,c,curr->startVert);
            Q = vert(b,c,curr->endVert);
            nextEdge = curr->forwardEdge;
        }
        else {
            P = vert(b,c,curr->endVert);
            Q = vert(b,c,curr->startVert);
            nextEdge = curr->prevEdge;
        }

        vector3d *v1 = new vector3d(Q->x - P->x, Q->y - P->y, Q->z - P->z); //deleted
        vector3d *v2 = new vector3d(T->x - P->x, T->y - P->y, T->z - P->z); //deleted
        vector3d *v3 = new vector3d(P->x - Q->x, P->y - Q->y, P->z - Q->z); //deleted
        vector3d *v4 = new vector3d(T->x - Q->x, T->y - Q->y, T->z - Q->z); //deleted
        angleSum += acos(v1->dot(v2)/(v1->mag() * v2->mag()));
        angleSum += acos(v3->dot(v4)/(v3->mag() * v4->mag()));
        delete v1;
        delete v2;
        delete v3;
        delete v4;
    }
    while(nextEdge != surf->edge);
    if(fabs(angleSum - 2*M_PI) < 0.01) {
        return true;
    }
    return false;
}
bool bsp2dTest(ProtonTag *b, CollisionBSP *c, uint32_t node, vector3d *T, vector3d *Tp) {
    if(node & 0x80000000) {
        return surfaceTest(b,c,T,node & 0x7FFFFFFF);
    }
    BSP2DNode *n = node2d(b,c,node);
    float s = dot2no(n, Tp) - n->c;
    if( s >= 0)
        return bsp2dTest(b, c, n->right, T, Tp);
    return bsp2dTest(b, c, n->left, T, Tp);
}

vector3d *leafTest(ProtonTag *b, CollisionBSP *c, vector3d *p, vector3d *q, uint16_t l) {
    Leaf *lea = leaf(b,c,l);
    uint32_t reference = lea->firstRef;
    
    #define kBigFloat 100000.0
    vector3d *S = new vector3d(kBigFloat,kBigFloat,kBigFloat);
    vector3d *T = nullptr;
    for (int i = 0; i < lea->bsp2dCount; i++) {
        BSP2DRef *ref = ref2d(b, c, reference+i);
        int pl = (ref->plane & 0x7FFFFFFF);
        
        Plane *N = plane(b,c,pl);
        float Na,Nb,Nc;
        if(ref->plane & 0x80000000){
            Na = -N->a; Nb = -N->b; Nc = -N->c;
        } else {
            Na = N->a; Nb = N->b; Nc = N->c;
        }
        
        // check if the line segment crosses the 2d plane upon which the 2dBsp was projected
        vector3d *nN = new vector3d(Na,Nb,Nc);
        float s = nN->dot(p) - N->d;
        float t = nN->dot(q) - N->d;
        if(s*t >= 0) { // if the line does not cross the plane, don't consider it
            delete nN;
            continue;
        }
        
        // Find the intersection between the planes
        vector3d *vec = new vector3d(0.0,0.0,0.0);
        vec->set(q);
        vec->sub(p);
        float ins = (N->d - nN->dot(p))/(nN->dot(vec));
        T = new vector3d(p);
        T->add(vec->mul(ins));
        
        // Test the 2d bsp
        vector3d *Tp = project(nN, T);
        float distanceS = sqrtf(powf(S->x-p->x, 2) + powf(S->y-p->y, 2) + powf(S->z-p->z, 2));
        float distanceT = sqrtf(powf(T->x-p->x, 2) + powf(T->y-p->y, 2) + powf(T->z-p->z, 2));
        if (distanceT < distanceS && bsp2dTest(b,c,ref->bsp2DNode,T,Tp)) {
            S->set(T);
        }
        delete nN;
    }
    return S;
}

vector3d *traverseTree(vector3d *p, vector3d *q, ProtonTag *b, CollisionBSP *c, int32_t node) {
    if(node == 0xFFFFFFFF) {
        return nullptr; // outside of the bsp tree
    }
    if (node & 0x80000000) { // we've hit a leaf, perform hit test
        return leafTest(b, c, p, q, (node & 0x7FFFFFFF));
    }
    
    BSP3DNode *node3 = node3d(b,c,node);
    Plane *N = plane(b, c, node3->plane & 0x7FFFFFFF);
    float Na,Nb,Nc;
    if(node3->plane & 0x80000000){
        Na = -N->a; Nb = -N->b; Nc = -N->c;
    } else {
        Na = N->a; Nb = N->b; Nc = N->c;
    }
    
    float s = Na*p->x + Nb*p->y + Nc*p->z - N->d; // if this is < 0 the point is behind the plane, > 0 in front
    float t = Na*q->x + Nb*q->y + Nc*q->z - N->d; // same as above, but for Q
    if(s > 0 && t > 0) { // both segments are infront of the plane
        if (node3->front == node) return nullptr;
        return traverseTree(p, q, b, c, node3->front);
    } else if(s < 0 && t < 0) { // both segments are behind the plane
        if (node3->back == node) return nullptr;
        return traverseTree(p, q, b, c, node3->back);
    } // else the vector intersects the plane

    // now what...
    printf("plane intersection\n");
    return nullptr;
}

vector3d *BSP::intersect(vector3d *p, vector3d *q, ProtonMap *map, ProtonTag *scenario) {
    HaloTagReflexive bsp = ((HaloScenarioTag*)scenario->Data())->bsp;
    int i, m;
    for (i=0; i < bsp.count; i++) {
        BSP_CHUNK *chunk = (BSP_CHUNK *)(map2mem(scenario, bsp.address) + sizeof(BSP_CHUNK) * i); // VERIFIED
        ProtonTag *bspTag = map->tags.at((uint16_t)(chunk->tagId)).get();
        uint32_t mesh_offset = *(uint32_t *)(bspTag->Data());
        BSP_MESH *mesh = (BSP_MESH *)map2mem(bspTag, mesh_offset);
        HaloTagReflexive collisionBSP = mesh->collBSP;
        for (m=0; m < bsp.count; m++) {
            CollisionBSP *collison = (CollisionBSP *)(map2mem(bspTag, collisionBSP.address) + 96 * m);
            return traverseTree(p, q, bspTag, collison, 0);
        }
    }
    return nullptr;
}























void BSPRenderBuffer::setup() {
    // Create the buffers for the vertices atttributes
#ifdef RENDER_VAO_NORMAL
	glGenVertexArrays(1, &geometryVAO);
	glBindVertexArray(geometryVAO);
#else
    glGenVertexArraysAPPLE(1, &geometryVAO);
    glBindVertexArrayAPPLE(geometryVAO);
#endif

    // Create the buffers for the vertices atttributes
    glGenBuffers(7, m_Buffers);
}

BSP::BSP(ShaderManager* manager) {
    printf("bsp setup\n");
    shaders = manager;
}

//TODO: Group elements into renderables
void BSP::setup(ProtonMap *map, ProtonTag *scenario) {
    
    vao = new BSPRenderBuffer;
    
    // Count data size
    int index_size  = 0;
    int vertex_size = 0;
    
    HaloTagReflexive bsp = ((HaloScenarioTag*)scenario->Data())->bsp;
    renderables.resize(bsp.count);
    
    int i, m, n;
    for (i=0; i < bsp.count; i++) {
        BSP_CHUNK *chunk = (BSP_CHUNK *)(map2mem(scenario, bsp.address) + sizeof(BSP_CHUNK) * i); // VERIFIED
        ProtonTag *bspTag = map->tags.at((uint16_t)(chunk->tagId)).get();
        uint32_t mesh_offset = *(uint32_t *)(bspTag->Data());
        BSP_MESH *mesh = (BSP_MESH *)map2mem(bspTag, mesh_offset);
        
        int renderable_count = 0;
        for (m=0; m < mesh->submeshHeader.count; m++) {
            BSP_SUBMESH *submesh = (BSP_SUBMESH *)(map2mem(bspTag, mesh->submeshHeader.address) + sizeof(BSP_SUBMESH) * m);
            for (n=0; n < submesh->material.count; n++) {
                MATERIAL_SUBMESH_HEADER *material = (MATERIAL_SUBMESH_HEADER *)(map2mem(bspTag, submesh->material.address) + sizeof(MATERIAL_SUBMESH_HEADER) * n);
                vertex_size += material->VertexCount1;
                index_size += material->VertIndexCount * 3;
                renderable_count++;
            }
        }
        renderables[i] = new BSPRenderMesh;
        renderables[i]->submeshes.resize(renderable_count);
        
        printf("creating lightmap 0x%x\n", mesh->lightmaps.tag_id.tag_index);
        renderables[i]->lightTexture = shaders->texture_manager()->create_texture(map, mesh->lightmaps);
    }

    vao->setup();
    vao->vertex_array    = (GLfloat*)malloc(vertex_size   * 3 * sizeof(GLfloat));
    vao->texture_uv      = (GLfloat*)malloc(vertex_size   * 2 * sizeof(GLfloat));
    vao->light_uv        = (GLfloat*)malloc(vertex_size   * 2 * sizeof(GLfloat));
    vao->normals         = (GLfloat*)malloc(vertex_size   * 3 * sizeof(GLfloat));
    vao->tangents        = (GLfloat*)malloc(vertex_size   * 3 * sizeof(GLfloat));
    vao->binormals       = (GLfloat*)malloc(vertex_size   * 3 * sizeof(GLfloat));
    vao->index_array     =   (GLint*)malloc(index_size    *     sizeof(GLint));

    int vertex_offset = 0;
    int index_offset = 0;
    
    int vert, uv;
    for (i=0; i < bsp.count; i++) {
        BSP_CHUNK *chunk = (BSP_CHUNK *)(map2mem(scenario, bsp.address) + sizeof(BSP_CHUNK) * i); // VERIFIED
        ProtonTag *bspTag = map->tags.at((uint16_t)(chunk->tagId)).get();
        uint32_t mesh_offset = *(uint32_t *)(bspTag->Data());
        BSP_MESH *mesh = (BSP_MESH *)map2mem(bspTag, mesh_offset);
        
        int render_pos = 0;
        for (m=0; m < mesh->submeshHeader.count; m++) {
            BSP_SUBMESH *submesh = (BSP_SUBMESH *)(map2mem(bspTag, mesh->submeshHeader.address) + sizeof(BSP_SUBMESH) * m);
            for (n=0; n < submesh->material.count; n++) {
                MATERIAL_SUBMESH_HEADER *material = (MATERIAL_SUBMESH_HEADER *)(map2mem(bspTag, submesh->material.address) + sizeof(MATERIAL_SUBMESH_HEADER) * n);
                uint8_t *vertIndexOffset = (uint8_t *)((sizeof(TRI_INDICES) * material->VertIndexOffset) + map2mem(bspTag, mesh->submeshIndices.address));
                uint8_t *PcVertexDataOffset = map2mem(bspTag, material->PcVertexDataOffset);
                int vertex_number = material->VertexCount1;
                int indexSize = material->VertIndexCount*3;
                
                HaloTagDependency shader = material->ShaderTag;
                printf("shader setup %d %d\n", n, shader.tag_id.tag_index);
                shader_object *material_shader = shaders->create_shader(map, shader);
                
                printf("renderer setup %d\n", n);
                BSPRenderSubmesh *renderer = new BSPRenderSubmesh;
                renderer->shader = material_shader;
                if (renderer->shader != nullptr && renderer->shader->is(shader_SENV)) {
                    ((senv_object*)renderer->shader)->useLight = true;
                }
                renderer->indexOffset = index_offset;
                renderer->vertexOffset = vertex_offset;
                renderer->indexCount = indexSize;
                renderer->vertCount = vertex_number;
                renderer->lightmap = submesh->LightmapIndex;
            
                int v;
                for (v = 0; v < vertex_number; v++)
                {
                    UNCOMPRESSED_BSP_VERT       *vert1 = (UNCOMPRESSED_BSP_VERT*)(PcVertexDataOffset + v * sizeof(UNCOMPRESSED_BSP_VERT));
                    UNCOMPRESSED_LIGHTMAP_VERT  *vert2 = (UNCOMPRESSED_LIGHTMAP_VERT*)(PcVertexDataOffset + vertex_number * sizeof(UNCOMPRESSED_BSP_VERT) + v * sizeof(UNCOMPRESSED_LIGHTMAP_VERT));
                    vert = vertex_offset * 3;
                    uv = vertex_offset * 2;
                    
                    //TODO: Just use the offset...
                    vao->vertex_array[vert]   = vert1->vertex_k[0];
                    vao->vertex_array[vert+1] = vert1->vertex_k[1];
                    vao->vertex_array[vert+2] = vert1->vertex_k[2];
                    vao->binormals[vert]      = vert1->binormal[0];
                    vao->binormals[vert+1]    = vert1->binormal[1];
                    vao->binormals[vert+2]    = vert1->binormal[2];
                    vao->tangents[vert]       = vert1->tangent[0];
                    vao->tangents[vert+1]     = vert1->tangent[1];
                    vao->tangents[vert+2]     = vert1->tangent[2];
                    vao->normals[vert]        = vert1->normal[0];
                    vao->normals[vert+1]      = vert1->normal[1];
                    vao->normals[vert+2]      = vert1->normal[2];
					vao->texture_uv[uv]       = vert1->uv[0];
                    vao->texture_uv[uv+1]     = vert1->uv[1];
					vao->light_uv[uv]		  = vert2->uv[0];
					vao->light_uv[uv+1]	      = vert2->uv[1];
                    vertex_offset++;
                }
                
                for (v = 0; v < indexSize; v+=3)
                {
                    TRI_INDICES *index = (TRI_INDICES*)(vertIndexOffset + sizeof(uint16_t) * v);
                    vao->index_array[index_offset]   = index->tri_ind[0];
                    vao->index_array[index_offset+1] = index->tri_ind[1];
                    vao->index_array[index_offset+2] = index->tri_ind[2];
                    index_offset += 3;
                }
                
                renderables[i]->submeshes[render_pos] = renderer;
                render_pos++;
            }
        }
    }
    
    // Assemble the VAO
    #define texCoord_buffer 1
    #define texCoord_buffer_light 3
    #define normals_buffer 2
    #define binormals_buffer 5
    #define tangents_buffer 6
    
    //Shift these to vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, vao->m_Buffers[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, vertex_size * 3 * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
    GLvoid* my_vertex_pointer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    memcpy(my_vertex_pointer, vao->vertex_array, vertex_size * 3 * sizeof(GLfloat));
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glEnableVertexAttribArray(0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    
    glBindBuffer(GL_ARRAY_BUFFER, vao->m_Buffers[TEXCOORD_VB]);
    glBufferData(GL_ARRAY_BUFFER, vertex_size * 2 * sizeof(GLfloat), vao->texture_uv, GL_STATIC_DRAW);
    glEnableVertexAttribArray(texCoord_buffer);
    glVertexAttribPointer(texCoord_buffer, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vao->m_Buffers[NORMAL_VB]);
    glBufferData(GL_ARRAY_BUFFER, vertex_size * 3 * sizeof(GLfloat), vao->normals, GL_STATIC_DRAW);
    glEnableVertexAttribArray(normals_buffer);
    glVertexAttribPointer(normals_buffer, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao->m_Buffers[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_size * sizeof(GLint), vao->index_array, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, vao->m_Buffers[LIGHT_VB]);
    glBufferData(GL_ARRAY_BUFFER, vertex_size * 2 * sizeof(GLfloat), vao->light_uv, GL_STATIC_DRAW);
    glEnableVertexAttribArray(texCoord_buffer_light);
    glVertexAttribPointer(texCoord_buffer_light, 2, GL_FLOAT, GL_FALSE, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, vao->m_Buffers[BINORMAL_VB]);
    glBufferData(GL_ARRAY_BUFFER, vertex_size * 3 * sizeof(GLfloat), vao->binormals, GL_STATIC_DRAW);
    glEnableVertexAttribArray(binormals_buffer);
    glVertexAttribPointer(binormals_buffer, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, vao->m_Buffers[TANGENT_VB]);
    glBufferData(GL_ARRAY_BUFFER, vertex_size * 3 * sizeof(GLfloat), vao->tangents, GL_STATIC_DRAW);
    glEnableVertexAttribArray(tangents_buffer);
    glVertexAttribPointer(tangents_buffer, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
#ifdef RENDER_VAO_NORMAL
	glBindVertexArray(0);
#else
	glBindVertexArrayAPPLE(0);
#endif

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    // Clean up
#ifdef RENDER_VBO
    free(vao->vertex_array);
    free(vao->texture_uv);
    free(vao->light_uv);
    free(vao->normals);
    free(vao->index_array);
    free(vao->binormals);
    free(vao->tangents);
#endif
}

void BSP::render(ShaderType pass) {
    
    //glEnable(GL_TEXTURE_2D);
    //glEnableClientState(GL_VERTEX_ARRAY);
	//glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
#ifndef RENDER_VAO
    glEnableVertexAttribArray(normals_buffer);
    glEnableVertexAttribArray(texCoord_buffer);
    glEnableVertexAttribArray(texCoord_buffer_light);
    glEnableVertexAttribArray(binormals_buffer);
    glEnableVertexAttribArray(tangents_buffer);
#endif
    
#ifdef RENDER_VAO
	#ifdef RENDER_VAO_NORMAL
		glBindVertexArray(vao->geometryVAO);
	#else
		glBindVertexArrayAPPLE(vao->geometryVAO);
	#endif
#elif defined(RENDER_VBO)
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vao->m_Buffers[POS_VB]);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vao->m_Buffers[TEXCOORD_VB]);
    glVertexAttribPointer(texCoord_buffer, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vao->m_Buffers[LIGHT_VB]);
    glVertexAttribPointer(texCoord_buffer_light, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vao->m_Buffers[NORMAL_VB]);
    glVertexAttribPointer(normals_buffer, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vao->m_Buffers[BINORMAL_VB]);
    glVertexAttribPointer(binormals_buffer, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vao->m_Buffers[TANGENT_VB]);
    glVertexAttribPointer(tangents_buffer, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao->m_Buffers[INDEX_BUFFER]);
#else
    glVertexPointer(3, GL_FLOAT, 0, vao->vertex_array);
    glVertexAttribPointer(texCoord_buffer, 2, GL_FLOAT, GL_FALSE, 0, vao->texture_uv);
    glVertexAttribPointer(texCoord_buffer_light, 2, GL_FLOAT, GL_FALSE, 0, vao->light_uv);
    glVertexAttribPointer(normals_buffer, 3, GL_FLOAT, GL_FALSE, 0, vao->normals);
    glVertexAttribPointer(binormals_buffer, 3, GL_FLOAT, GL_FALSE, 0, vao->binormals);
    glVertexAttribPointer(tangents_buffer, 3, GL_FLOAT, GL_FALSE, 0, vao->tangents);
#endif
    
    int i, s;
    shader_object *previous_shader = nullptr;
    for (i=0; i < renderables.size(); i++) {
        BSPRenderMesh *mesh = renderables[i];
        for (s=0; s < mesh->submeshes.size(); s++) {
            BSPRenderSubmesh *submesh = mesh->submeshes[s];
            if ((submesh->shader == nullptr && pass == shader_NULL) ||
                (submesh->shader != nullptr && submesh->shader->is(pass))) {
                if (submesh->shader != nullptr && submesh->shader != previous_shader) {
                    submesh->shader->setBaseUV(1.0, 1.0);
                    submesh->shader->render();
                    previous_shader = submesh->shader;
                }
                glActiveTexture(GL_TEXTURE3);
                mesh->lightTexture->bind(submesh->lightmap);
                
                #ifdef RENDER_VBO
                glDrawElementsBaseVertex(GL_TRIANGLES,
                                         submesh->indexCount,
                                         GL_UNSIGNED_INT,
                                         (void*)(submesh->indexOffset * sizeof(GLuint)),
                                         (submesh->vertexOffset));
                #else
                glDrawElementsBaseVertex(GL_TRIANGLES,
                                         submesh->indexCount,
                                         GL_UNSIGNED_INT,
                                         (void*)((long)vao->index_array + submesh->indexOffset * sizeof(GLuint)),
                                         (submesh->vertexOffset));
                #endif
            }
        }
        break;
    }
#ifdef RENDER_VAO_NORMAL
	glBindVertexArray(0);
#else
	glBindVertexArrayAPPLE(0);
#endif
    
#ifndef RENDER_VAO
    glDisableVertexAttribArray(texCoord_buffer_light);
    glDisableVertexAttribArray(texCoord_buffer);
    glDisableVertexAttribArray(normals_buffer);
    glDisableVertexAttribArray(binormals_buffer);
    glDisableVertexAttribArray(tangents_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
}