//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "bsp.h"
#include "shader.h"

uint8_t* map2mem(ProtonTag *scenario, uint32_t address) {
    return (uint8_t*)(scenario->Data() + scenario->PointerToOffset(address));
}

class BSPRenderMesh {
public:
    GLuint m_Buffers[5];
    GLuint geometryVAO;
    shader_object *shader;
    
    void setup() {
        // Create the buffers for the vertices atttributes
        glGenVertexArraysAPPLE(1, &geometryVAO);
        glBindVertexArrayAPPLE(geometryVAO);
        
        // Create the buffers for the vertices atttributes
        glGenBuffers(5, m_Buffers);
    }
};

void BSP::setup(ProtonMap *map, ProtonTag *scenario) {
    printf("bsp setup\n");
    HaloTagReflexive bsp = ((HaloScenarioTag*)scenario->Data())->bsp;
    int i, m, n;
    
    // Count renderables
    
    // Generate renderables
    for (i=0; i < bsp.count; i++) {
        printf("chunk setup %d\n", i);
        BSP_CHUNK *chunk = (BSP_CHUNK *)(map2mem(scenario, bsp.address) + sizeof(BSP_CHUNK) * i); // VERIFIED
        ProtonTag *bspTag = map->tags.at((uint16_t)(chunk->tagId)).get();
        uint32_t mesh_offset = *(uint32_t *)(bspTag->Data());
        BSP_MESH *mesh = (BSP_MESH *)map2mem(bspTag, mesh_offset);
        
        for (m=0; m < mesh->submeshHeader.count; m++) {
            printf("mesh setup %d\n", m);
            BSP_SUBMESH *submesh = (BSP_SUBMESH *)(map2mem(bspTag, mesh->submeshHeader.address) + sizeof(BSP_SUBMESH) * m);
            
            for (n=0; n < submesh->material.count; n++) {
                printf("material setup %d\n", n);
                MATERIAL_SUBMESH_HEADER *material = (MATERIAL_SUBMESH_HEADER *)(map2mem(bspTag, submesh->material.address) + sizeof(MATERIAL_SUBMESH_HEADER) * n);
                uint8_t *vertIndexOffset = (uint8_t *)((sizeof(TRI_INDICES) * material->VertIndexOffset) + map2mem(bspTag, mesh->submeshIndices.address));
                uint8_t *PcVertexDataOffset = map2mem(bspTag, material->PcVertexDataOffset);
                
                printf("shader setup %d\n", n);
                HaloTagDependency shader = material->ShaderTag;
                shader_object *material_shader = load_shader(map, shader);
                
                printf("renderer setup %d\n", n);
                BSPRenderMesh *renderer = new BSPRenderMesh();
                renderer->setup();
                renderer->shader = material_shader;
                
                printf("vbo setup %d\n", n);
                int vertex_number = material->VertexCount1;
                int indexSize = material->VertIndexCount;
                GLfloat* vertex_array    = (GLfloat*)malloc(vertex_number   * 3 * sizeof(GLfloat));
                GLfloat* texture_uv      = (GLfloat*)malloc(vertex_number   * 2 * sizeof(GLfloat));
                GLfloat* light_uv        = (GLfloat*)malloc(vertex_number   * 2 * sizeof(GLfloat));
                GLfloat* normals         = (GLfloat*)malloc(vertex_number   * 3 * sizeof(GLfloat));
                GLint* index_array       = (GLint*)malloc(indexSize * sizeof(GLint));

                int v;
                int vert = 0, uv = 0;
                for (v = 0; v < vertex_number; v++)
                {
                    UNCOMPRESSED_BSP_VERT *vert1 = (UNCOMPRESSED_BSP_VERT*)(PcVertexDataOffset + v * sizeof(UNCOMPRESSED_BSP_VERT));
                    vertex_array[vert]   = vert1->vertex_k[0];
                    vertex_array[vert+1] = vert1->vertex_k[1];
                    vertex_array[vert+2] = vert1->vertex_k[2];
                    normals[vert]        = vert1->normal[0];
                    normals[vert+1]      = vert1->normal[1];
                    normals[vert+2]      = vert1->normal[2];
                    texture_uv[uv]       = vert1->uv[0];
                    texture_uv[uv+1]     = vert1->uv[1];
                    vert+=3; uv+=2;
                }

                #define texCoord_buffer 1
                #define normals_buffer 2
                #define texCoord_buffer_light 3
                
                //Shift these to vertex buffers
                glBindBuffer(GL_ARRAY_BUFFER, renderer->m_Buffers[POS_VB]);
                glBufferData(GL_ARRAY_BUFFER, vertex_number * 3 * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
                GLvoid* my_vertex_pointer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
                memcpy(my_vertex_pointer, vertex_array, vertex_number * 3 * sizeof(GLfloat));
                glUnmapBuffer(GL_ARRAY_BUFFER);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
                
                glBindBuffer(GL_ARRAY_BUFFER, renderer->m_Buffers[TEXCOORD_VB]);
                glBufferData(GL_ARRAY_BUFFER, vertex_number * 2 * sizeof(GLfloat), &texture_uv[0], GL_STATIC_DRAW);
                glEnableVertexAttribArray(texCoord_buffer);
                glVertexAttribPointer(texCoord_buffer, 2, GL_FLOAT, GL_FALSE, 0, 0);
                
                glBindBuffer(GL_ARRAY_BUFFER, renderer->m_Buffers[NORMAL_VB]);
                glBufferData(GL_ARRAY_BUFFER, vertex_number * 3 * sizeof(GLfloat), &normals[0], GL_STATIC_DRAW);
                glEnableVertexAttribArray(normals_buffer);
                glVertexAttribPointer(normals_buffer, 3, GL_FLOAT, GL_FALSE, 0, 0);
                
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->m_Buffers[INDEX_BUFFER]);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * sizeof(GLint), &index_array[0], GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, renderer->m_Buffers[LIGHT_VB]);
                glBufferData(GL_ARRAY_BUFFER, vertex_number * 2 * sizeof(GLfloat), &light_uv[0], GL_STATIC_DRAW);
                glEnableVertexAttribArray(texCoord_buffer_light);
                glVertexAttribPointer(texCoord_buffer_light, 2, GL_FLOAT, GL_FALSE, 0, 0);
                
                glBindVertexArrayAPPLE(0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            }
        }
    }
    printf("done bsp setup\n");
}

