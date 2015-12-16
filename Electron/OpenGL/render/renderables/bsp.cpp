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
#define EPSILON 0.01
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
    int edgeCount = 0;
    do {
        currentEdge = nextEdge;
        Edge *curr = edge(b,c,currentEdge);
        if (s == curr->leftSurface) {
            P = vert(b,c,curr->startVert);
            Q = vert(b,c,curr->endVert);
            nextEdge = curr->forwardEdge;
        } else {
            P = vert(b,c,curr->endVert);
            Q = vert(b,c,curr->startVert);
            nextEdge = curr->prevEdge;
        }
        float v1x = Q->x - P->x;
        float v1y = Q->y - P->y;
        float v1z = Q->z - P->z;
        float v2x = T->x - P->x;
        float v2y = T->y - P->y;
        float v2z = T->z - P->z;
        float v3x = P->x - Q->x;
        float v3y = P->y - Q->y;
        float v3z = P->z - Q->z;
        float v4x = T->x - Q->x;
        float v4y = T->y - Q->y;
        float v4z = T->z - Q->z;
        angleSum += acos((v1x*v2x+v1y*v2y+v1z*v2z)/sqrtf((v1x*v1x+v1y*v1y+v1z*v1z)*(v2x*v2x+v2y*v2y+v2z*v2z)));
        angleSum += acos((v3x*v4x+v3y*v4y+v3z*v4z)/sqrtf((v3x*v3x+v3y*v3y+v3z*v3z)*(v4x*v4x+v4y*v4y+v4z*v4z)));
        edgeCount++;
    }
    while(nextEdge != surf->edge);
    float expected = (edgeCount - 2) * M_PI;
    if(fabs(angleSum - expected) < EPSILON) {
        return true;
    }
    return false;
}
bool bsp2dTest(ProtonTag *b, CollisionBSP *c, uint32_t node, vector3d *T, vector3d *Tp, intersection *output) {
    if(node & 0x80000000) {
        bool success = surfaceTest(b,c,T,node & 0x7FFFFFFF);
        if (output != NULL) {
            intersection *current = output;
            while (current->next != NULL) {
                current = current->next;
            }
            intersection *report = new intersection;
            report->position = NULL;
            report->surface = node & 0x7FFFFFFF;
            report->next = NULL;
            report->accurate = !success;
            current->next = report;
        }
        return success;
    }
    BSP2DNode *n = node2d(b,c,node);
    float s = dot2no(n, Tp) - n->c;
    if( s >= 0)
        return bsp2dTest(b, c, n->right, T, Tp, output);
    return bsp2dTest(b, c, n->left, T, Tp, output);
}

vector3d *leafTest(ProtonTag *b, CollisionBSP *c, vector3d *p, vector3d *q, uint16_t l, intersection *output) {
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
        if (bsp2dTest(b,c,ref->bsp2DNode,T,Tp, output)) {
            S->set(T);
        }
        delete nN;
    }
    return S;
}

