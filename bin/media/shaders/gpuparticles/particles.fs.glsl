#version 430

in vec2 ex_TexCoor;
out vec4 color;

uniform sampler2D texture;
uniform float time;

void main(void){
  vec4 outputColor = texture2D(texture, ex_TexCoor)*4;
  
  float red = 0.6, green = 0.3, blue = 0.8;
  
  red = cos(time*0.5) + 1.f;

  
  outputColor.x *=  red;
  outputColor.y *=  green;
  outputColor.z *=  blue;
  
  color = outputColor;
}