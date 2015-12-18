#version 120
uniform sampler2D tDiffuse;
uniform sampler2D tPosition;
uniform sampler2D tNormals;
uniform sampler2D tDepth;
uniform sampler2D tSSAO;
uniform sampler2D tReflection;
uniform vec3 cameraPosition;
uniform mat4 ProjectionMatrix;
uniform vec3 UBOCamera;

// Fogging
uniform vec4 fog;
uniform vec2 fogSettings;

#define SIGMA 10.0
#define BSIGMA 0.1
#define MSIZE 15
float getDepth(vec2 sample) {
    return 0.5;
}

uniform float zNear = 0.1;
uniform float zFar = 200.0;

float linearDepth(float depthSample)
{
    depthSample = 2.0 * depthSample - 1.0;
    float zLinear = 2.0 * zNear * zFar / (zFar + zNear - depthSample * (zFar - zNear));
    return zLinear;
}

/*
 * Raytracing to get reflected color
 */
const float rayStep = 0.50;
const float minRayStep = 1.0;
const float maxSteps = 30;
const float searchDist = 50;
const float searchDistInv = 0.2;
const int numBinarySearchSteps = 20;
const float maxDDepth = 1.0;
const float maxDDepthInv = 1.0;

const float reflectionSpecularFalloffExponent = 3.0;

vec3 raytrace(in vec3 reflectionVector, in float startDepth)
{
    vec3 color = vec3(0.0f);
    float stepSize = rayStep;
    
    float size = length(reflectionVector.xy);
    reflectionVector = normalize(reflectionVector/size);
    reflectionVector = reflectionVector * stepSize;
    
    // Current sampling position is at current fragment
    vec2 sampledPosition = gl_TexCoord[0].xy;
    
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
            if(delta < 0.05f )
            {
                color = texture2D(tDiffuse, sampledPosition).rgb;
                break;
            }
        }
    }
    
    return color;
}

vec3 BinarySearch(vec3 dir, inout vec3 hitCoord, out float dDepth)
{
    float depth;
    for(int i = 0; i < numBinarySearchSteps; i++)
    {
        vec4 projectedCoord = ProjectionMatrix * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
        depth = texture2D(tPosition, projectedCoord.xy).z;
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
        depth = texture2D(tPosition, projectedCoord.xy).z;
        dDepth = hitCoord.z - depth;
        if(dDepth < 0.0) {
            return vec4(BinarySearch(dir, hitCoord, dDepth), 1.0);
        }
    }
    return vec4(1.0, 0.0, 0.0, 0.0);
}

vec4 rainbow(float x) {
    float level = x * 2.0;
    float r, g, b;
    if (level <= 0) {
        r = g = b = 0;
    } else if (level <= 1) {
        r = mix(1, 0, level);
        g = mix(0, 1, level);
        b = 0;
    } else if (level > 1) {
        r = 0;
        g = mix(1, 0, level-1);
        b = mix(0, 1, level-1);
    }
    return vec4(r, g, b, 1);
}