vector3d *traverseTree(vector3d *p, vector3d *q, ProtonTag *b, CollisionBSP *c, int32_t node, intersection *output) {
    if(node == 0xFFFFFFFF) {
        return nullptr; // outside of the bsp tree
    }
    if (node & 0x80000000) { // we've hit a leaf, perform hit test
        return leafTest(b, c, p, q, (node & 0x7FFFFFFF), output);
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
        return traverseTree(p, q, b, c, node3->front, output);
    } else if(s < 0 && t < 0) { // both segments are behind the plane
        return traverseTree(p, q, b, c, node3->back, output);
    } // else the vector intersects the plane

    // Split the line segment
    vector3d *V = new vector3d(q);
    V->sub(p);
    float ins = (N->d - (p->x*N->a+p->y*N->b+p->z*N->c))/(V->x*N->a+V->y*N->b+V->z*N->c);
    
    // Now we have it that so that P+ins*V lies on the plane N
    // we have to make sure that the new line (P, Pn) can still intersect the splitting plane
    // so we fudge ins a little bit to make sure that it can still intersect with N (since the splitting plane may contain
    // the surface that we want to have an intersection test against in the leaf)
    vector3d *temp = new vector3d(0.0,0.0,0.0);
    vector3d *S = nullptr;
    vector3d *T = nullptr;
    if(s >= 0) {
        temp->x = p->x + (ins+EPSILON)* V->x;
        temp->y = p->y + (ins+EPSILON)* V->y;
        temp->z = p->z + (ins+EPSILON)* V->z;
        S = traverseTree(p, temp, b, c, node3->front, output);
        
        temp->x = p->x + (ins-EPSILON)* V->x;
        temp->y = p->y + (ins-EPSILON)* V->y;
        temp->z = p->z + (ins-EPSILON)* V->z;
        T = traverseTree(temp, q, b, c, node3->back, output);
    }
    else {
        temp->x = p->x + (ins+EPSILON)* V->x;
        temp->y = p->y + (ins+EPSILON)* V->y;
        temp->z = p->z + (ins+EPSILON)* V->z;
        S = traverseTree(p, temp, b, c, node3->back, output);
        temp->x = p->x + (ins-EPSILON)* V->x;
        temp->y = p->y + (ins-EPSILON)* V->y;
        temp->z = p->z + (ins-EPSILON)* V->z;
        T = traverseTree(temp, q, b, c, node3->front, output);
    }
    delete temp;
    if (!T && !S) {
        return nullptr;
    } else if (!S) {
        return T;
    } else if (!T) {
        return S;
    }
    
    float distanceS = sqrtf(powf(S->x-p->x, 2) + powf(S->y-p->y, 2) + powf(S->z-p->z, 2));
    float distanceT = sqrtf(powf(T->x-p->x, 2) + powf(T->y-p->y, 2) + powf(T->z-p->z, 2));
    if(distanceS < distanceT) {
        delete T;
        return S;
    }
    delete S;
    return T;
}

vector3d *BSP::intersect(vector3d *p, vector3d *q, ProtonMap *map, ProtonTag *scenario) {
    return intersect_report(p, q, map, scenario, NULL);
}

vector3d *BSP::intersect_report(vector3d *p, vector3d *q, ProtonMap *map, ProtonTag *scenario, intersection *output) {
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
            vector3d *traversal = traverseTree(p, q, bspTag, collison, 0, output);
            if (traversal != nullptr) {
                if (traversal->x == kBigFloat || traversal->y == kBigFloat || traversal->z == kBigFloat) return nullptr;
            }
            return traversal;
        }
    }
    return nullptr;
}

bool e_float(float f1, float f2) {
    return (f1 > f2 - 0.001 && f1 < f2 + 0.001);
}

void BSP::render_experimental(ShaderType pass) {
    if (pass != shader_NULL) return;
    
    // Draw our sun
    vector3d *sun = new vector3d(0, 0, 50);
    glPushMatrix();
    glTranslatef(sun->x, sun->y, sun->z);
    glColor3f(1.0, 1.0, 0.0);
    GLUquadric *sphere=gluNewQuadric();
    gluQuadricDrawStyle( sphere, GLU_FILL);
    gluQuadricNormals( sphere, GLU_SMOOTH);
    gluQuadricOrientation( sphere, GLU_OUTSIDE);
    gluQuadricTexture( sphere, GL_TRUE);
    gluSphere(sphere, 10.0,10,10);
    gluDeleteQuadric ( sphere );
    glPopMatrix();
    glColor4f(1.0, 1.0, 1.0, 1.0);
    
    // Draw our lights
    glPointSize(5.0);
    glBegin(GL_POINTS);
    for (int s = 0; s < light_experiment.size(); s++) {
        vector3d *position = light_experiment[s];
        glVertex3f(position->x, position->y, position->z);
        /*
        glPushMatrix();
        glTranslatef(position->x, position->y, position->z);
        glColor3f(1.0, 1.0, 0.0);
        GLUquadric *sphere=gluNewQuadric();
        gluQuadricDrawStyle( sphere, GLU_FILL);
        //gluQuadricNormals( sphere, GLU_SMOOTH);
        gluQuadricOrientation( sphere, GLU_OUTSIDE);
        gluQuadricTexture( sphere, GL_TRUE);
        gluSphere(sphere,0.05,3,3);
        gluDeleteQuadric ( sphere );
        glPopMatrix();
        */
    }
    glEnd();
}

