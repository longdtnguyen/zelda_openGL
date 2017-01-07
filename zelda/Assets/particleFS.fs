#version 330 core


// Ouput data
out vec4 color;
in vec2 texUV;
uniform sampler2D tex;
void main(){
    // Output color = color of the texture at the specified UV
    color = vec4(texture(tex,texUV).rgb,0.5);

}