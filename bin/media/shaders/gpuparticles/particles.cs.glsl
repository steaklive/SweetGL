#version 430
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

struct particle{
  vec4  currPos;
  vec4  prevPos;
};

layout(std430, binding=0) buffer particles{
  particle p[];
};

uniform vec4 attPos;
uniform float frameTimeDiff;
uniform uint maxParticles = 10000;

layout (local_size_x = 256) in;

void main(){
  uint gid = gl_GlobalInvocationID.x;
  
  if(gid <= maxParticles)
  {
    particle part = p[gid];
    vec4 acceleration, tempCurrPos;

    acceleration = attPos - part.currPos - vec4(0.0, -1.0, 0, 0);
    acceleration = 50* normalize(acceleration) * length(part.currPos.xyz);

    tempCurrPos  = 1.99f * part.currPos - 0.99 * part.prevPos + acceleration * frameTimeDiff * frameTimeDiff;

    part.prevPos = part.currPos;
    part.currPos = tempCurrPos;
    
    p[gid] = part;
  }
}