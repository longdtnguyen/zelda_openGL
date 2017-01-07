#version 330 core
in vec2 position;
in vec2 uv;

noperspective out vec2 texUV;

void main() {
    gl_Position = vec4(position.x, position.y, 0.0f, 1.0f); 
   	texUV = uv;
}  