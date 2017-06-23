//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "object.h"
#include "scen.h"
#include "vehi.h"
#include "itmc.h"

ObjectRef::ObjectRef(ObjectManager *manager, ProtonMap *map, HaloTagDependency tag) {
    printf("creating object ref\n");
    TagID = tag;
    
    if (tag.tag_id.tag_index != NULLED_TAG_ID && tag.tag_id.tag_index > 0 && tag.tag_id.tag_index < map->tags.size()) {
        ProtonTag *objectTag = map->tags.at(tag.tag_id.tag_index).get();
        HaloObjectTagData *objectData = (HaloObjectTagData *)objectTag->Data();
        
        printf("talking to models\n");
        model = manager->modelManager->create_model(map, objectData->model);
    } else {
        printf("null model\n");
        model = nullptr;
    }
}

void ObjectRef::render(ShaderType pass) {
    model->render(pass);
}

ObjectRef *ObjectManager::create_object(ProtonMap *map, HaloTagDependency tag) {
    //fprintf(stderr, "creating object %d %c%c%c%c\n", tag.tag_id.tag_index, tag.tag_class[3], tag.tag_class[2], tag.tag_class[1], tag.tag_class[0]);
    
    // Has this bitmap been loaded before? Check the cache
    std::map<uint16_t, ObjectRef*>::iterator iter = this->objects.find(tag.tag_id.tag_index);
    if (iter != this->objects.end()) {
        return iter->second;
    }
    
    // Create a new texture
    ObjectRef *obj = new ObjectRef(this, map, tag);
    this->objects[tag.tag_id.tag_index] = obj;
    return obj;
}

ObjectClass *ObjectManager::getClass(SelectionType type) {
    switch (type) {
        case s_scenery:
            return scen;
            break;
        case s_vehicle:
            return vehi;
            break;
        case s_item:
            return itmc;
            break;
        default:
            break;
    }
    return nullptr;
}

void ObjectManager::read(ShaderManager *shaders, ProtonMap *map, ProtonTag *scenario) {
    printf("starting object manager\n");
    selection = std::vector<ObjectInstance*>();
    modelManager = new ModelManager(shaders);
    scen = new ScenClass;
    scen->read(this, map, scenario);
    vehi = new VehiClass;
    vehi->read(this, map, scenario);
    itmc = new ItmcClass;
    itmc->read(this, map, scenario);
    
	// Clear selection
	clearSelection();

    // Store the map and scenario for bsp lookup
    this->map = map;
    this->scenario = scenario;
}

void ObjectManager::write(ProtonMap *map, ProtonTag *scenario) {
    scen->write(map, scenario);
    vehi->write(map, scenario);
    itmc->write(map, scenario);
}

typedef float MATRIX[4][4]; // A 4×4 matrix
void matmult(MATRIX &A, MATRIX B)
{
    MATRIX conc;
    // Multiplies by rows and columns
    for (int i = 0; i <4; i++)
        for (int j = 0; j < 4; j++)
        {
            conc[i][j] =
            (A[0][j] * B[i][0])+
            (A[1][j] * B[i][1])+
            (A[2][j] * B[i][2])+
            (A[3][j] * B[i][3]);
        }
    // Copy result matrix into matrix A
    for (int p = 0; p < 4; p++)
        for (int q = 0; q < 4; q++)
            A[p][q] = conc[p][q];
}

// matinit: Initializes a matrix, and sets to 'identity'
void matinit(MATRIX &a)
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            a[i][j] = 0.0f;
    a[0][0] = 1.0f;
    a[1][1] = 1.0f;
    a[2][2] = 1.0f;
    a[3][3] = 1.0f;
}