void main( void )
{
    vec4 image    = texture2D( tDiffuse, gl_TexCoord[0].xy );
    vec4 position = texture2D( tPosition, gl_TexCoord[0].xy );
    vec4 normal   = texture2D( tNormals, gl_TexCoord[0].xy );
    vec4 depth    = texture2D( tDepth, gl_TexCoord[0].xy );
    vec3 ssao     = texture2D( tSSAO, gl_TexCoord[0].xy ).xyz;
    vec3 refl     = texture2D( tReflection, gl_TexCoord[0].xy ).xyz;
    
    vec4 outColor;
    if (refl.r == 1.0) {
        vec2 gTexCoord = gl_TexCoord[0].xy;
        vec4 viewNormal = texture2D(tNormals, gTexCoord);
        vec4 viewPos = texture2D(tPosition, gTexCoord);

        float depth = linearDepth(texture2D(tDepth, gTexCoord).r);
        vec3 view = vec3(gl_TexCoord[0].xy, depth);
        vec3 norm = normalize(viewPos.xyz);
        vec4 projected = ProjectionMatrix * normalize(vec4(norm.x, norm.y*0.5 + 0.5, norm.z, 0.0));
        vec3 reflected = normalize(reflect(projected.xyz, normalize(viewNormal.xyz)));

        float dDepth;
        vec3 hitPos = viewPos.xyz;
        vec4 coords = RayCast(reflected.xyz, hitPos, dDepth);
        vec2 dCoords = abs(vec2(0.5, 0.5) - coords.xy);
        float screenEdgefactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);
        
        
        vec3 reflectedColor = texture2D(tDiffuse, coords.xy).xyz;
        outColor = mix(image, vec4(reflectedColor.xyz, 1.0), min(0.3, clamp(-reflected.z, 0.0, 1.0) * coords.w)); //clamp(screenEdgefactor * clamp(-reflected.z, 0.0, 1.0) *
                           //clamp((searchDist - length(viewPos.xyz - hitPos)) * searchDistInv, 0.0, 1.0) * coords.w - 0.3, 0.0, 1.0));
    } else {
        vec3 light = vec3(0,0,50);
        vec3 lightDir = light - position.xyz ;
        
        lightDir = normalize(lightDir);
        vec3 eyeDir = normalize(cameraPosition-position.xyz);
        vec3 vHalfVector = normalize(lightDir.xyz+eyeDir);
        vec2 resolution = vec2(4096.0, 2048.0);
        vec2 uv = gl_FragCoord.xy / resolution.xy;
        
        const float yblur = 2048;
        const float blursize = 1.0/yblur;
        vec2 vTexCoord = uv;//vec2(gl_TexCoord[0].x, gl_TexCoord[0].y);
        vec4 sum = vec4(0.0);
        sum += texture2D(tSSAO, vec2(vTexCoord.x - 4.0*blursize, vTexCoord.y)) * 0.05;
        sum += texture2D(tSSAO, vec2(vTexCoord.x - 3.0*blursize, vTexCoord.y)) * 0.09;
        sum += texture2D(tSSAO, vec2(vTexCoord.x - 2.0*blursize, vTexCoord.y)) * 0.12;
        sum += texture2D(tSSAO, vec2(vTexCoord.x - blursize, vTexCoord.y)) * 0.15;
        sum += texture2D(tSSAO, vec2(vTexCoord.x, vTexCoord.y)) * 0.16;
        sum += texture2D(tSSAO, vec2(vTexCoord.x + blursize, vTexCoord.y)) * 0.15;
        sum += texture2D(tSSAO, vec2(vTexCoord.x + 2.0*blursize, vTexCoord.y)) * 0.12;
        sum += texture2D(tSSAO, vec2(vTexCoord.x + 3.0*blursize, vTexCoord.y)) * 0.09;
        sum += texture2D(tSSAO, vec2(vTexCoord.x + 4.0*blursize, vTexCoord.y)) * 0.05;
        outColor = sum.x*image; //vec4(mix(image / 2, image, sum.x).xyz, image.a);
    }
    
    //Fog
    const float LOG2 = 1.442695;
    float posZ = ((position.z / position.w));
    float fogFactor = exp2(posZ * LOG2 * 8 / fogSettings[0]);
    fogFactor = fogSettings[1] - clamp(fogFactor, 0.0, fogSettings[1]);
    outColor = mix(outColor * 1.2, vec4(fog[0],fog[1],fog[2], 1.0), fogFactor);
    gl_FragColor = outColor;
}













// JUNK



//gl_FragColor = vec4(normal);
/*
 const int NUM_TAPS = 12;
 float max_siz = texture2D( tDepth, gl_TexCoord[0].xy ).x;// * (0.5+0.5*sin(iGlobalTime));
 
 vec2 fTaps_Poisson[NUM_TAPS];
 fTaps_Poisson[0]  = vec2(-.326,-.406);
 fTaps_Poisson[1]  = vec2(-.840,-.074);
 fTaps_Poisson[2]  = vec2(-.696, .457);
 fTaps_Poisson[3]  = vec2(-.203, .621);
 fTaps_Poisson[4]  = vec2( .962,-.195);
 fTaps_Poisson[5]  = vec2( .473,-.480);
 fTaps_Poisson[6]  = vec2( .519, .767);
 fTaps_Poisson[7]  = vec2( .185,-.893);
 fTaps_Poisson[8]  = vec2( .507, .064);
 fTaps_Poisson[9]  = vec2( .896, .412);
 fTaps_Poisson[10] = vec2(-.322,-.933);
 fTaps_Poisson[11] = vec2(-.792,-.598);
 
 sum = vec4(0);
 float rnd = 6.28 * nrand( uv);
 
 vec4 basis = vec4( rot2d(vec2(1,0),rnd), rot2d(vec2(0,1),rnd) );
 for (int i=0; i < NUM_TAPS; i++)
 {
 vec2 ofs = fTaps_Poisson[i]; ofs = vec2(dot(ofs,basis.xz),dot(ofs,basis.yw) );
 //vec2 ofs = rot2d( fTaps_Poisson[i], rnd );
 vec2 texcoord = uv + max_siz * ofs / resolution.xy;
 sum += texture2D(tDiffuse, texcoord, -10.0);
 }
 
 image = (sum / vec4(NUM_TAPS));
 gl_FragColor = image;
 */




