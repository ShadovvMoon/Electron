//
//  shader.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "shader.h"
#include "shaders/senv.h"

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
        fprintf(stderr, "Cannot load %s: Missing source", filename);
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
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
    if (!program_ok)
    {
        fprintf(stderr, "Failed to link shader program:\n");
        glDeleteProgram(program);
        return 0;
    }
    return program;
}


GLuint load_bitm(HaloTagDependency bitm) {
    // Has this bitmap been loaded before? Check the cache
    
    // Create a new texture
    GLuint texture;
    glGenTextures(1,&texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    return texture;
}

shader_object *load_shader(ProtonMap *map, HaloTagDependency shader) {
    ProtonTag *shaderTag = map->tags.at((uint16_t)shader.tag_id.tag_index).get();
    if(strncmp(shaderTag->tag_classes, "vnes", 4) == 0) { // senv shader
        senv_object *shaderObj = new senv_object();
        shaderObj->setup(map, shaderTag);
        return shaderObj;
    }
    return nullptr;
}

void shader::setup() {
    fprintf(stderr, "INVALID SHADER - setup()\n");
}
void shader::start() {
    fprintf(stderr, "INVALID SHADER - start()\n");
}
void shader::stop() {
    fprintf(stderr, "INVALID SHADER - stop()\n");
}
void shader_object::setup(ProtonMap *map, ProtonTag *shaderTag) {
    fprintf(stderr, "INVALID SHADER OBJECT - setup()\n");
};

void shader_object::render() {
    fprintf(stderr, "INVALID SHADER OBJECT - render()\n");
}


shader *shaders[ShaderCount];
void load_shaders() {
    senv *senv_shader = new senv();
    senv_shader->setup();
    shaders[shader_SENV] = (shader*)senv_shader;
}

shader *get_shader(ShaderType pass) {
    return shaders[pass];
}
