#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform float textureScale;

void main()
{    
    FragColor = texture(texture_diffuse1, TexCoords * textureScale);
}