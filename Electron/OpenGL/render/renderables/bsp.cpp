//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "bsp.h"

uint8_t* map2mem(ProtonTag *scenario, uint32_t address) {
    return (uint8_t*)(scenario->Data() + scenario->PointerToOffset(address));
}

void BSPRenderMesh::setup() {
    // Create the buffers for the vertices atttributes
    glGenVertexArraysAPPLE(1, &geometryVAO);
    glBindVertexArrayAPPLE(geometryVAO);
    
    // Create the buffers for the vertices atttributes
    glGenBuffers(5, m_Buffers);
}

BSP::BSP(ShaderManager* manager) {
    printf("bsp setup\n");
    shaders = manager;
}

//TODO: Group elements into renderables
void BSP::setup(ProtonMap *map, ProtonTag *scenario) {
    printf("bsp setup\n");
    HaloTagReflexive bsp = ((HaloScenarioTag*)scenario->Data())->bsp;
    int i, m, n;
    
    // Count renderables
    int renderable_count = 0;
    for (i=0; i < bsp.count; i++) {
        BSP_CHUNK *chunk = (BSP_CHUNK *)(map2mem(scenario, bsp.address) + sizeof(BSP_CHUNK) * i); // VERIFIED
        ProtonTag *bspTag = map->tags.at((uint16_t)(chunk->tagId)).get();
        uint32_t mesh_offset = *(uint32_t *)(bspTag->Data());
        BSP_MESH *mesh = (BSP_MESH *)map2mem(bspTag, mesh_offset);
        
        for (m=0; m < mesh->submeshHeader.count; m++) {
            BSP_SUBMESH *submesh = (BSP_SUBMESH *)(map2mem(bspTag, mesh->submeshHeader.address) + sizeof(BSP_SUBMESH) * m);
            renderable_count += submesh->material.count;
        }
    }
    
    // Generate renderables
    renderables.resize(renderable_count);
    
    int render_pos = 0;
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
                int vertex_number = material->VertexCount1;
                int indexSize = material->VertIndexCount*3;
                
                HaloTagDependency shader = material->ShaderTag;
                printf("shader setup %d %d\n", n, shader.tag_id.tag_index);
                shader_object *material_shader = shaders->create_shader(map, shader);
                
                printf("renderer setup %d\n", n);
                BSPRenderMesh *renderer = new BSPRenderMesh;
                renderer->setup();
                renderer->shader = material_shader;
                renderer->indexCount = indexSize;
                renderer->vertCount = vertex_number;
                
                printf("vbo setup %d\n", n);
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
                
                // Store the renderable
                renderables[render_pos] = renderer;
                render_pos++;
            }
        }
    }
    printf("done bsp setup\n");
}

void BSP::render(ShaderType pass) {
    
    
    // Render the renderables
    //glBindVertexArrayAPPLE([[mapBSP getBsp:number] geometryVAO]);
  
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    int i;
    
    shader_object *previous_shader = nullptr;
    for (i=0; i < renderables.size(); i++) {
        BSPRenderMesh *mesh = renderables[i];
        if (mesh->shader != nullptr && mesh->shader->is(pass)) {
            glBindVertexArrayAPPLE(mesh->geometryVAO);
            if (mesh->shader != previous_shader) {
                mesh->shader->render();
                previous_shader = mesh->shader;
            }
            glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
            glBindVertexArrayAPPLE(0);
        }
    }
}