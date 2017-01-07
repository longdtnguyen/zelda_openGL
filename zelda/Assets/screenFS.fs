#version 330 core
noperspective in vec2 texUV;

out vec4 fragColor;

uniform sampler2D screenTex;
uniform sampler1D lensColor;
uniform sampler2D brightTex;
uniform float weight[5] = float[] (0.000227027, 0.0001945946, 0.0001216216, 0.000054054, 0.000016216);
uniform int login;
uniform int showFlare;
//- gaussian blur all directions (8)
vec3 blur(vec2 u_v)
{ 
  vec2 tex_offset = vec2(1.0 / 300,1.0/300);
  vec3 result = texture(brightTex, u_v).rgb; 
  vec3 origin = result;
  if (result.x < 100) {
    result = vec3(0.0);
    origin = vec3(0.0);
  }else result/100.0;
  
  result *= weight[0];
  for(int i = 1; i < 5; ++i) {
    result += texture(brightTex, u_v + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
    result += texture(brightTex, u_v - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
    result += texture(brightTex, u_v + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
    result += texture(brightTex, u_v - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
    result += texture(brightTex, u_v + vec2(tex_offset.x * i, -tex_offset.y * i)).rgb * weight[i];
    result += texture(brightTex, u_v - vec2(tex_offset.x * i, -tex_offset.y * i)).rgb * weight[i];
    result += texture(brightTex, u_v + vec2(-tex_offset.x * i, tex_offset.y * i)).rgb * weight[i];
    result += texture(brightTex, u_v - vec2(-tex_offset.x * i, tex_offset.y * i)).rgb * weight[i];
    result += texture(brightTex, u_v + vec2(tex_offset.x * i, tex_offset.y * i)).rgb * weight[i];
    result += texture(brightTex, u_v - vec2(tex_offset.x * i, tex_offset.y * i)).rgb * weight[i];
    }   
   
    return result + origin;
} 


void main() {


  //---- original picture
  vec3 origin = texture(screenTex,texUV).rgb;
  // fragColor = origin;
  vec2 flippedUV = -texUV - vec2(1.0);

  vec2 texSize = 1.0 / vec2(textureSize(screenTex, 0));
 
  // ghost vector to image centre:
  vec2 ghostVec = (vec2(0.5) - flippedUV) * 0.5;

  // sample ghosts:  
  vec4 result = vec4(0.0);
  for (int i = 0; i < 5; i++) { 
    vec2 offset = fract(flippedUV + ghostVec * float(i));
    result += texture(screenTex, offset);
    //result += vec4(blur(offset),1.0);
  }

  result *= texture(lensColor,0.5);
  vec3 flare = vec3(max(result-10,vec4(0.0)));
   

  float alpha = (flare.x == 0) ? 1 :0.8;
  vec3 gblur = blur(texUV);

  vec4 retCol;

 
  if (login == 0) retCol =  vec4(gblur + flare + origin,alpha);
  else {
    vec2 flip = texUV;
    flip.y = 1 - flip.y;
    retCol = texture(screenTex,flip);
  }
  if (showFlare != 1 && login == 0) retCol = vec4(gblur + origin,alpha);

  fragColor = retCol;
 
} 