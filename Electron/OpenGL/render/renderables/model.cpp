//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "model.h"

typedef struct {
    char unk[36];
    HaloTagReflexive parts;
} HaloGeometry;

typedef struct {
    uint32_t flags;
    uint16_t shader;
    char unk0[66];
    uint32_t indexCount;
    uint32_t indexPointer1;
    uint32_t indexPointer2;
    uint32_t unk1;
    uint32_t vertCount;
    char unk2[8];
    uint32_t vertPointer1;
    uint32_t compressedVertCount;
    char unk3[8];
    uint32_t compressedVertPointer;
    char unk4[12];
} HaloGeometryPart;

typedef struct {
    char unk0[48];
    float uv[2];
    char unk1[140];
    HaloTagReflexive region;
    HaloTagReflexive geometry;
    HaloTagReflexive shaders;
} HaloModel;

typedef struct {
    HaloTagDependency shader;
    char unk0[16];
} HaloShader;

typedef struct MODEL_REGION_PERMUTATION
{
    char Name[32];
    uint32_t Flags[8];
    short LOD_MeshIndex[5];
    short Reserved[7];
} MODEL_REGION_PERMUTATION;

typedef struct MODEL_REGION
{
    char Name[64];
    HaloTagReflexive Permutations;
} MODEL_REGION;

typedef struct
{
    float vertex_k[3]; //0
    float normal[3];   //0xC
    
    float binormal[3]; //0x18
    float tangent[3];  //0x24
    
    float uv[2]; //0x30
    char unk0[12];
} MODEL_VERT;

void ModelRenderMesh::setup() {
    // Create the buffers for the vertices atttributes
#ifdef _WINDOWS
	glGenVertexArrays(1, &geometryVAO);
	glBindVertexArray(geometryVAO);
#else
	glGenVertexArraysAPPLE(1, &geometryVAO);
	glBindVertexArrayAPPLE(geometryVAO);
#endif

    // Create the buffers for the vertices atttributes
    glGenBuffers(7, m_Buffers);
}

