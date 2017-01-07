

#version 330 core
in vec3 position;

uniform mat4 Pers;
uniform mat4 View;
uniform mat4 Model;
void main() {
   gl_Position = Pers * View * Model * vec4(position,1.0f); 
}  