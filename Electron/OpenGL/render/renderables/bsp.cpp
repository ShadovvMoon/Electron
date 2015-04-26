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
    free(vao->vertex_array);
    free(vao->texture_uv);
    free(vao->light_uv);
    free(vao->normals);
    free(vao->index_array);
    free(vao->binormals);
    free(vao->tangents);
}

void BSP::render(ShaderType pass) {
    
    //glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
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
#else
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
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vao->m_Buffers[TANGENT_VB]);
    glVertexAttribPointer(tangents_buffer, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao->m_Buffers[INDEX_BUFFER]);
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
                glDrawElementsBaseVertex(GL_TRIANGLES,
                                         submesh->indexCount,
                                         GL_UNSIGNED_INT,
                                         (void*)(submesh->indexOffset * sizeof(GLuint)),
                                         (submesh->vertexOffset));
            }
        }
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