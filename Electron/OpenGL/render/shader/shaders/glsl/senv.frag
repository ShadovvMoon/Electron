#version 120
uniform sampler2D baseTexture;
uniform sampler2D primaryDetailMap;
uniform sampler2D secondaryDetailMap;

uniform vec4 maps; //0 = hasPrimary, 1 = primaryScale, 2 = hasSecondary, 3 = secondaryScale
varying vec2 tex_coord;
void main(void) {
    vec4 texel0 = texture2D(baseTexture, tex_coord);
    vec4 texel1 = texture2D(primaryDetailMap, tex_coord*maps[1]);
    vec4 texel2 = texture2D(secondaryDetailMap, tex_coord*maps[3]);
    
    vec4 white = vec4(1.0,1.0,1.0,1.0);
    vec4 detail1 = mix(white, texel1, maps[0]);
    vec4 detail2 = mix(white, texel2, maps[2]);
    vec4 detail  = mix(detail2, detail1, texel0.a);

    gl_FragColor = texel0 * detail;
    gl_FragColor.a = 1.0;
}