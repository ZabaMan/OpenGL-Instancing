#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D diffuseMap;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float specSize;
uniform float specStrength;
uniform float ambient;
uniform float textureScale;

void main()
{           
    vec3 color = texture(diffuseMap, fs_in.TexCoords * textureScale).rgb;
    // ambient
    vec3 ambient = ambient * color;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 normal = normalize(fs_in.Normal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), specStrength);
    vec3 specular = vec3(specSize) * spec; // assuming bright white light color
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}