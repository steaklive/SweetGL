#version 450 core

layout (location = 0) in vec3 vert_pos;
layout (location = 1) in vec3 vert_normal;
layout (location = 2) in vec2 vert_uv;
layout (location = 3) in vec3 vert_tangent;

out VERTEX_OUT {
  vec2 frag_uv;
  mat3 TBN;
  vec3 ts_frag_pos;
  vec3 ts_view_pos;
} vertex_out;

// Uniforms
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 viewPosition;

void main() {
	gl_Position = projection * view* model * vec4(vert_pos, 1.0);
	vec3 frag_pos = vec3(model * vec4(vert_pos, 1.0));

	mat3 normal_matrix = transpose(inverse(mat3(model)));
	vec3 N = normalize(vec3(normal_matrix * vert_normal));
	vec3 T = normalize(vec3(normal_matrix * vert_tangent));
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);

	vertex_out.frag_uv = vert_uv;
	vertex_out.TBN = transpose(mat3(T, B, N));    
	vertex_out.ts_view_pos  = vertex_out.TBN * viewPosition;
	vertex_out.ts_frag_pos = vertex_out.TBN * frag_pos;
}