//gl_FragColor = vec4(reflectedColor, 1.0f);



//vec4 cameraRay = vec4( gl_TexCoord[0].xy, 0.1, 1.0);
//vec4 pixelLocation = ProjectionMatrix * cameraRay;


//float specular = 0.1;

/*
 vec2 screen = gl_TexCoord[0].xy ;
 vec3 worldStartingPos = texture2D(tPosition, screen).xyz;
 vec3 normal = texture2D(tNormals, screen).xyz;
 vec3 cameraToWorld = worldStartingPos.xyz - UBOCamera.xyz;
 float cameraToWorldDist = length(cameraToWorld);
 float scaleNormal = max(3.0, cameraToWorldDist*1.5);
 vec3 cameraToWorldNorm = normalize(cameraToWorld);
 vec3 refl = normalize(reflect(cameraToWorldNorm, normal));
 */

//vec3 viewNormal = texture2D(tNormals, gl_TexCoord[0].xy).xyz;
//vec3 viewPos = texture2D(tPosition, gl_TexCoord[0].xy).xyz;


//vec4 viewDir = vec4(0.0, 0.0, 1.0, 1.0);
//viewDir /= viewDir.w;

// Pss = [texcoordxy, depth]



//float depth = texture2D(tDepth, gl_TexCoord[0].xy).z;

//
//vec4 Pss    = vec4( gl_TexCoord[0].xy, depth, 0.0);
//vec4 Pcs = (reflect(texture2D(tPosition, gl_TexCoord[0].xy), texture2D(tNormals, gl_TexCoord[0].xy)));
//vec4 Psss = (Pcs / Pcs.w) * vec4(0.5, -0.5, 1.0, 1.0) + vec4(0.5, 0.5, 0.0, 0.0);
//vec3 incident = Psss.xyz - Pss.xyz;
//vec3 reflected = Pcs.xyz;//normalize(reflect(normalize(incident), normalize(viewNormal)));

/*
 vec2 screen = gl_TexCoord[0].xy ;
 vec3 worldStartingPos = texture2D(tPosition, screen).xyz;
 vec3 normal = texture2D(tNormals, screen).xyz;
 vec3 cameraToWorld = worldStartingPos.xyz - UBOCamera.xyz;
 float cameraToWorldDist = length(cameraToWorld);
 float scaleNormal = max(3.0, cameraToWorldDist*1.5);
 vec3 cameraToWorldNorm = normalize(cameraToWorld);
 vec3 refl = normalize(reflect(cameraToWorldNorm, normal));
 reflected = refl;
 */

/*
 vec3 Pss    = vec3( gl_TexCoord[0].xy, depth);
 vec4 Pcs = reflect(texture2D(tPosition, gl_TexCoord[0].xy), texture2D(tNormals, gl_TexCoord[0].xy)) * ProjectionMatrix;
 vec4 Psss = (Pcs / Pcs.w) * vec4(0.5, -0.5, 1.0, 1.0) + vec4(0.5, 0.5, 0.0, 0.0);
 vec3 incident = Psss.xyz - Pss.xyz;
 incident = vec3(viewPos.x / 100, viewPos.y, viewPos.z);
 vec3 reflected = normalize(reflect(normalize(incident), normalize(viewNormal)));
 */


//vec3 vspPosReflect = viewPos +  reflected;
//vec4 unknown = vec4(vspPosReflect, 1.0) * ProjectionMatrix;
//vec3 sspPosReflect = .xyz / vspPosReflect.z;
//vec3 sspReflect = sspPosReflect - Input.ScreenPos;