Model::Model(ModelManager *manager, ProtonMap *map, HaloTagDependency tag) {
    if (tag.tag_id.tag_index == NULLED_TAG_ID) {
        return;
    }
    
    printf("reading model 0x%x\n", tag.tag_id.tag_index);
    ProtonTag *modelTag = map->tags.at(tag.tag_id.tag_index).get();
    HaloModel *modelData = (HaloModel *)modelTag->Data();
    float base_u = *(float*)(modelTag->Data() + 0x30);
    float base_v = *(float*)(modelTag->Data() + 0x34);
    int i,p;
    
    // Read shaders
    printf("reading shaders\n");
    shaders.resize(modelData->shaders.count);
    for (i=0; i < modelData->shaders.count; i++) {
        HaloShader *shaderRef = (HaloShader *)(modelTag->Data() + modelTag->PointerToOffset(modelData->shaders.address) + 32 * i);
        shaders[i] = (manager->shaders->create_shader(map, shaderRef->shader));
    }
    
    // Count geometry
    printf("counting geom\n");
    geometries.resize(modelData->geometry.count);
    for (i=0; i < modelData->geometry.count; i++) {
        HaloGeometry *geometry = (HaloGeometry *)(modelTag->Data() + modelTag->PointerToOffset(modelData->geometry.address) + 48 * i);
        int renderable_count = 0;
        for (p=0; p < geometry->parts.count; p++) {
            HaloGeometryPart *part = (HaloGeometryPart *)(modelTag->Data() + modelTag->PointerToOffset(geometry->parts.address) + 132 * p);
            if (part->indexPointer1 != part->indexPointer2) {
                continue;
            }
            renderable_count++;
        }
        geometries[i] = std::vector<ModelRenderMesh*>(renderable_count);
    }
    
    // Read geometry
    printf("reading geom\n");
    for (i=0; i < modelData->geometry.count; i++) {
        HaloGeometry *geometry = (HaloGeometry *)(modelTag->Data() + modelTag->PointerToOffset(modelData->geometry.address) + 48 * i);

        int render = 0;
        for (p=0; p < geometry->parts.count; p++) {
            HaloGeometryPart *part = (HaloGeometryPart *)(modelTag->Data() + modelTag->PointerToOffset(geometry->parts.address) + 132 * p);
            if (part->indexPointer1 != part->indexPointer2) {
                printf("bad part\n");
                continue;
            }
            
            uint8_t *vertIndexOffset    = (uint8_t *)(modelTag->ResourcesData() + part->indexPointer1);
            uint8_t *PcVertexDataOffset = (uint8_t *)(modelTag->ResourcesData() + part->vertPointer1);
            
            int vertex_number = part->vertCount;
            int indexSize     = part->indexCount + 2;
            
            ModelRenderMesh *renderer = new ModelRenderMesh;
            renderer->setup();
            renderer->base_u = base_u;
            renderer->base_v = base_v;
            renderer->shader = shaders[part->shader];
            renderer->indexCount = indexSize;
            
            // Assemble the VBO
            renderer->vertex_array    = (GLfloat*)malloc(vertex_number   * 3 * sizeof(GLfloat)); // cleaned
            renderer->texture_uv      = (GLfloat*)malloc(vertex_number   * 2 * sizeof(GLfloat)); // cleaned
            renderer->light_uv        = (GLfloat*)malloc(vertex_number   * 2 * sizeof(GLfloat)); // cleaned
            renderer->normals         = (GLfloat*)malloc(vertex_number   * 3 * sizeof(GLfloat)); // cleaned
            renderer->tangents        = (GLfloat*)malloc(vertex_number   * 3 * sizeof(GLfloat));
            renderer->binormals       = (GLfloat*)malloc(vertex_number   * 3 * sizeof(GLfloat));
            
            renderer->index_array     = (GLint*)  malloc(indexSize * sizeof(GLint)); // cleaned
            
            int v;
            int vert = 0, uv = 0;
            for (v = 0; v < vertex_number; v++)
            {
                MODEL_VERT *vert1 = (MODEL_VERT*)(PcVertexDataOffset + v * sizeof(MODEL_VERT));
                renderer->vertex_array[vert]   = vert1->vertex_k[0];
                renderer->vertex_array[vert+1] = vert1->vertex_k[1];
                renderer->vertex_array[vert+2] = vert1->vertex_k[2];
                renderer->normals[vert]        = vert1->normal[0];
                renderer->normals[vert+1]      = vert1->normal[1];
                renderer->normals[vert+2]      = vert1->normal[2];
                renderer->binormals[vert]      = vert1->binormal[0];
                renderer->binormals[vert+1]    = vert1->binormal[1];
                renderer->binormals[vert+2]    = vert1->binormal[2];
                renderer->tangents[vert]       = vert1->tangent[0];
                renderer->tangents[vert+1]     = vert1->tangent[1];
                renderer->tangents[vert+2]     = vert1->tangent[2];
                renderer->texture_uv[uv]       = vert1->uv[0];
                renderer->texture_uv[uv+1]     = vert1->uv[1];
                vert+=3; uv+=2;
            }
            
            for (v = 0; v < indexSize; v++)
            {
                uint16_t *index = (uint16_t*)(vertIndexOffset + sizeof(uint16_t) * v);
                renderer->index_array[v]   = index[0];
            }
            
            #define texCoord_buffer 1
            #define texCoord_buffer_light 3
            #define normals_buffer 2
            #define binormals_buffer 5
            #define tangents_buffer 6
            
            //Shift these to vertex buffers
            glBindBuffer(GL_ARRAY_BUFFER, renderer->m_Buffers[POS_VB]);
            glBufferData(GL_ARRAY_BUFFER, vertex_number * 3 * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
            GLvoid* my_vertex_pointer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
            memcpy(my_vertex_pointer, renderer->vertex_array, vertex_number * 3 * sizeof(GLfloat));
            glUnmapBuffer(GL_ARRAY_BUFFER);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
            
            glBindBuffer(GL_ARRAY_BUFFER, renderer->m_Buffers[TEXCOORD_VB]);
            glBufferData(GL_ARRAY_BUFFER, vertex_number * 2 * sizeof(GLfloat), renderer->texture_uv, GL_STATIC_DRAW);
            glEnableVertexAttribArray(texCoord_buffer);
            glVertexAttribPointer(texCoord_buffer, 2, GL_FLOAT, GL_FALSE, 0, 0);
            
            glBindBuffer(GL_ARRAY_BUFFER, renderer->m_Buffers[NORMAL_VB]);
            glBufferData(GL_ARRAY_BUFFER, vertex_number * 3 * sizeof(GLfloat), renderer->normals, GL_STATIC_DRAW);
            glEnableVertexAttribArray(normals_buffer);
            glVertexAttribPointer(normals_buffer, 3, GL_FLOAT, GL_FALSE, 0, 0);
            
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->m_Buffers[INDEX_BUFFER]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * sizeof(GLint), renderer->index_array, GL_STATIC_DRAW);
            
            glBindBuffer(GL_ARRAY_BUFFER, renderer->m_Buffers[LIGHT_VB]);
            glBufferData(GL_ARRAY_BUFFER, vertex_number * 2 * sizeof(GLfloat), renderer->light_uv, GL_STATIC_DRAW);
            glEnableVertexAttribArray(texCoord_buffer_light);
            glVertexAttribPointer(texCoord_buffer_light, 2, GL_FLOAT, GL_FALSE, 0, 0);
            
            glBindBuffer(GL_ARRAY_BUFFER, renderer->m_Buffers[BINORMAL_VB]);
            glBufferData(GL_ARRAY_BUFFER, vertex_number * 3 * sizeof(GLfloat), renderer->binormals, GL_STATIC_DRAW);
            glEnableVertexAttribArray(binormals_buffer);
            glVertexAttribPointer(binormals_buffer, 3, GL_FLOAT, GL_FALSE, 0, 0);
            
            glBindBuffer(GL_ARRAY_BUFFER, renderer->m_Buffers[TANGENT_VB]);
            glBufferData(GL_ARRAY_BUFFER, vertex_number * 3 * sizeof(GLfloat), renderer->tangents, GL_STATIC_DRAW);
            glEnableVertexAttribArray(tangents_buffer);
            glVertexAttribPointer(tangents_buffer, 3, GL_FLOAT, GL_FALSE, 0, 0);
            
		#ifdef _WINDOWS
			glBindVertexArray(0);
		#else
			glBindVertexArrayAPPLE(0);
		#endif

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            
            // Clean up
            #ifdef RENDER_VBO
            free(renderer->vertex_array);
            free(renderer->texture_uv);
            free(renderer->light_uv);
            free(renderer->normals);
            free(renderer->index_array);
            #endif
            
            printf("geom %d %d\n", i, render);
            geometries[i][render] = renderer;
            render++;
        }
    }
    printf("geometries %d\n", geometries.size());
    
    // Read regions
    int LOD = 4;
    printf("regions %d\n", modelData->region.count);
    for (i=0; i < modelData->region.count; i++) {
        MODEL_REGION *region = (MODEL_REGION *)(modelTag->Data() + modelTag->PointerToOffset(modelData->region.address) + 76 * i);
        printf("region %d %d\n", i, region->Permutations.count);
        for (p=0; p < region->Permutations.count; p++) {
            MODEL_REGION_PERMUTATION *permutation = (MODEL_REGION_PERMUTATION *)(modelTag->Data() + modelTag->PointerToOffset(region->Permutations.address) + 88 * p);
            printf("permutation %d\n", i, permutation->LOD_MeshIndex[LOD]);
            if ((permutation->Flags[0] & 0xFF) != 1) {
                renderIndices.push_back(permutation->LOD_MeshIndex[LOD]);
            }
        }
    }
    
    printf("creating model object\n");
    name = modelTag->Name();
    ready = true;
}

