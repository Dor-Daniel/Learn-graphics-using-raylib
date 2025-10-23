#version 330

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;
in vec4 vertexColor;          // <-- read color from Draw* calls

uniform mat4 mvp;
uniform mat4 matModel;

out vec3 vWorldPos;
out vec3 vWorldNormal;
out vec2 vUV;
out vec4 vColor;              // pass to fragment

void main() {
    vec4 worldPos = matModel * vec4(vertexPosition, 1.0);
    vWorldPos    = worldPos.xyz;
    vWorldNormal = mat3(matModel) * vertexNormal;  // may be zero for some shapes
    vUV          = vertexTexCoord;
    vColor       = vertexColor;                    // <-- carry Draw* color
    gl_Position  = mvp * vec4(vertexPosition, 1.0);
}
