#version 330 core

// Input vertex data, different for all executions of this shader.
in vec3 vert;
in vec4 position; // Position of the center of the particule and size of the square
in vec2 uv;
uniform mat4 ModelView;
uniform mat4 Perspective;
uniform mat4 Model;

out vec2 texUV;
void main()
{
  float size= position.w; // because we encoded it this way.
  vec3 pos = position.xyz;

  vec3  cr = vec3(ModelView[0][0],ModelView[1][0],ModelView[2][0]);
  vec3  cu = vec3(ModelView[0][1],ModelView[1][1],ModelView[2][1]);
  vec3 finalPos = pos   + 
  				  cr * vert.x * size + 
                  cu * vert.y * size;
  texUV = uv;
  // Output position of the vertex
  gl_Position =  Perspective * ModelView * Model * vec4(finalPos,1.0);


}
