#version 450 core

#define MAX_LIGHTS_PER_TILE 128

in VERTEX_OUT{
    vec2 frag_uv;
    mat3 TBN;
    vec3 ts_frag_pos;
    vec3 ts_view_pos;
} fragment_in;

struct PointLight {
	vec4 position;
	vec4 color;
	vec4 paddingAndRadius;
};

// Shader storage buffer objects
layout(std430, binding = 0) buffer LightBuffer {
	PointLight data[];
} lightBuffer;

layout(std430, binding = 1) buffer visible_lights_indices {
	int lights_indices[];
};

// Uniforms
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;

uniform int doLightDebug;
uniform int numberOfTilesX;

out vec4 fragColor;

void main() {
	// Determine which tile this pixel belongs to
	ivec2 location = ivec2(gl_FragCoord.xy);
	ivec2 tileID = location / ivec2(16, 16);
	uint index = tileID.y * numberOfTilesX + tileID.x;
    uint offset = index * MAX_LIGHTS_PER_TILE;

    vec4 result = vec4(0.0, 0.0, 0.0, 1.0);

	vec4 base_diffuse = texture(texture_diffuse1, fragment_in.frag_uv);

	vec3 normal = texture(texture_normal1, fragment_in.frag_uv).rgb;
	normal = normalize(normal * 2.0 - 1.0);

	float specpower = 60.0f;

	for (uint i = 0; i < MAX_LIGHTS_PER_TILE; i++)
	{
		if (lights_indices[offset + i] != -1) 
		{
		    int indices = lights_indices[offset + i];

			PointLight light = lightBuffer.data[indices];

			vec3 ts_light_pos = fragment_in.TBN * vec3(light.position);
			vec3 ts_light_dir = normalize(ts_light_pos - fragment_in.ts_frag_pos);
			float dist = length(ts_light_pos - fragment_in.ts_frag_pos);
	
			vec3 N = normal;
			vec3 L = ts_light_dir;
	
			vec3 R = reflect(-L, N);
			float NdotR = max(0.0, dot(N, R));
			float NdotL = max(0.0, dot(N, L));
	
			float attenuation = clamp(1.0 - dist * dist / (light.paddingAndRadius.w * light.paddingAndRadius.w), 0.0, 1.0);
	
			vec3 diffuse_color  = 1.0 * vec3(light.color.x, light.color.y, light.color.z) * vec3(base_diffuse.r, base_diffuse.g, base_diffuse.b) * NdotL * attenuation;
			vec3 specular_color = vec3(1.0) * pow(NdotR, specpower) * attenuation;
	
			result += vec4(diffuse_color + specular_color, 0.0);	
		}
	}

	if (base_diffuse.a <= 0.2) {
		discard;
	}
	
	fragColor = result;
	
	if (doLightDebug==1){
		uint count;
		for (uint i = 0; i < MAX_LIGHTS_PER_TILE; i++) {
		    if (lights_indices[offset + i] != -1 ) {
			count++;
		    }
		}
		float shade = float(count) / float(MAX_LIGHTS_PER_TILE * 2); 
		fragColor = vec4(shade, shade, shade, 1.0);
	}
}
