#version 120
uniform sampler2D multipurposeMap;
uniform samplerCube cubeTextureMap;

uniform vec2 maps;  //0 = useMulti, useDetail, useCube
uniform vec3 scale; //0 = uscale, vscale, detailScale, detailScaleV
varying vec2 tex_coord;
varying vec3 normals;

// Cubemapping
varying vec3 lightVec, eyeVec;
uniform int togglebump; // false/true
uniform int textureon; // false/true
uniform float normalweight = 1.0;
varying mat3 TBNMatrix;
uniform vec2 reflectionScale;

// Fog
uniform vec4 fog;
uniform vec2 fogSettings;

void main(void) {
    // CUBEMAPPING
    vec2 coords  = vec2(tex_coord[0]*scale[0], tex_coord[1]*scale[1]);
    vec4 white   = vec4(1.0,1.0,1.0,1.0);

    vec4 reflex = vec4(0.0, 0.0, 0.0, 1.0);
    vec3 eye = normalize(eyeVec);
    eye = vec3(eye.x * 0.5, eye.y * 0.5, eye.z);
    
    // Query the Maps
    vec3 color = vec3(0.0,0.0,0.0);
    vec3 norm = normalize(normals);
    if ( maps[0] > 0 ) {
        norm = texture2D(multipurposeMap, coords*scale[2]).rgb - 0.5;
        norm = vec3(norm.x * normalweight, norm.y * normalweight, norm.z);
    }
    vec3 refl = reflect(norm, eye);  // in tangent space !
    vec3 reflw = vec3( 1.0, -1.0, 1.0) * (TBNMatrix * refl);
    reflex = textureCube(cubeTextureMap, reflw);
    
    float oneMinusDot = 1.0 - dot(normalize(normals), normalize(eye));
    float scale = mix(reflectionScale[0], reflectionScale[1], oneMinusDot);
    gl_FragColor = reflex;
    gl_FragColor.a = scale;

    // FOGGING
    float z = (gl_FragCoord.z / gl_FragCoord.w);
    float fogFactor = (z/fogSettings[0]);
    fogFactor = clamp(fogFactor, 0.0, fogSettings[1]);
    gl_FragColor = mix(gl_FragColor, vec4(fog[0],fog[1],fog[2],1.0), fogFactor);
}