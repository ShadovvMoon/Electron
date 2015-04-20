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

Geometry::Geometry(uint8_t *offset) {
    
}

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
    MODEL_REGION_PERMUTATION *modPermutations;
} MODEL_REGION;

class ModelRenderMesh {
public:
    GLuint m_Buffers[5];
    
    // Rendering
    GLuint geometryVAO;
    int indexCount;
    int vertCount;
    
    // Arrays
    GLfloat* vertex_array;
    GLfloat* texture_uv;
    GLfloat* light_uv;
    GLfloat* normals;
    GLint* index_array;
    
    shader_object *shader;
    void setup();
};
void ModelRenderMesh::setup() {
    // Create the buffers for the vertices atttributes
    glGenVertexArraysAPPLE(1, &geometryVAO);
    glBindVertexArrayAPPLE(geometryVAO);
    
    // Create the buffers for the vertices atttributes
    glGenBuffers(5, m_Buffers);
}

Model::Model(ProtonMap *map, HaloTagDependency tag) {
    ProtonTag *modelTag = map->tags.at(tag.tag_id.tag_index).get();
    HaloModel *modelData = (HaloModel *)modelTag->Data();
    
    int i;
    
    // Read regions
    
    // Read shaders
    for (i=0; i < modelData->shaders.count; i++) {
        HaloShader *shaderRef = (HaloShader *)(modelTag->Data() + modelTag->PointerToOffset(modelData->shaders.address) + sizeof(HaloShader) * i);
    }
    
    // Read geometry
    for (i=0; i < modelData->geometry.count; i++) {
        HaloGeometry *geometry = (HaloGeometry *)(modelTag->Data() + modelTag->PointerToOffset(modelData->geometry.address) + sizeof(HaloGeometry) * i);
        
        int p;
        for (p=0; p < geometry->parts.count; p++) {
            HaloGeometryPart *part = (HaloGeometryPart *)(modelTag->Data() + modelTag->PointerToOffset(geometry->parts.address) + sizeof(HaloGeometryPart) * p);
            if (part->indexPointer1 != part->indexPointer2) {
                printf("bad part\n");
                continue;
            }
            
            uint8_t *vertIndexOffset = (uint8_t *)(modelTag->ResourcesData() + part->indexPointer1);
            uint8_t *PcVertexDataOffset = (uint8_t *)(modelTag->ResourcesData() + part->vertPointer1);
            
            int vertex_number = part->vertCount;
            int indexSize = part->indexCount * 3;
            ModelRenderMesh *renderer = new ModelRenderMesh;
            renderer->setup();
            
            // Assemble the VBO
            renderer->vertex_array    = (GLfloat*)malloc(vertex_number   * 3 * sizeof(GLfloat));
            renderer->texture_uv      = (GLfloat*)malloc(vertex_number   * 2 * sizeof(GLfloat));
            renderer->light_uv        = (GLfloat*)malloc(vertex_number   * 2 * sizeof(GLfloat));
            renderer->normals         = (GLfloat*)malloc(vertex_number   * 3 * sizeof(GLfloat));
            renderer->index_array     = (GLint*)malloc(indexSize * sizeof(GLint));
            
            int v;
            int vert = 0, uv = 0;
            for (v = 0; v < vertex_number; v++)
            {
                UNCOMPRESSED_BSP_VERT *vert1 = (UNCOMPRESSED_BSP_VERT*)(PcVertexDataOffset + v * sizeof(UNCOMPRESSED_BSP_VERT));
                renderer->vertex_array[vert]   = vert1->vertex_k[0];
                renderer->vertex_array[vert+1] = vert1->vertex_k[1];
                renderer->vertex_array[vert+2] = vert1->vertex_k[2];
                renderer->normals[vert]        = vert1->normal[0];
                renderer->normals[vert+1]      = vert1->normal[1];
                renderer->normals[vert+2]      = vert1->normal[2];
                renderer->texture_uv[uv]       = vert1->uv[0];
                renderer->texture_uv[uv+1]     = vert1->uv[1];
                vert+=3; uv+=2;
            }
            
            for (v = 0; v < indexSize; v+=3)
            {
                TRI_INDICES *index = (TRI_INDICES*)(vertIndexOffset + sizeof(uint16_t) * v);
                renderer->index_array[v]   = index->tri_ind[0];
                renderer->index_array[v+1] = index->tri_ind[1];
                renderer->index_array[v+2] = index->tri_ind[2];
            }
            
            #define texCoord_buffer 1
            #define normals_buffer 2
            #define texCoord_buffer_light 3
            
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
            
            glBindVertexArrayAPPLE(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
        
        
        //Geometry *geometry = new Geometry(offset);
        //geometries.push_back(geometry);
    }
    
    printf("creating model object\n");
}

Model *ModelManager::create_model(ProtonMap *map, HaloTagDependency mod2) {
    printf("creating object %d\n", mod2.tag_id.tag_index);
    
    // Has this model been loaded before? Check the cache
    std::map<uint16_t, Model*>::iterator iter = models.find(mod2.tag_id.tag_index);
    if (iter != models.end()) {
        return iter->second;
    }
    
    // Create a new texture
    Model *model = new Model(map, mod2);
    models[mod2.tag_id.tag_index] = model;
    return model;
}