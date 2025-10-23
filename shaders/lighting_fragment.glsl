#version 330

uniform vec3 blockColor;
uniform vec3 cameraPosition;

out vec4 FragColor;

in vec3 fragPosition;
in vec3 fragNormal;

void main(){
    vec3 lightPosition = vec3(-50, 500, -50);
    vec3 lightAmbient = vec3(1.0, 1.0, 1.0);
    vec3 lightDiffuse = vec3(1.0, 1.0, 1.0);
    vec3 lightDirection = normalize(fragPosition - lightPosition);
    vec3 lightSpecular = vec3(0.5, 0.5, 0.5);

    vec3 blockAmbient = vec3(0.4, 0.4 ,0.4);
    vec3 blockDiffuse = vec3(0.7, 0.7 ,0.7);
    vec3 blockSpecular = vec3(1.0, 1.0, 1.0);

    vec3 reflectDir = reflect(lightDirection ,fragNormal);
    vec3 viewDirection = normalize(cameraPosition - fragPosition);

    float diff = max(dot(fragNormal, -lightDirection), 0.0);
    float spec = pow(max(dot(viewDirection, reflectDir), 0.0), 64.0);

    vec3 ambient = blockAmbient * lightAmbient;
    vec3 diffuse = diff * lightDiffuse * blockDiffuse;
    vec3 specular = spec * lightSpecular * blockSpecular;

    FragColor = vec4((ambient + diffuse + specular) * blockColor, 1.0);
}