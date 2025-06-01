#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragTexture;

uniform vec3 viewPos;
uniform float time;

void main()
{
    vec3 ambient = vec3(0.5, 0.5, 0.5); // Increase ambient light
    vec3 lighting = ambient;

    vec3 pointLightPositions[4] = {
        vec3(0.0, 6.0, 0.0),
        vec3(0.0, 6.0, 12.0),
        vec3(0.0, 6.0, 24.0),
        vec3(0.0, 6.0, 36.0)
    };

    vec3 pointLightDiffuse = vec3(1.0, 1.0, 1.0); // Set to full white
    vec3 pointLightSpecular = vec3(1.0, 1.0, 1.0); // Set to full white

    for (int i = 0; i < 4; i++) {
        vec3 lightPos = pointLightPositions[i];
        vec3 lightDir = normalize(lightPos - FragPos);
        vec3 norm = normalize(Normal);
        
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * pointLightDiffuse;

        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
        vec3 specular = pointLightSpecular * spec;

        float distance = length(lightPos - FragPos);
        float attenuation = 1.0 / (1.0 + 0.2 * distance + 0.1 * (distance * distance));

        lighting += (diffuse + specular) * attenuation;
    }

    FragTexture = vec4(clamp(lighting, 0.0, 1.0), 1.0);
}

