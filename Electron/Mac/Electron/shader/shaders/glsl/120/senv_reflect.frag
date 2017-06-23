#version 120
uniform sampler2D baseTexture;
uniform sampler2D primaryDetailMap;
uniform sampler2D secondaryDetailMap;
uniform sampler2D lightMap;
uniform samplerCube cubeTextureMap;
uniform sampler2D bumpMap;

uniform vec2 baseMapUV;
uniform vec4 maps; //0 = hasPrimary, 1 = primaryScale, 2 = hasSecondary, 3 = secondaryScale
uniform vec4 maps2;  //0 = useLight, //1 = blend
uniform float maps3;  //0 = useLight, //1 = blend

uniform vec2 reflectionScale;
uniform vec2 scale;
varying vec2 tex_coord;
varying vec2 tex_coord_light;
varying vec3 normals;
varying vec4 position;

// Cubemapping
varying vec3 lightVec, eyeVec;
uniform int togglebump; // false/true
uniform int textureon; // false/true
uniform float normalweight = 1.0;
varying mat3 TBNMatrix;

// Fog
uniform vec4 fog;
uniform vec2 fogSettings;

// Reflect
uniform sampler2D tDiffuse;
uniform sampler2D tPosition;
uniform sampler2D tNormals;
uniform sampler2D tDepth;




// Consts should help improve performance
const float rayStep = 0.25;
const float minRayStep = 0.1;
const float maxSteps = 1;
const float searchDist = 5;
const float searchDistInv = 0.2;
const int numBinarySearchSteps = 0;
const float maxDDepth = 1.0;
const float maxDDepthInv = 1.0;

const float reflectionSpecularFalloffExponent = 3.0;
uniform mat4 ProjectionMatrix;

uniform float zNear = 0.1;
uniform float zFar = 500.0;

// depthSample from depthTexture.r, for instance
float linearDepth(float depthSample)
{
    depthSample = 2.0 * depthSample - 1.0;
    float zLinear = 2.0 * zNear * zFar / (zFar + zNear - depthSample * (zFar - zNear));
    return zLinear;
}

vec3 BinarySearch(vec3 dir, inout vec3 hitCoord, out float dDepth)
{
    float depth;
    for(int i = 0; i < numBinarySearchSteps; i++)
    {
        vec4 projectedCoord = ProjectionMatrix * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
        depth = linearDepth(texture2D(tDepth, projectedCoord.xy).r);
        dDepth = hitCoord.z - depth;
        if(dDepth > 0.0) {
            hitCoord += dir;
        }

        dir *= 0.5;
        hitCoord -= dir;
    }

    vec4 projectedCoord = ProjectionMatrix * vec4(hitCoord, 1.0);
    projectedCoord.xy /= projectedCoord.w;
    projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
    return vec3(projectedCoord.xy, depth);
}


vec4 RayCast(vec3 dir, inout vec3 hitCoord, out float dDepth)
{
    dir *= rayStep;
    float depth;
    for(int i = 0; i < maxSteps; i++)
    {
        hitCoord += dir;
        vec4 projectedCoord = ProjectionMatrix * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
        depth = linearDepth(texture2D(tDepth, projectedCoord.xy).x);
        dDepth = hitCoord.z - depth;
        if(dDepth < 0.0) {
            return vec4(BinarySearch(dir, hitCoord, dDepth), 1.0);
        }
    }
    return vec4(1.0, 0.0, 0.0, 0.0);
}

/*
 * Raytracing to get reflected color
 */
vec3 raytrace(in vec3 reflectionVector, in float startDepth)
{
    vec3 color = vec3(0.0f);
    float stepSize = rayStep;
    
    float size = length(reflectionVector.xy);
    reflectionVector = normalize(reflectionVector/size);
    reflectionVector = reflectionVector * stepSize;
    
    // Current sampling position is at current fragment
    vec2 sampledPosition = vec2(gl_FragCoord.x / 3000.0, gl_FragCoord.y / 1650.0);

    // Current depth at current fragment
    float currentDepth = startDepth;
    // The sampled depth at the current sampling position
    float sampledDepth = linearDepth( texture2D(tDepth, sampledPosition).z );
    
    // Raytrace as long as in texture space of depth buffer (between 0 and 1)
    while(sampledPosition.x <= 1.0 && sampledPosition.x >= 0.0 &&
          sampledPosition.y <= 1.0 && sampledPosition.y >= 0.0)
    {
        // Update sampling position by adding reflection vector's xy and y components
        sampledPosition = sampledPosition + reflectionVector.xy;
        // Updating depth values
        currentDepth = currentDepth + reflectionVector.z * startDepth;
        float sampledDepth = linearDepth( texture2D(tDepth, sampledPosition).z );
        
        // If current depth is greater than sampled depth of depth buffer, intersection is found
        if(currentDepth > sampledDepth)
        {
            // Delta is for stop the raytracing after the first intersection is found
            // Not using delta will create "repeating artifacts"
            float delta = (currentDepth - sampledDepth);
            if(delta < 0.5f )
            {
                color = texture2D(tDiffuse, sampledPosition).rgb;
                break;
            }
        }
    }
    
    return color;
}

void main(void) {
    
    
    //reflection vector
    float specular = 0.1;
    
    
    vec2 uv = vec2(gl_FragCoord.x / 3000.0, gl_FragCoord.y / 1650.0);
    vec3 viewPos    = normalize(eyeVec); // + eye.xyz;
    vec3 viewNormal = texture2D(tNormals, uv).xyz;//normalize(normals);
    float currDepth = linearDepth( texture2D(tDepth, uv).z );
    
    if (viewNormal.x != 0 || viewNormal.y != 0 || viewNormal.z < 1.0) {
        //discard;
    }
    
    vec4 reflected  = vec4(0.0); //vec4(viewNormal, 0.0); //ProjectionMatrix * reflect(vec4(viewPos, 0), vec4(viewNormal, 0));
    
    
    
    // Ray cast
    vec3 hitPos = viewPos;
    float dDepth;
    
    vec4 coords = RayCast(reflected.xyz * max(minRayStep, -viewPos.z), hitPos, dDepth);
    vec2 dCoords = abs(vec2(0.5, 0.5) - coords.xy);
    float screenEdgefactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);
    
    // Get color
    //gl_FragData[0] = vec4(raytrace(reflected.xyz, currDepth), 1.0);
    gl_FragData[0] = vec4(texture2D(tDiffuse, coords.xy).rgb,
                        pow(specular, reflectionSpecularFalloffExponent) *
                        screenEdgefactor * clamp(-reflected.z, 0.0, 1.0) *
                        clamp((searchDist - length(viewPos - hitPos)) * searchDistInv, 0.0, 1.0) * coords.w);
    //gl_FragData[0] = vec4(viewNormal, 1.0);

}