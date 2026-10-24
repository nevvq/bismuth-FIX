#extension GL_ARB_shading_language_include:require

#include "shared.h"

//// VERTEX ATTRIBUTES ////
// layout (location = 0) in vec2 a_positionOffset;
// layout (location = 1) in vec2 a_texCoord;
// layout (location = 2) in int  a_spriteIndex;

//// VARIABLES TO BE TRANSFERED TO THE FRAGMENT SHADER ////
out vec2 t_texCoord;
out vec4 t_color;

STORAGE_BUFFER(VERTEX_BUFFER_BINDING) VertexBuffer {
    ObjectVertex verticies[];
};

//// MAIN FUNCTION ////
void main() {
    uint vertexIndex = u_culledTriIndicies[gl_VertexID / 3] * 3 + gl_VertexID % 3;

    vec2 positionOffset = verticies[vertexIndex].positionOffset;
    vec2 texCoord       = verticies[vertexIndex].texCoord;
    uint spriteIndex    = verticies[vertexIndex].spriteIndex;

    vec2 vertexPos = RawSprites.position[spriteIndex] + RawSprites.localTransform[spriteIndex] * positionOffset;

    gl_Position = u_mvp * vec4(vertexPos, 0.0, 1.0);

    t_color    = RawSprites.color[spriteIndex];
    t_texCoord = texCoord;
}