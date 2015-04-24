#version 120
uniform sampler2D baseTexture;
uniform sampler2D multipurposeMap;
uniform sampler2D detailMap;
uniform samplerCube cubeTextureMap;
uniform vec2 baseMapUV;

uniform vec3 maps;  //0 = useMulti, useDetail, useCube
uniform vec4 scale; //0 = uscale, vscale, detailScale, detailScaleV
varying vec2 tex_coord;
varying vec3 normals;
void main(void) {
    vec2 coords  = vec2(tex_coord[0]*scale[0], tex_coord[1]*scale[1]);
    vec4 texel0  = texture2D(baseTexture, coords);
    vec4 texel1  = texture2D(detailMap, coords*scale[2]);
    vec4 texel2  = texture2D(multipurposeMap, coords);
    vec4 texel3  = textureCube(cubeTextureMap, normals);
    
    vec4 white   = vec4(1.0,1.0,1.0,1.0);
    vec4 detail  = mix(white, texel1*2, maps[1]);
    vec4 cube    = mix(white, texel3*2, maps[2]);
    
    gl_FragColor = gl_FrontMaterial.diffuse.rgba * texel0 * detail * cube;
}