#version 430 core

uniform sampler2D gFinalImage;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDepth;
uniform sampler2D gMetalness;

//uniform mat4 invView;
uniform mat4 projection;
//uniform mat4 invprojection;
uniform mat4 view;

uniform float stepDist;
uniform float minRayStep;
uniform float reflectionSpecularFalloffExponent;
uniform int raycastSamples;
uniform int numBinarySearchSteps;
 

noperspective in vec2 TexCoords;

out vec4 outColor;


float Metallic;

vec3 CalcViewPositionFromDepth(vec2 TexCoord)
{
    // Combine UV & depth into XY & Z (NDC)
    vec3 rawPosition                = vec3(TexCoord, texture(gDepth, TexCoord).r);

    // Convert from (0, 1) range to (-1, 1)
    vec4 ScreenSpacePosition        = vec4( rawPosition * 2 - 1, 1);

    // Undo Perspective transformation to bring into view space
	
    vec4 ViewPosition               = inverse(projection) * ScreenSpacePosition;
	
    // Perform perspective divide and return
    return                          ViewPosition.xyz / ViewPosition.w;
}

vec3 BinarySearch(inout vec3 dir, inout vec3 hitCoord, inout float dDepth)
{
    float depth;

    vec4 projectedCoord;
 
    for(int i = 0; i < numBinarySearchSteps; i++)
    {

        projectedCoord = projection * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
 
        depth = CalcViewPositionFromDepth(projectedCoord.xy).z;
		depth = textureLod(gPosition, projectedCoord.xy, 2).z;
 
        dDepth = hitCoord.z - depth;

        dir *= 0.5;
        if(dDepth > 0.0)
            hitCoord += dir;
        else
            hitCoord -= dir;    
    }

        projectedCoord = projection * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
 
    return vec3(projectedCoord.xy, depth);
}


vec2 RayCast(vec3 dir, inout vec3 hitCoord, out float dDepth)
{
    dir *= stepDist;  

    for(int i = 0; i < raycastSamples; ++i) {
        hitCoord               += dir; 

        vec4 projectedCoord     = projection * vec4(hitCoord, 1.0);
        projectedCoord.xy      /= projectedCoord.w;
        projectedCoord.xy       = projectedCoord.xy * 0.5 + 0.5; 

        float depth             = CalcViewPositionFromDepth(projectedCoord.xy).z;  
        

        dDepth                  = hitCoord.z - depth; 

        if(dDepth < 0.0)
            return projectedCoord.xy;

    }

    return vec2(-1.0f);
}


vec4 RayMarch(vec3 dir, inout vec3 hitCoord, out float dDepth)
{

    dir *= stepDist;
 
 
    float depth;
    int steps;
    vec4 projectedCoord;

 
    for(int i = 0; i < raycastSamples; i++)
    {
        hitCoord += dir;
 
        projectedCoord = projection * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
 
        //depth = CalcViewPositionFromDepth(projectedCoord.xy).z;
		depth = textureLod(gPosition, projectedCoord.xy, 2).z;
        if(depth > 1000.0)
            continue;
 
        dDepth = hitCoord.z - depth;

        if((dir.z - dDepth) < 1.2)
        {
            if(dDepth <= 0.0)
            {   
                vec4 Result;
                Result = vec4(BinarySearch(dir, hitCoord, dDepth), 1.0);

                return Result;
            }
        }
        
        steps++;
    }
 
    
    return vec4(projectedCoord.xy, depth, 0.0);
}
void main()
{

    vec2 MetallicEmmissive = texture2D(gMetalness, TexCoords).rg;
    Metallic = MetallicEmmissive.r;

    if(Metallic < 0.01)
       discard;
 
    vec3 viewNormal = vec3(texture2D(gNormal, TexCoords));
    vec3 viewPos = CalcViewPositionFromDepth(TexCoords);
    vec3 albedo = texture(gFinalImage, TexCoords).rgb;

    float spec = 15.0f; //texture(ColorBuffer, TexCoords).w;


	vec3 reflected = normalize(reflect(normalize(viewPos), normalize(viewNormal)));

    vec3 hitPos = viewPos.xyz;
    float dDepth;


    vec2 coords = RayCast((/*vec3(jitt) + */reflected.xyz * max(minRayStep, -viewPos.z)), hitPos, dDepth);
    //vec4 coords = RayMarch((/*vec3(jitt) + */reflected.xyz * max(minRayStep, -viewPos.z)), hitPos, dDepth);


	vec2 dCoords = smoothstep(0.2, 0.6, abs(vec2(0.5, 0.5) - coords.xy));
 
 
    float screenEdgefactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);
	float ReflectionMultiplier = pow(Metallic, reflectionSpecularFalloffExponent) * 
                screenEdgefactor * 
                -reflected.z;
 
    // Get color
    //vec3 SSR = textureLod(gFinalImage, coords.xy, 0).rgb ;//* clamp(ReflectionMultiplier, 0.0, 0.9) * Fresnel;  
    vec3 SSR = texture(gFinalImage, coords.xy).rgb * clamp(ReflectionMultiplier, 0.0, 0.9);  
	//outColor = vec4(SSR, Metallic);
	
	if (coords != vec2(-1.0)) 
		outColor = vec4(SSR, Metallic);
	else outColor = vec4(texture(gFinalImage, TexCoords).rgb, 1.0f);
	//	//outColor = vec4(SSR, Metallic);
	//	outColor = mix(texture(gFinalImage, TexCoords) * clamp(ReflectionMultiplier, 0.0, 0.9), texture(gFinalImage, coords), Metallic) * clamp(ReflectionMultiplier, 0.0, 0.9);
	//else outColor = vec4(SSR, 1.0f);

}


