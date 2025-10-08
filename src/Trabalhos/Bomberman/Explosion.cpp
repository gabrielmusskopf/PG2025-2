#include "Explosion.h"

Explosion::Explosion()
{
}

Explosion::~Explosion()
{
}

void Explosion::initialize(GLuint shaderID, GLuint texID, int nAnimations, int nFrames, vec3 pos, vec3 dimensions, double duration)
{
    Sprite::initialize(shaderID, texID, nAnimations, nFrames, pos, dimensions);
    this->creationTime = glfwGetTime();
    this->duration = duration;
}

bool Explosion::hasExpired()
{
    return (glfwGetTime() - this->creationTime) > this->duration;
}
