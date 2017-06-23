#version 410 core
uniform sampler2D baseTexture;
uniform sampler2D multipurposeMap;
uniform sampler2D detailMap;
uniform samplerCube cubeTextureMap;
uniform vec2 baseMapUV;

uniform vec3 maps;  //0 = useMulti, useDetail, useCube
uniform vec4 scale; //0 = uscale, vscale, detailScale, detailScaleV
in vec2 tex_coord;
in vec3 normals;

// Cubemapping
in vec3 lightVec, eyeVec;
in mat3 TBNMatrix;

uniform int togglebump; // false/true
uniform int textureon; // false/true
uniform float normalweight = 1.0;
uniform vec2 reflectionScale;

// Fogging
uniform vec4 fog;
uniform vec2 fogSettings;
out vec4 FragColor;

void main(void) {
    vec2 coords  = vec2(tex_coord[0]*scale[0], tex_coord[1]*scale[1]);
    vec4 texel0  = texture(baseTexture, coords);
    if(texel0.a < 0.05){
        discard;
    }

    vec4 texel1  = texture(detailMap, coords*scale[2]);
    vec4 texel2  = texture(multipurposeMap, coords);
    vec4 white   = vec4(1.0,1.0,1.0,1.0);
    vec4 detail  = mix(white, texel1*2, maps[1]);
    FragColor = texel0 * detail;
   
    // CUBEMAPPING
    if ( maps[0] > 0 && maps[2] > 0 ) {
        vec4 reflex = vec4(0.0, 0.0, 0.0, 1.0);
        vec3 eye = normalize(eyeVec);
        eye = vec3(eye.x * 0.5, eye.y * 0.5, eye.z);
        vec2 offsetdir = vec2( eye.x, eye.y );
        float dist = length(lightVec);
        vec3 light = normalize(lightVec);
        float attenuation = 1.0; // / (gl_LightSource[0].constantAttenuation
                                 // + gl_LightSource[0].linearAttenuation * dist
                                 // + gl_LightSource[0].quadraticAttenuation * dist * dist);
        
        // Query the Maps
        vec3 color = vec3(0.0,0.0,0.0);
        vec3 norm = normalize(normals);
        if ( maps[0] > 0 ) {
            norm = texture(multipurposeMap, coords).rgb - 0.5;
            norm = vec3(norm.x * normalweight, norm.y * normalweight, norm.z);
        }
        
        vec3 refl = reflect(norm, eye);  // in tangent space !
        vec3 reflw = vec3( 1.0, -1.0, 1.0) * (TBNMatrix * refl);
        reflex = texture(cubeTextureMap, reflw);
        
        float oneMinusDot = 1.0 - dot(normalize(normals), normalize(eye));
        float scale = mix(reflectionScale[0], reflectionScale[1], 0.0);
        FragColor = FragColor * mix(white, reflex*2, min(texel0.a, scale));
    }
    
    // FOGGING
    float z = (gl_FragCoord.z / gl_FragCoord.w);
    float fogFactor = (z/fogSettings[0]);
    fogFactor = clamp(fogFactor, 0.0, fogSettings[1]);
    FragColor = mix(FragColor, vec4(fog[0],fog[1],fog[2],1.0), fogFactor);
}