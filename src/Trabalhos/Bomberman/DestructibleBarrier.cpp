#include "DestructibleBarrier.h"

DestructibleBarrier::DestructibleBarrier() {}
DestructibleBarrier::~DestructibleBarrier() {}

void DestructibleBarrier::initialize(float cellSize, GLuint shaderID, GLuint texID, vec3 pos)
{
    Sprite::initialize(shaderID, texID, 1, 1, pos, vec3(cellSize));
}
