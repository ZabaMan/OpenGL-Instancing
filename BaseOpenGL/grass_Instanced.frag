#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in float Color;

uniform sampler2D texture1;


void main()
{             
    vec4 texColor = texture(texture1, TexCoords);
    if(texColor.a < 0.1)
        discard;
    float color = Color + 3;
    color = (((1.0 - 0.0)*(color - 0)) / 6) + 0.0;
    vec4 outColor = vec4(color, 1.0, clamp(1 - color, 0.0, 0.5), 1.0);
    FragColor = texColor * outColor;
}