#include <xmmintrin.h>
void M4x4_SSE(float *A, float *B, float *C) {
    __m128 row1 = _mm_load_ps(&B[0]);
    __m128 row2 = _mm_load_ps(&B[4]);
    __m128 row3 = _mm_load_ps(&B[8]);
    __m128 row4 = _mm_load_ps(&B[12]);
    for(int i=0; i<4; i++) {
        __m128 brod1 = _mm_set1_ps(A[4*i + 0]);
        __m128 brod2 = _mm_set1_ps(A[4*i + 1]);
        __m128 brod3 = _mm_set1_ps(A[4*i + 2]);
        __m128 brod4 = _mm_set1_ps(A[4*i + 3]);
        __m128 row = _mm_add_ps(
                                _mm_add_ps(
                                           _mm_mul_ps(brod1, row1),
                                           _mm_mul_ps(brod2, row2)),
                                _mm_add_ps(
                                           _mm_mul_ps(brod3, row3),
                                           _mm_mul_ps(brod4, row4)));
        _mm_store_ps(&C[4*i], row);
    }
}

#include <math.h>
void ObjectManager::render_instance(ObjectInstance *instance, ShaderType pass, shader_options *options) {
    // Frustrum culling

    // Culling
    //vector3d *position = new vector3d(instance->x, instance->y, instance->z);
    //bsp->intersect(camera->position, position, map, scenario); too slow 
    //delete position;
    
    // Render
    if (instance->selected) {
        // Render a cross as the coordinates. TODO: Replace with non fixed pipeline
        if (pass == shader_NULL) {
            float lineLen = 100.0;
            float largeFloat = 1000000.0;
            vector3d *position      = new vector3d(instance->x, instance->y, instance->z);
            vector3d *position_down = new vector3d(instance->x, instance->y, instance->z-largeFloat);
            vector3d *intersect = bsp->intersect(position_down, position, map, scenario);

            // Is this object below the BSP? Move it above automatically
            #ifndef RENDER_PIPELINE
                #ifndef RENDER_CORE_32
                glEnable(GL_COLOR_MATERIAL);
                if (intersect == nullptr) {
                    glColor4f(1.0, 0.0, 0.0, 1.0);
                } else {
                    delete intersect;
                    glColor4f(1.0, 1.0, 1.0, 1.0);
                }
                delete position;
                delete position_down;
            
                glLineWidth(1.0);
                glBegin(GL_LINES);
                glVertex3f(instance->x-lineLen, instance->y, instance->z);
                glVertex3f(instance->x+lineLen, instance->y, instance->z);
                glVertex3f(instance->x, instance->y-lineLen, instance->z);
                glVertex3f(instance->x, instance->y+lineLen, instance->z);
                glVertex3f(instance->x, instance->y, instance->z-lineLen);
                glVertex3f(instance->x, instance->y, instance->z+lineLen);
                glEnd();
                #endif
            #endif
        }
    }
    
    //#ifdef RENDER_CORE_32
    options->position[0] = 0;
    options->position[1] = 0;
    options->position[2] = 0;

    // Store the current modelview
    /*
    fprintf(stderr, "\n\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n",
            options->modelview[0],
            options->modelview[1],
            options->modelview[2],
            options->modelview[3],
            options->modelview[4],
            options->modelview[5],
            options->modelview[6],
            options->modelview[7],
            options->modelview[8],
            options->modelview[9],
            options->modelview[10],
            options->modelview[11],
            options->modelview[12],
            options->modelview[13],
            options->modelview[14],
            options->modelview[15]);
    */
    
    float modelview[16];
    memcpy(modelview, options->modelview, sizeof(float) * 16);
    GLfloat output[16];
    memcpy(output, options->modelview, sizeof(float) * 16);
    
    // Perform the translation
    float t[16] = {1.0, 0.0, 0.0, 0.0f,
                     0.0, 1.0, 0.0, 0.0f,
                     0.0, 0.0, 1.0, 0.0f,
                     instance->x, instance->y, instance->z, 1.0f};
    // Perform the rotation
    float angle = instance->roll;
    float c = cos(angle/CONVERSION);
    float s = sin(angle/CONVERSION);
    float x[16] = {1.0, 0.0, 0.0, 0.0f,
                     0.0,   c,   s, 0.0f,
                     0.0,  -s,   c, 0.0f,
                     0.0, 0.0, 0.0, 1.0f};
    angle = instance->pitch;
    c = cos(angle/CONVERSION);
    s = sin(angle/CONVERSION);
    float y[16] = {  c, 0.0,  -s, 0.0f,
                     0.0, 1.0, 0.0, 0.0f,
                       s, 0.0,   c, 0.0f,
                     0.0, 0.0, 0.0, 1.0f};
    angle = instance->yaw;
    c = cos(angle/CONVERSION);
    s = sin(angle/CONVERSION);
    float z[16] = {  c,   s, 0.0, 0.0f,
                      -s,   c, 0.0, 0.0f,
                     0.0, 0.0, 1.0, 0.0f,
                     0.0, 0.0, 0.0, 1.0f};
    M4x4_SSE(t, output, output);
    M4x4_SSE(x, output, output);
    M4x4_SSE(y, output, output);
    M4x4_SSE(z, output, output);
    memcpy(options->modelview, &output, sizeof(float) * 16);
    
    // Render the object
    shader *shader = modelManager->shaders->get_shader(pass);
    shader->update(options);
    #ifdef RENDER_PIPELINE
    instance->reference->render(pass);
    #endif
    
    /*
    fprintf(stderr, "\n\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n",
            options->modelview[0],
            options->modelview[1],
            options->modelview[2],
            options->modelview[3],
            options->modelview[4],
            options->modelview[5],
            options->modelview[6],
            options->modelview[7],
            options->modelview[8],
            options->modelview[9],
            options->modelview[10],
            options->modelview[11],
            options->modelview[12],
            options->modelview[13],
            options->modelview[14],
            options->modelview[15]);
    */
    
    // Restore the modelview
    memcpy(options->modelview, &modelview, sizeof(GLfloat) * 16);
    #ifndef RENDER_PIPELINE
        glPushMatrix();
        glTranslatef(instance->x, instance->y, instance->z);
        glRotatef(instance->roll , 1.0, 0.0, 0.0); //1.5%
        glRotatef(instance->pitch, 0.0, 1.0, 0.0); //0.9%
        glRotatef(instance->yaw  , 0.0, 0.0, 1.0); //1.2%
        instance->reference->render(pass);
        glPopMatrix();
        if (instance->selected) {
            glColor4f(1.0, 1.0, 1.0, 1.0);
        }
    #endif
}

