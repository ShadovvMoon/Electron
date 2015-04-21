#version 120
uniform sampler2D baseTexture;
uniform sampler2D primaryDetailMap;
uniform sampler2D secondaryDetailMap;
uniform sampler2D lightMap;

uniform vec2 baseMapUV;
uniform vec4 maps; //0 = hasPrimary, 1 = primaryScale, 2 = hasSecondary, 3 = secondaryScale
uniform vec2 maps2;  //0 = useLight, //1 = blend

varying vec2 tex_coord;
varying vec2 tex_coord_light;
void main(void) {
    vec4 texel0 = texture2D(baseTexture, tex_coord);
    vec4 texel1 = texture2D(primaryDetailMap, tex_coord*maps[1]);
    vec4 texel2 = texture2D(secondaryDetailMap, tex_coord*maps[3]);
    vec4 light  = texture2D(lightMap, tex_coord_light);
    
    vec4 white   = vec4(1.0,1.0,1.0,1.0);
    vec4 detail1 = mix(white, texel1 * 2.0, maps[0]);
    vec4 detail2 = mix(white, texel2 * 2.0, maps[2]);
    vec4 detail3 = mix(white, light, maps2[0]);
    vec4 detail  = mix(detail2, detail1, texel0.a);
    gl_FragColor = texel0 * detail * detail3;
    gl_FragColor.a = 1.0;
}