#version 430 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gMetalness;


in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec3 Tangent;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_specular1;



void main()
{    
	// pos
    gPosition = FragPos;

	vec3 N = normalize(Normal);
    vec3 T = normalize(Tangent);
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);

    vec3 nm = texture(texture_normal1, TexCoords).xyz * 2.0 - vec3(1.0);
    nm = TBN * normalize(nm);

	// normal
    gNormal = normalize(Normal);


    // albedo
    gAlbedo.rgb = texture(texture_diffuse1, TexCoords).rgb;

	// metalness
    gMetalness.r = texture(texture_specular1, TexCoords).r;



}