void BSP::generate_lightmap(vector3d *sun, ProtonMap *map, ProtonTag *scenario) {
    vector3d *position = new vector3d(0,0,0);

    int cmax = 2000;
    int tmax = 2000;
    double cdelta = 360.0 / cmax;
    double tdelta = 360.0 / tmax;
    double distance = 1000.0;
    for (int c = 0; c < cmax; c++) {
        for (int t = 0; t < tmax; t++) {
            double cangle = cdelta * c;
            double tangle = tdelta * t;
            position->x = distance * cos(cangle) * sin(tangle);
            position->y = distance * sin(cangle) * sin(tangle);
            position->z = distance * cos(tangle);
            vector3d *intersection = intersect(sun, position, map, scenario);
            if (intersection != nullptr) {
                light_experiment.push_back(intersection);
            }
        }
    }
  
    /* //smartish
    int i, s, v;
    for (i=0; i < renderables.size(); i++) {
        BSPRenderMesh *mesh = renderables[i];
        for (s=0; s < mesh->submeshes.size(); s++) {
            BSPRenderSubmesh *submesh = mesh->submeshes[s];
            for (v = 0; v < submesh->vertCount; v++) {
                GLfloat *verts = &vao->vertex_array[submesh->vertexOffset + v];
                position->x = sun->x + (verts[0] - sun->x) * 5;
                position->y = sun->y + (verts[1] - sun->y) * 5;
                position->z = sun->z + (verts[2] - sun->z) * 5;
                vector3d *intersection = intersect(position, sun, map, scenario);
                if (intersection != nullptr) {
                    if ((e_float(verts[0], intersection->x) && e_float(verts[1], intersection->y) && e_float(verts[2], intersection->z))) {
                        // direct sunlight
                        printf("light at %f %f %f\n", verts[0], verts[1], verts[2]);
                        light_experiment.push_back(intersection);
                    }
                }
            }
        }
    }
    */
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
    //light_experiment = std::vector<vector3d*>();
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
                
                if (renderer->shader->is(shader_SWAT)) {
                    shaders->set_reflection_height(vao->vertex_array[(vertex_offset-1)*3 + 2]);
                }
                
                renderables[i]->submeshes[render_pos] = renderer;
                render_pos++;
            }
        }
    }
    
    // Assemble the VAO
    #define vertex_buffer 0
    #define texCoord_buffer 1
    #define texCoord_buffer_light 3
    #define normals_buffer 2
    #define binormals_buffer 5
    #define tangents_buffer 6
    
    //Shift these to vertex buffers
   
#ifdef RENDER_CORE_32
    glBindBuffer(GL_ARRAY_BUFFER, vao->m_Buffers[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, vertex_size * 3 * sizeof(GLfloat), vao->vertex_array, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(vertex_buffer);
    glVertexAttribPointer(vertex_buffer, 3, GL_FLOAT, GL_FALSE, 0, NULL);
#else
    //GLvoid* my_vertex_pointer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    //memcpy(my_vertex_pointer, vao->vertex_array, vertex_size * 3 * sizeof(GLfloat));
    //glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, vao->m_Buffers[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, vertex_size * 3 * sizeof(GLfloat), vao->vertex_array, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
#endif
    
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
    //free(vao->vertex_array);
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
                    if (!submesh->shader->render(pass)) {
                        continue;
                    }
                    previous_shader = submesh->shader;
                }
                
                #ifdef GL_VALIDATE
                GLint program = shaders->get_shader(pass)->program;
                glValidateProgram(program);
                
                int bufflen = 1024;
                int validate = 0;
                
                glGetProgramiv(program, GL_VALIDATE_STATUS, &validate);
                if(validate == GL_FALSE) {
                    printf("validation failed bsp\n");
                    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufflen);
                    if (bufflen > 1) {
                        GLchar* log_string = new char[bufflen + 1];
                        glGetProgramInfoLog(program, bufflen, 0, log_string);
                        printf("LOG: %s\n", log_string);
                    }
                }
                #endif
                
                glActiveTexture(GL_TEXTURE3);
                mesh->lightTexture->bind(submesh->lightmap);
                
                #ifdef RENDER_VBO
                glDrawRangeElementsBaseVertex(GL_TRIANGLES, submesh->vertexOffset, submesh->vertexOffset+submesh->vertCount, submesh->indexCount, GL_UNSIGNED_INT, (void*)(submesh->indexOffset * sizeof(GLuint)), submesh->vertexOffset);
                #else
                glDrawRangeElementsBaseVertex(GL_TRIANGLES, submesh->vertexOffset, submesh->vertexOffset+submesh->vertCount, submesh->indexCount, GL_UNSIGNED_INT, (void*)((long)vao->index_array + submesh->indexOffset * sizeof(GLuint)), submesh->vertexOffset);
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
    
    render_experimental(pass);
}