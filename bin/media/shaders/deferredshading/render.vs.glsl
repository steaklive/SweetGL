#version 420

layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord0;
layout (location = 4) in vec3 tangent;

out VS_OUT
{
    vec3        ws_coords;
    vec3        normal;
    vec3        tangent;
    vec2        texcoord0;

} vs_out;

layout (std140) uniform transforms
{
    mat4        projection_matrix;
    mat4        view_matrix;
    mat4        model_matrix;
};

void main(void)
{

    mat4 mv_matrix = view_matrix * model_matrix;

    gl_Position = projection_matrix * mv_matrix * position;
    vs_out.ws_coords = (model_matrix * position).xyz;
    vs_out.normal = mat3(model_matrix) * normal;
    vs_out.tangent = tangent;
    vs_out.texcoord0 = texcoord0;

}