void ObjectManager::render_subclass(ObjectClass* objClass, SelectionType selection, uint *name, uint *lookup, ShaderType pass, shader_options *options) {
    int i;
    for (i=0; i < objClass->objects.size(); i++) {
        #ifdef RENDER_PIPELINE
            // TODO
            render_instance(objClass->objects[i], pass, options);
        #else
            glLoadName(*name);
            glPushName(*name);
            if (lookup) {
                lookup[*name] = (GLuint)((selection * MAX_SCENARIO_OBJECTS) + i);
                (*name)++;
            }
            render_instance(objClass->objects[i], pass, options);
            glPopName();
        #endif
    }
}

void ObjectManager::fast_render_subclass(ObjectClass* objClass, SelectionType selection, ShaderType pass, shader_options *options) {
    int i;
    for (i=0; i < objClass->objects.size(); i++) {
        render_instance(objClass->objects[i], pass, options);
    }
}

void clear_selection(ObjectClass* objClass) {
    int i;
    for (i=0; i < objClass->objects.size(); i++) {
        objClass->objects[i]->selected = false;
    }
}

ObjectInstance *ObjectManager::duplicate(ObjectInstance *instance) {
    printf("duplicate object\n");
    SelectionType ctype = instance->type();
    ObjectInstance *copy = instance->duplicate();
    getClass(ctype)->objects.push_back(copy);
    return copy;
}