void Model::render(ShaderType pass) {
    if (!ready) return;
    
    //glEnable(GL_TEXTURE_2D);
    //glEnable(GL_TEXTURE_CUBE_MAP);
    glEnableClientState(GL_VERTEX_ARRAY);
    //glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
#ifndef RENDER_VAO
    glEnableVertexAttribArray(texCoord_buffer);
    glEnableVertexAttribArray(normals_buffer);
    glEnableVertexAttribArray(binormals_buffer);
    glEnableVertexAttribArray(tangents_buffer);
#endif
    
    shader_object *previous_shader = nullptr;
    
    int r, i;
    for (r=0; r < renderIndices.size(); r++) {
        uint8_t renderIndex = renderIndices[r];
        std::vector<ModelRenderMesh*>renderables = geometries[renderIndex];
        for (i=0; i < renderables.size(); i++) {
            ModelRenderMesh *mesh = renderables[i];
            if ((mesh->shader == nullptr && pass == shader_NULL) ||
                (mesh->shader != nullptr && mesh->shader->is(pass))) {
#ifdef RENDER_VAO
			#ifdef _WINDOWS
				glBindVertexArray(mesh->geometryVAO);
			#else
				glBindVertexArrayAPPLE(mesh->geometryVAO);
			#endif
#elseif RENDER_VBO
                glBindBufferARB(GL_ARRAY_BUFFER_ARB, mesh->m_Buffers[POS_VB]);
                glVertexPointer(3, GL_FLOAT, 0, 0);
                glBindBufferARB(GL_ARRAY_BUFFER_ARB, mesh->m_Buffers[TEXCOORD_VB]);
                glVertexAttribPointer(texCoord_buffer, 2, GL_FLOAT, GL_FALSE, 0, 0);
                glBindBufferARB(GL_ARRAY_BUFFER_ARB, mesh->m_Buffers[NORMAL_VB]);
                glVertexAttribPointer(normals_buffer, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glBindBufferARB(GL_ARRAY_BUFFER_ARB, mesh->m_Buffers[BINORMAL_VB]);
                glVertexAttribPointer(binormals_buffer, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glBindBufferARB(GL_ARRAY_BUFFER_ARB, mesh->m_Buffers[TANGENT_VB]);
                glVertexAttribPointer(tangents_buffer, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->m_Buffers[INDEX_BUFFER]);
#else
                glVertexPointer(3, GL_FLOAT, 0, mesh->vertex_array);
                glVertexAttribPointer(texCoord_buffer, 2, GL_FLOAT, GL_FALSE, 0, mesh->texture_uv);
                glVertexAttribPointer(normals_buffer, 3, GL_FLOAT, GL_FALSE, 0, mesh->normals);
                glVertexAttribPointer(binormals_buffer, 3, GL_FLOAT, GL_FALSE, 0, mesh->binormals);
                glVertexAttribPointer(tangents_buffer, 3, GL_FLOAT, GL_FALSE, 0, mesh->tangents);
#endif
                if (mesh->shader != nullptr) {
                    if (mesh->shader != previous_shader) {
                        mesh->shader->render();
                        previous_shader = mesh->shader;
                    }
                    mesh->shader->setBaseUV(mesh->base_u, mesh->base_v);
                }
                
            #ifdef RENDER_VBO
                glDrawElements(GL_TRIANGLE_STRIP, mesh->indexCount, GL_UNSIGNED_INT, 0);
            #else
                glDrawElements(GL_TRIANGLE_STRIP, mesh->indexCount, GL_UNSIGNED_INT, mesh->index_array);
            #endif
                
			#ifdef _WINDOWS
				glBindVertexArray(0);
			#else
				glBindVertexArrayAPPLE(0);
			#endif
            }
        }
    }
    
#ifndef RENDER_VAO
    glDisableVertexAttribArray(texCoord_buffer);
    glDisableVertexAttribArray(normals_buffer);
    glDisableVertexAttribArray(binormals_buffer);
    glDisableVertexAttribArray(tangents_buffer);
#endif
}

Model *ModelManager::create_model(ProtonMap *map, HaloTagDependency mod2) {

    // Has this model been loaded before? Check the cache
    std::map<uint16_t, Model*>::iterator iter = models.find(mod2.tag_id.tag_index);
    if (iter != models.end()) {
        return iter->second;
    }
    
    // Create a new texture
    Model *model = new Model(this, map, mod2);
    models[mod2.tag_id.tag_index] = model;
    return model;
}

ModelManager::ModelManager(ShaderManager* manager) {
    printf("model manager setup\n");
    shaders = manager;
}