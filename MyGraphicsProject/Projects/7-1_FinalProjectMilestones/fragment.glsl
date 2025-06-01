#version 330 core

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragTexture;

uniform sampler2D floorTexture;
uniform sampler2D stoneTexture;
uniform sampler2D crackTexture;
uniform sampler2D lampBaseTexture;
uniform sampler2D lampFlameTexture;
uniform sampler2D lanternSupportTexture;
uniform sampler2D grassTexture;
uniform sampler2D plankTexture;
uniform sampler2D supportTexture;
uniform sampler2D groundSupportTexture;
uniform sampler2D dirtTexture;
uniform sampler2D toriiTexture;
uniform sampler2D toriiRoofTexture;
uniform sampler2D shrineRoofTexture;
uniform sampler2D shrineWallTexture;
uniform sampler2D kanjiTexture;

uniform int object;
uniform vec4 objectColor;
uniform bool useTexture;
uniform vec3 viewPos; 
uniform float time;
uniform bool uHighlight;

void main()
{
    vec4 finalTexture;

    if (useTexture) {
        if (object == 0) {
            finalTexture = texture(floorTexture, TexCoords);
        } 
        else if (object == 1) {
            vec4 stoneColor = texture(stoneTexture, TexCoords);
            vec4 crackColor = texture(crackTexture, TexCoords);
            finalTexture = mix(stoneColor, crackColor, 0.05);
        } 
        else if (object == 2) {
            finalTexture = texture(lanternSupportTexture, TexCoords);
        } 
        else if (object == 3) {
            finalTexture = texture(lampBaseTexture, TexCoords);
        } 
        else if (object == 4) {
            finalTexture = texture(lampFlameTexture, TexCoords);
            vec3 emissive = vec3(1.0, 0.8, 0.4);
            finalTexture.rgb += emissive * 1.5;
        } 
        else if (object == 5) {
            finalTexture = texture(grassTexture, TexCoords);
        } 
        else if (object == 6) {
            finalTexture = texture(plankTexture, TexCoords);
        }
        else if (object == 7) {
            finalTexture = texture(supportTexture, TexCoords);
        }
        else if (object == 8) {
            finalTexture = texture(groundSupportTexture, TexCoords);
        }
        else if (object == 9) {
            finalTexture = texture(dirtTexture, TexCoords);
        }
        else if (object == 10){
            finalTexture = texture(toriiTexture, TexCoords);
        }
        else if (object == 11){
            finalTexture = texture(toriiRoofTexture, TexCoords);
        }
        else if (object == 12){
            finalTexture = texture(shrineRoofTexture, TexCoords);
        }
        else if (object == 13){
            finalTexture = texture(shrineWallTexture, TexCoords);
        }
        else if (object == 14){
            finalTexture = texture(kanjiTexture, TexCoords);
        }
    } else {
        finalTexture = objectColor;
    }

    if (uHighlight) {
        finalTexture = vec4(1.0);  // full white, ignore texture
    }

    vec3 ambient = vec3(0.2, 0.1, 0.2) * finalTexture.rgb;

    vec3 pointLightPos[8];
    pointLightPos[0] = vec3(0.0, 6.0, 0.0);
    pointLightPos[1] = vec3(0.0, 6.0, 12.0);
    pointLightPos[2] = vec3(0.0, 6.0, 24.0);
    pointLightPos[3] = vec3(0.0, 6.0, 36.0);
    pointLightPos[4] = vec3(18.0, 6.0, 0.0);
    pointLightPos[5] = vec3(18.0, 6.0, 36.0);
    pointLightPos[6] = vec3(18.75, 4.80, 15.5);
    pointLightPos[7] = vec3(18.75, 4.80, 20.5);

    vec3 pointLightDiffuse = vec3(1.0, 0.6, 0.3);
    vec3 pointLightSpecular = vec3(1.0, 0.9, 0.5);

    float flicker[8];
    flicker[0] = 0.5 + 0.3 * sin(time * 3.0);
    flicker[1] = 0.5 + 0.3 * sin(time * 3.0 + 1.0);
    flicker[2] = 0.5 + 0.3 * sin(time * 3.0 + 2.0);
    flicker[3] = 0.5 + 0.3 * sin(time * 3.0 + 3.0);
    flicker[4] = 0.5 + 0.3 * sin(time * 3.0);
    flicker[5] = 0.5 + 0.3 * sin(time * 3.0 + 3.0);
    flicker[6] = 1.0;
    flicker[7] = 1.0;

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lighting = ambient;

    vec3 dirLightDir = normalize(vec3(1.0, -0.5, -1.0));
    vec3 dirLightDiffuse = vec3(0.8, 0.4, 0.3);
    vec3 dirLightSpecular = vec3(1.0, 1.0, 1.0);

    float dirDiff = max(dot(norm, dirLightDir), 0.0);
    vec3 dirDiffuse = dirDiff * dirLightDiffuse * finalTexture.rgb;
    
    vec3 dirReflectDir = reflect(-dirLightDir, norm);
    float dirSpec = pow(max(dot(viewDir, dirReflectDir), 0.0), 16.0);
    vec3 dirSpecular = dirLightSpecular * dirSpec * finalTexture.rgb;

    for (int i = 0; i < 8; i++) {
        vec3 lightPos = pointLightPos[i];
        float flickerAmount = flicker[i];

        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * pointLightDiffuse * flickerAmount * finalTexture.rgb;

        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
        vec3 specular = pointLightSpecular * spec * flickerAmount * finalTexture.rgb;

        float distance = length(lightPos - FragPos);
        float attenuation = 1.0 / (1.0 + 0.1 * distance + 0.05 * (distance * distance));

        lighting += (diffuse + specular) * attenuation;
    }

    lighting += dirDiffuse + dirSpecular;

    FragTexture = vec4(clamp(lighting, 0.0, 1.0), finalTexture.a);
}