void ObjectManager::render(uint *name, uint *lookup, ShaderType pass, shader_options *options) {
    if (lookup == nullptr) {
        fast_render_subclass(scen, s_scenery, pass, options);
        fast_render_subclass(vehi, s_vehicle, pass, options);
        fast_render_subclass(itmc, s_item, pass, options);
    } else {
        render_subclass(scen, s_scenery, name, lookup, pass, options);
        render_subclass(vehi, s_vehicle, name, lookup, pass, options);
        render_subclass(itmc, s_item, name, lookup, pass, options);
    }
}

ObjectManager::ObjectManager(Camera *camera, BSP* bsp) {
    this->bsp = bsp;
    this->camera = camera;
}

void ObjectManager::clearSelection() {
    selection.clear();
    clear_selection(scen);
    clear_selection(vehi);
    clear_selection(itmc);
}

void ObjectManager::select(bool shift, float x, float y) {
    printf("select %f %f\n",x,y);
    #ifdef RENDER_PIPELINE
        //TODO
    #else
        const GLsizei bufferSize = 16384;
        GLuint nameBuf[bufferSize];
        GLuint tmpLookup[bufferSize];
    
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT,viewport);
        glSelectBuffer(bufferSize,nameBuf);
        glRenderMode(GL_SELECT);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
    
        GLdouble selectWidth = 1.0;
        GLdouble selectHeight = 1.0;
        gluPickMatrix((GLdouble)(x + selectWidth / 2),(GLdouble)(y + selectHeight / 2),(GLdouble)(selectWidth),(GLdouble)(selectHeight),viewport);
        gluPerspective(45.0f,(GLfloat)((GLfloat)(viewport[2] - viewport[0])/(GLfloat)(viewport[3] - viewport[1])),0.1f,400000.0f);
        glMatrixMode(GL_MODELVIEW);

        GLuint name = 1;
        GLuint *lookup = (GLuint *)tmpLookup;
        glInitNames();
    
        shader_options *options = new shader_options;
    
        printf("render names\n");
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glEnableClientState(GL_VERTEX_ARRAY);
        ShaderType type = static_cast<ShaderType>(shader_SOSO);
        shader *shader = modelManager->shaders->get_shader(shader_SOSO);
        shader->start(options);
        render(&name, lookup, shader_SOSO, options);
        shader->stop();
        glDisableClientState(GL_VERTEX_ARRAY);
        glPopAttrib();
        free(options);
        printf("done render names\n");
    
        GLuint hits = glRenderMode(GL_RENDER);
        GLuint names, *ptr = (GLuint *)nameBuf;
        if (hits == 0) {
            clearSelection();
            return;
        }

        int i;
        unsigned int  j, z1, z2;
        for (i = 0; i < hits; i++) {
            names = *ptr;
            ptr++;
            z1 = (float)*ptr/0x7fffffff;
            ptr++;
            z2 = (float)*ptr/0x7fffffff;
            ptr++;
            for (j = 0; j < names; j++) {
                if (*ptr >= bufferSize)
                    break;
                printf("%d %d\n", bufferSize, *ptr);
                int type  = (unsigned int)(lookup[*ptr] / MAX_SCENARIO_OBJECTS);
                int index = (unsigned int)(lookup[*ptr] % MAX_SCENARIO_OBJECTS);
                
                SelectionType ctype = static_cast<SelectionType>(type);
                ObjectInstance *instance = getClass(ctype)->objects[index];
                if (!instance->selected) {
                    instance->selected = true;
                    selection.push_back(instance);
                }
            }
        }
    
        // If shift is down, duplicate all of the selected objects
        if (shift) {
            std::vector<ObjectInstance*> duplicates;
            duplicates.resize(selection.size());
            int i;
            for (i=0; i < selection.size(); i++) {
                duplicates[i] = duplicate(selection[i]);
            }
            clearSelection();
            selection.resize(duplicates.size());
            for (i=0; i < selection.size(); i++) {
                selection[i] = duplicates[i];
                selection[i]->selected = true;
            }
        }
        glPopMatrix();
    #endif
}
