#version 430

layout (location = 0) out vec4 color_out;

layout (location = 1) uniform sampler2D gPosition;
layout (location = 2) uniform sampler2D gNormal;
layout (location = 3) uniform sampler2D gAlbedo;

struct Light
{
    vec4    position;
    vec4    color;
	vec4    padding;
};

uniform int num_lights;

in vec2 TexCoords;

layout(std430, binding = 5) buffer LightBuffer 
{
	Light data[];
} lightBuffer;

struct fragment_info_t
{
    vec3 color;
    vec3 normal;
    float specular_power;
    vec3 ws_coord;
};

void unpackGBuffer(vec2 coord,
                   out fragment_info_t fragment)
{
    fragment.ws_coord   = texture(gPosition, coord).rgb;
    fragment.normal		= texture(gNormal, coord).rgb;
    fragment.color		= texture(gAlbedo, coord).rgb;

    fragment.specular_power = 60.0f;
}

vec4 light_fragment(fragment_info_t fragment)
{
    int i;
    vec4 result = vec4(0.0f, 0.0f, 0.0f, 1.0);


    for (i = 0; i < num_lights; i++)
    {
	    Light light = lightBuffer.data[i];
        vec3 L = vec3(light.position.x, light.position.y, light.position.z) - fragment.ws_coord;
        float dist = length(L);
        L = normalize(L);
        vec3 N = normalize(fragment.normal);
        vec3 R = reflect(-L, N);
        float NdotR = max(0.0, dot(N, R));
        float NdotL = max(0.0, dot(N, L));
	
		float attenuation = clamp(1.0 - dist * dist / (15.0f * 15.0f), 0.0, 1.0);
        //float attenuation = 50.0 / (pow(dist, 2.0) + 1.0);
	
        vec3 diffuse_color  = 1.0 * vec3(light.color.x, light.color.y, light.color.z) * fragment.color * NdotL * attenuation;
        vec3 specular_color = vec3(1.0) /* * light[i].color */* pow(NdotR, fragment.specular_power) * attenuation;
	
        result += vec4(diffuse_color + specular_color, 0.0);
    }

    return result;
}

void main(void)
{
    fragment_info_t fragment;

    unpackGBuffer(TexCoords, fragment);

    color_out = light_fragment(fragment);
}
