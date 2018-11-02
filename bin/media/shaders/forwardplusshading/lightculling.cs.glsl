#version 450 core

#define MAX_LIGHTS_PER_TILE 128
#define TILE_SIZE 16

struct PointLight {
	vec4 position;
	vec4 color;
	vec4 paddingAndRadius;
};

// Shader storage buffer objects
layout(std430, binding = 0) readonly buffer LightBuffer {
	PointLight data[];
};

layout(std430, binding = 1) writeonly buffer visible_lights_indices {
	int lights_indices[];
};

// Uniforms
uniform sampler2D depthMap;
uniform mat4 view;
uniform mat4 projection;

uniform vec2 screenSize;
uniform int lightCount;

// Shared values between all the threads in the group
shared uint minDepthInt;
shared uint maxDepthInt;
shared uint visibleLightCount;
shared vec4 frustumPlanes[6];
// Shared local storage for visible indices, will be written out to the global buffer at the end
shared int visibleLightIndices[MAX_LIGHTS_PER_TILE];
//shared mat4 viewProjection;


layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE) in;
void main() {

	ivec2 tile_id = ivec2(gl_WorkGroupID.xy);

	if (gl_LocalInvocationIndex == 0) {
		minDepthInt = 0xFFFFFFFF;
		maxDepthInt = 0x0;
		visibleLightCount = 0;
		uint index = gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x;
		uint offset = index * MAX_LIGHTS_PER_TILE;
		for (uint i = 0; i < MAX_LIGHTS_PER_TILE; i++) {
			lights_indices[offset + i] = -1;
		}
	}
	barrier();

	// Compute depth min and max of the workgroup
	vec2 screen_uv = gl_GlobalInvocationID.xy / screenSize;

	float depth = texture(depthMap, screen_uv).r;

	uint depth_uint = floatBitsToUint(depth);
	atomicMin(minDepthInt, depth_uint);
	atomicMax(minDepthInt, depth_uint);

	barrier();

	// Compute Tile frustrum planes
	if (gl_LocalInvocationIndex == 0) {
		float min_group_depth = uintBitsToFloat(minDepthInt);
		float max_group_depth = uintBitsToFloat(maxDepthInt);

		vec4 vs_min_depth = (inverse(projection) * vec4(0.0, 0.0, (2.0 * min_group_depth - 1.0), 1.0));
		vec4 vs_max_depth = (inverse(projection) * vec4(0.0, 0.0, (2.0 * max_group_depth - 1.0), 1.0));
		vs_min_depth /= vs_min_depth.w;
		vs_max_depth /= vs_max_depth.w;
		min_group_depth = vs_min_depth.z;
		max_group_depth = vs_max_depth.z;

		vec2 tile_scale = vec2(screenSize) * (1.0 / float(2 * TILE_SIZE));
		vec2 tile_bias = tile_scale - vec2(gl_WorkGroupID.xy);

		vec4 col1 = vec4(-projection[0][0] * tile_scale.x, projection[0][1], tile_bias.x, projection[0][3]);
		vec4 col2 = vec4(projection[1][0], -projection[1][1] * tile_scale.y, tile_bias.y, projection[1][3]);
		vec4 col4 = vec4(projection[3][0], projection[3][1], -1.0, projection[3][3]);

		frustumPlanes[0] = col4 + col1;
		frustumPlanes[1] = col4 - col1;
		frustumPlanes[2] = col4 - col2;
		frustumPlanes[3] = col4 + col2;
		frustumPlanes[4] = vec4(0.0, 0.0, 1.0, -min_group_depth);
		frustumPlanes[5] = vec4(0.0, 0.0, -1.0, max_group_depth);
		for (uint i = 0; i < 4; i++) {
			frustumPlanes[i] *= 1.0f / length(frustumPlanes[i].xyz);
		}
	}

	barrier();

	// Cull lights
	uint thread_count = TILE_SIZE * TILE_SIZE;
	for (uint i = gl_LocalInvocationIndex; i < lightCount; i += thread_count) {
		PointLight light = data[i];
		vec4 vs_light_pos = view * vec4(light.position);

		if (visibleLightCount < MAX_LIGHTS_PER_TILE) {
			bool inFrustum = true;
			for (uint j = 0; j < 6 && inFrustum; j++) {
				float d = dot(frustumPlanes[j], vs_light_pos);
				inFrustum = (d >= -light.paddingAndRadius.w);
			}
			if (inFrustum) {
				uint id = atomicAdd(visibleLightCount, 1);
				visibleLightIndices[id] = int(i);
			}
		}
	}

	barrier();
	if (gl_LocalInvocationIndex == 0) {
		uint index = gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x;
		uint offset = index * MAX_LIGHTS_PER_TILE;
		for (uint i = 0; i < visibleLightCount; i++) {
			lights_indices[offset + i] = visibleLightIndices[i];
		}
	}
}