//vec3 reflectedColor = vec3(0.0f);
//vec2 vert_UV = gl_TexCoord[0].xy;
//vec3 normal = normalize( texture2D(tNormals, vert_UV) ).xyz;
//float currDepth = linearDepth( texture2D(tDepth, vert_UV).z );


//vec4 cpos1        = vec4( gl_TexCoord[0].xy, depth, 1.0);
//vec4 cpos2        = ProjectionMatrix * vec4(UBOCamera, 1.0);
//vec4 cpos3 = cpos2 - cpos1;
//vec4 reflectionVector = reflect( vec4(normalize(viewPos), 0), vec4(normal, 0) ) ;

/*
 vec4 Pvs = vec4(0,0,0,0);
 vec4 Vvs = vec4(0,0,-1,0);
 vec4 Nvs = texture2D(tNormals, gl_TexCoord[0].xy);
 
 vec3 Pss    = vec3( gl_TexCoord[0].xy, depth);
 vec4 Pcs = (Pvs + reflect(Vvs, Nvs)) * ProjectionMatrix;
 vec4 Psss = (Pcs / Pcs.w) * vec4(0.5, -0.5, 1.0, 1.0) + vec4(0.5, 0.5, 0.0, 0.0);
 vec3 incident = Psss.xyz - Pss.xyz;
 
 vec3 reflectionVector = normalize(reflect(normalize(incident), normalize(viewNormal)));
 
 
 // Ray cast
 vec3 hitPos = viewPos;
 float dDepth;
 
 vec4 coords = RayCast(reflectionVector.xyz * max(minRayStep, -viewPos.z), hitPos, dDepth);
 vec2 dCoords = abs(vec2(0.5, 0.5) - coords.xy);
 float screenEdgefactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);
 
 
 // Get color
 vec4 reflectionColor = vec4(texture2D(tDiffuse, coords.xy).rgb,
 pow(specular, reflectionSpecularFalloffExponent) *
 screenEdgefactor * clamp(-reflectionVector.z, 0.0, 1.0) *
 clamp((searchDist - length(viewPos - hitPos)) * searchDistInv, 0.0, 1.0) * coords.w);
 
 
 
 
 gl_FragColor = vec4(reflectionVector.xyz, 1.0); //mix(, reflectionColor, 0.0);
 */



//gl_FragColor = vec4(raytrace(reflected, linearDepth(texture2D(tDepth, gl_TexCoord[0].xy).r)), 1.0);'

//gl_FragColor = vec4(incident, 1.0);





// Compute a camera ray
/*
 vec3 pixel  = vec3(gl_TexCoord[0].xy, texture2D(tDepth, gl_TexCoord[0].xy).r); // -1 to 1. 0 is center
 vec3 camera = vec3(0,0,0);
 vec3 ray    = pixel - camera;
 vec3 normal = texture2D(tNormals, gl_TexCoord[0].xy).xyz;
 vec3 reflection = reflect(normalize(ray), normalize(normal));
 */


/*
 float depth = texture2D(tDepth, gl_TexCoord[0].xy).z;
 vec3 Pss    = vec3( gl_TexCoord[0].xy, depth);
 vec4 Pcs = reflect(texture2D(tPosition, gl_TexCoord[0].xy), texture2D(tNormals, gl_TexCoord[0].xy)) * ProjectionMatrix;
 vec4 Psss = (Pcs / Pcs.w) * vec4(0.5, -0.5, 1.0, 1.0) + vec4(0.5, 0.5, 0.0, 0.0);
 vec3 incident = Psss.xyz - Pss.xyz;
 incident = vec3(viewPos.x / 100, viewPos.y, viewPos.z);
 vec3 reflected = normalize(reflect(normalize(incident), normalize(viewNormal.xyz)));
 */

/*
 vec2 vert_UV = gl_TexCoord[0].xy;
 vec3 reflectedColor = vec3(0.0f);
 vec3 normal = normalize(texture2D(tNormals, vert_UV)).xyz;
 float currDepth = linearDepth( texture2D(tPosition, vert_UV).z);
 
 // Eye position, camera is at (0, 0, 0), we look along negative z, add near plane to correct parallax
 vec3 eyePosition = normalize( vec3(0, 0, 1) );
 vec4 reflectionVector = normalize(reflect( normalize(texture2D(tPosition, vert_UV)), vec4(normal, 0)));
 */

// Call raytrace to get reflected color
//reflectedColor = raytrace(reflectionVector.xyz, currDepth);

