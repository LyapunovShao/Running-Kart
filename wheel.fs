#version 400 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 wheelColor;

void main()
{
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(0.0, dot(norm, lightDir));
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient + diffuse ) * wheelColor;
    FragColor = vec4(result, 1.0);
}