void BSP::render(ProtonMap *map, ProtonTag *scenario) {
    
    
    // Render the renderables
    //glBindVertexArrayAPPLE([[mapBSP getBsp:number] geometryVAO]);
    
    
    
    HaloTagReflexive bsp = ((HaloScenarioTag*)scenario->Data())->bsp;
    int i;
    for (i=0; i < bsp.count; i++) {
        BSP_CHUNK *chunk = (BSP_CHUNK *)(map2mem(scenario, bsp.address) + sizeof(BSP_CHUNK) * i); // VERIFIED
        ProtonTag *bspTag = map->tags.at((uint16_t)(chunk->tagId)).get();
        uint32_t mesh_offset = *(uint32_t *)(bspTag->Data());
        BSP_MESH *mesh = (BSP_MESH *)map2mem(bspTag, mesh_offset);
        
        int m;
        for (m=0; m < mesh->submeshHeader.count; m++) {
            BSP_SUBMESH *submesh = (BSP_SUBMESH *)(map2mem(bspTag, mesh->submeshHeader.address) + sizeof(BSP_SUBMESH) * m);
            
            int n;
            for (n=0; n < submesh->material.count; n++) {
                MATERIAL_SUBMESH_HEADER *material = (MATERIAL_SUBMESH_HEADER *)(map2mem(bspTag, submesh->material.address) + sizeof(MATERIAL_SUBMESH_HEADER) * n);
                uint8_t *vertIndexOffset = (uint8_t *)((sizeof(TRI_INDICES) * material->VertIndexOffset) + map2mem(bspTag, mesh->submeshIndices.address));
                uint8_t *PcVertexDataOffset = map2mem(bspTag, material->PcVertexDataOffset);
                
                // Bind the VAO
                
                
                
                glBegin(GL_TRIANGLES);
                
                int v;
                for (v=0; v < material->VertIndexCount; v++) {
                    TRI_INDICES *index = (TRI_INDICES*)(vertIndexOffset + sizeof(TRI_INDICES) * v);
                    
                    // Render a vertex for tri_ind[0]
                    UNCOMPRESSED_BSP_VERT *vert1 = (UNCOMPRESSED_BSP_VERT*)(PcVertexDataOffset + index->tri_ind[0] * sizeof(UNCOMPRESSED_BSP_VERT));
                    UNCOMPRESSED_BSP_VERT *vert2 = (UNCOMPRESSED_BSP_VERT*)(PcVertexDataOffset + index->tri_ind[1] * sizeof(UNCOMPRESSED_BSP_VERT));
                    UNCOMPRESSED_BSP_VERT *vert3 = (UNCOMPRESSED_BSP_VERT*)(PcVertexDataOffset + index->tri_ind[2] * sizeof(UNCOMPRESSED_BSP_VERT));
                    
                    glVertex3f(vert1->vertex_k[0], vert1->vertex_k[1], vert1->vertex_k[2]);
                    glVertex3f(vert2->vertex_k[0], vert2->vertex_k[1], vert2->vertex_k[2]);
                    glVertex3f(vert3->vertex_k[0], vert3->vertex_k[1], vert3->vertex_k[2]);
                    glColor4f(arc4random()%10000/10000.0, arc4random()%10000/10000.0, arc4random()%10000/10000.0, 1.0);
                }
                
                glEnd();
            }
        }
    }
}