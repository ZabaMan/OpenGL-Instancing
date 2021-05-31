#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in float Color;

uniform sampler2D texture1;


void main()
{             
    vec4 texColor = texture(texture1, TexCoords);
    if(texColor.a < 0.5)
        discard;
    FragColor = texColor * vec4(1.5, 1.5, 1.5, 1.0);
}