#include "DropBomb.h"

DropBomb::DropBomb()
{
}

DropBomb::~DropBomb()
{
}

void DropBomb::initialize(GLuint shaderID, GLuint texID, int nAnimations, int nFrames, vec3 pos, vec3 dimensions)
{
    Sprite::initialize(shaderID, texID, nAnimations, nFrames, pos, dimensions);
}
