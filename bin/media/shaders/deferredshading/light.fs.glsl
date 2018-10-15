#version 430

layout (location = 0) out vec4 color_out;

layout (binding = 0) uniform usampler2D gbuf_tex0;
layout (binding = 1) uniform sampler2D gbuf_tex1;

struct Light
{
    vec4    position;
    vec4    color;
	vec4    padding;
};

uniform int num_lights;


layout(std430, binding = 2) buffer LightBuffer 
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

void unpackGBuffer(ivec2 coord,
                   out fragment_info_t fragment)
{
    uvec4 data0 = texelFetch(gbuf_tex0, ivec2(coord), 0);
    vec4 data1 = texelFetch(gbuf_tex1, ivec2(coord), 0);
    vec2 temp;

    temp = unpackHalf2x16(data0.y);
    fragment.color = vec3(unpackHalf2x16(data0.x), temp.x);
    fragment.normal = normalize(vec3(temp.y, unpackHalf2x16(data0.z)));

    fragment.ws_coord = data1.xyz;
    fragment.specular_power = data1.w;
}

vec4 light_fragment(fragment_info_t fragment)
{
    int i;
    vec4 result = vec4(0.0, 0.0, 0.0, 1.0);


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

		float attenuation = clamp(1.0 - dist * dist / (5.0f * 5.0f), 0.0, 1.0);
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

    unpackGBuffer(ivec2(gl_FragCoord.xy), fragment);

    color_out = light_fragment(fragment);
}
