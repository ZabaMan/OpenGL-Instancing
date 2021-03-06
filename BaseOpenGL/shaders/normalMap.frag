#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float specSize;
uniform float specStrength;
uniform float ambient;
uniform float textureScale;
uniform float normalStrength;

void main()
{           
     // obtain normal from normal map in range [0,1]
    vec3 normal = texture(normalMap, fs_in.TexCoords * textureScale).rgb;
    // transform normal vector to range [-1,1]
    normal = normal * 2.0 - 1.0;  // this normal is in tangent space
    normal.xy *= normalStrength;
    normal = normalize(normal);
   
    // get diffuse color
    vec3 color = texture(diffuseMap, fs_in.TexCoords * textureScale).rgb;
    // ambient
    vec3 ambient = ambient * color;
    // diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), specStrength);

    vec3 specular = vec3(specSize) * spec;
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}