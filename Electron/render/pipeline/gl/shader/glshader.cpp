//
//  shader.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "glshader.h"
#include "shaders/senv.h"
#include "shaders/schi.h"
#include "shaders/soso.h"
#include "shaders/null.h"
#include "shaders/sgla.h"
#include "shaders/swat.h"
#include "shaders/scex.h"
#include "shaders/deff.hpp"
#include "shaders/ssao.hpp"
#include "shaders/blur.hpp"
#include "shaders/senv_reflect.h"

float b2f(bool b) {
    return b?1.0:0.0;
}

char *file_contents(const char *filename, GLint *len)
{
    char * buffer = 0;
    GLint length;
    FILE * f = fopen (filename, "rb");
    
    if (f)
    {
        fseek (f, 0, SEEK_END);
        length = (GLuint)ftell(f);
        fseek (f, 0, SEEK_SET);
        buffer = (char*)malloc (length);
        if (buffer)
        {
            fread (buffer, 1, length, f);
        }
        fclose (f);
    }
    memcpy(len, &length, sizeof(GLint));
    return buffer;
}


GLuint make_shader(GLenum type, const char *filename)
{
    GLint length;
    GLchar *source = (GLchar*)file_contents(filename, &length);
    GLuint shader;
    GLint shader_ok;
    
    if (!source)
    {
        fprintf(stderr, "Cannot load %s: Missing source\n", filename);
        return 0;
    }
    
    shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar**)&source, &length);
    free(source);
    glCompileShader(shader);
    
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
    if (!shader_ok)
    {
        fprintf(stderr, "Failed to compile %s:\n", filename);
        
        GLint log_length;
        char *log;
        
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        log = (char*)malloc(log_length);
        glGetShaderInfoLog(shader, log_length, NULL, log);
        fprintf(stderr, "Error: %s", log);
        free(log);
        
        glDeleteShader(shader);
        return 0;
    }
    else
    {
        fprintf(stderr, "Loaded %s\n", filename);
    }
    return shader;
}

GLuint make_program(GLuint vertex_shader, GLuint fragment_shader)
{
    GLint program_ok;
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glBindAttribLocation(program, 0, "vertex_buffer");
	glBindAttribLocation(program, 1, "texCoord_buffer");
	glBindAttribLocation(program, 3, "texCoord_buffer_light");
	glBindAttribLocation(program, 2, "normal_buffer");
    glBindAttribLocation(program, 5, "binormals_buffer");
    glBindAttribLocation(program, 6, "tangents_buffer");
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
    if (!program_ok) {
        fprintf(stderr, "Failed to link shader program:\n");
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

GLShaderManager::GLShaderManager(const char *resources) {
    printf("shader manager setup\n");
    
    // Create the reflection
    reflections.resize(Reflections);
    int i;
    for (i=0; i < Reflections; i++) {
        glDeleteTextures(1, &reflections[i]);
        glGenTextures(1, &reflections[i]);
        glBindTexture(GL_TEXTURE_2D, reflections[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Create a texture manager
    textures = new GLTextureManager();
    
#ifdef _WINDOWS
	std::string path = "C:/Users/admin_000/Documents/GitHub/Electron/Electron/OpenGL/render/shader/shaders/glsl";
#else
    std::string path = resources;
#endif
    
#ifdef RENDER_CORE_32
    path += "/440";
#else
    path += "/120";
#endif

    // Setup our shader types
    senv *senv_shader = new senv;
    senv_shader->setup(path);
    shaders[shader_SENV] = (shader*)senv_shader;
    
    schi *schi_shader = new schi;
    schi_shader->setup(path);
    shaders[shader_SCHI] = (shader*)schi_shader;
    
    soso *soso_shader = new soso;
    soso_shader->setup(path);
    shaders[shader_SOSO] = (shader*)soso_shader;
    
    null *null_shader = new null;
    null_shader->setup(path);
    shaders[shader_NULL] = (shader*)null_shader;
    
    sgla *sgla_shader = new sgla;
    sgla_shader->setup(path);
    shaders[shader_SGLA] = (shader*)sgla_shader;
    
    swat *swat_shader = new swat;
    swat_shader->setup(path);
    shaders[shader_SWAT] = (shader*)swat_shader;
    
    scex *scex_shader = new scex;
    scex_shader->setup(path);
    shaders[shader_SCEX] = (shader*)scex_shader;
    
    deff *deff_shader = new deff;
    deff_shader->setup(path);
    shaders[shader_DEFF] = (shader*)deff_shader;
    
    ssao *ssao_shader = new ssao;
    ssao_shader->setup(path);
    shaders[shader_SSAO] = (shader*)ssao_shader;
    
    blur *blur_shader = new blur;
    blur_shader->setup(path);
    shaders[shader_BLUR] = (shader*)blur_shader;
    
    senv_reflect *senv_reflect_shader = new senv_reflect;
    senv_reflect_shader->setup(path);
    shaders[shader_SENV_REFLECT] = (shader*)senv_reflect_shader;
}

shader_object * GLShaderManager::create_shader(ProtonMap *map, HaloTagDependency shader) {
    // Do we already have this shader?
    std::map<uint16_t, shader_object*>::iterator iter = shader_objects.find(shader.tag_id.tag_index);
    if (iter != shader_objects.end()) {
        return iter->second;
    }
    
    ProtonTag *shaderTag = map->tags.at((uint16_t)shader.tag_id.tag_index).get();
    shader_object *shaderObj = nullptr;
    if(strncmp(shaderTag->tag_classes, "vnes", 4) == 0) { // senv shader
        shaderObj = new senv_object;
    } else if(strncmp(shaderTag->tag_classes, "ihcs", 4) == 0) { // senv shader
        shaderObj = new schi_object;
    } else if(strncmp(shaderTag->tag_classes, "osos", 4) == 0) { // senv shader
        shaderObj = new soso_object;
    } else if(strncmp(shaderTag->tag_classes, "algs", 4) == 0) { // senv shader
        shaderObj = new sgla_object;
    } else if(strncmp(shaderTag->tag_classes, "taws", 4) == 0) { // senv shader
        shaderObj = new swat_object;
    } else if(strncmp(shaderTag->tag_classes, "xecs", 4) == 0) { // senv shader
        shaderObj = new scex_object;
    }
    
    if (shaderObj != nullptr) {
        shaderObj->setup(this, map, shaderTag);
    }
    shader_objects[shader.tag_id.tag_index] = shaderObj;
    return shaderObj;
}

TextureManager *GLShaderManager::texture_manager() {
    return textures;
}

shader * GLShaderManager::get_shader(ShaderType pass) {
    return shaders[pass];
}

GLuint GLShaderManager::get_reflection(int index) {
    return reflections[index];
}

bool GLShaderManager::needs_reflection() {
    return reflecting;
}

void GLShaderManager::set_needs_reflection(bool reflect) {
    reflecting = reflect;
}

float GLShaderManager::reflection_height() {
    return reflect_height;
}
void GLShaderManager::set_reflection_height(float height) {
    reflect_height = height;
}
