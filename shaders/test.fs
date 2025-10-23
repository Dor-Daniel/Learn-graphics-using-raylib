#version 330

in vec3 vWorldPos;
in vec3 vWorldNormal;
in vec2 vUV;
in vec4 vColor;                 // color from Draw* calls

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec3 viewPos;
uniform vec3 lightDir;          // direction from light toward scene
uniform vec3 lightColor;
uniform float ambient;
uniform float shininess;
uniform float specStrength;
uniform vec3 lightPos;

// Fallback: if vWorldNormal is missing (e.g. some immediate-mode shapes),
// compute a face normal from derivatives.
vec3 computeNormal() {
    vec3 N = normalize(vWorldNormal);
    if (length(N) < 1e-3) {
        vec3 dx = dFdx(vWorldPos);
        vec3 dy = dFdy(vWorldPos);
        N = normalize(cross(dx, dy));
    }
    if (!gl_FrontFacing) N = -N;
    return N;
}

vec3 calcDirLight(vec3 base, vec3 N) {
    vec3 L = normalize(-lightDir);
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse  = base * lightColor * NdotL;

    vec3 V = normalize(viewPos - vWorldPos);
    vec3 H = normalize(L + V);
    float NdotH = max(dot(N, H), 0.0);
    float spec   = pow(NdotH, shininess) * specStrength;
    vec3 specular = lightColor * spec;

    vec3 ambientTerm = base * ambient;
    return ambientTerm + diffuse + specular;
}

vec3 calcPointLight(vec3 base, vec3 N) {
    vec3 V  = normalize(viewPos - vWorldPos);
    vec3 LD = normalize(lightPos - vWorldPos);
    float diff = max(dot(N, LD), 0.0);

    vec3 RD   = reflect(-LD, N);
    float spec = pow(max(dot(V, RD), 0.0), shininess) * specStrength;

    float D   = length(lightPos - vWorldPos);
    float ATT = 1.0 / (1.0 + 0.09 * D + 0.032 * D * D);

    vec3 AM  = ambient * base;
    vec3 DIF = diff * base * vec3(1.0, 0.87, 0.65);
    vec3 SPE = spec * lightColor;
    return ATT * (AM + DIF + SPE);
}

void main() {
    // base color: texture * material tint * per-vertex color
    vec3 baseTex = texture(texture0, vUV).rgb;
    vec3 base    = baseTex * colDiffuse.rgb * vColor.rgb;

    vec3 N = computeNormal();
    vec3 color = calcDirLight(base, N) + calcPointLight(base, N);
    finalColor = vec4(color, colDiffuse.a * vColor.a);
}
