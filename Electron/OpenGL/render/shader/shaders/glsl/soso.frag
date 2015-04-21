#version 120
uniform sampler2D baseTexture;
uniform sampler2D multipurposeMap;
uniform sampler2D detailMap;
uniform sampler2D cubeMap;

uniform vec3 maps;  //0 = useMulti, useDetail, useCube
uniform vec4 scale; //0 = uscale, vscale, detailScale, detailScaleV
varying vec2 tex_coord;
void main(void) {
    vec2 mapScale    = vec2(scale[0], scale[1]);
    vec4 texel0 = texture2D(baseTexture, tex_coord*mapScale);
    vec4 texel1 = texture2D(detailMap, tex_coord*scale[2]);
    vec4 white = vec4(1.0,1.0,1.0,1.0);
    vec4 detail = mix(white, texel1, maps[1]);
    gl_FragColor = texel0 * detail;
}