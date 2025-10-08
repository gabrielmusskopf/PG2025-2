#include "Bomb.h"

Bomb::Bomb()
{
    this->state = BombState::TICKING;
    this->fuseTime = 3.0f; 
    this->placementTime = 0.0;
}

Bomb::~Bomb() {}

void Bomb::initialize(GLuint shaderID, GLuint texID, vec3 pos)
{
    float cellSize = 60.0f;
    sprite.initialize(shaderID, texID, 1, 4, pos, vec3(cellSize * 0.8f, cellSize * 0.8f, 1.0f));

    this->fuseTime = 3.0f;
    this->placementTime = glfwGetTime();
    this->state = BombState::TICKING;
}

void Bomb::update()
{
    if (this->state == BombState::TICKING)
    {
        if (glfwGetTime() - this->placementTime > this->fuseTime)
        {
            this->state = BombState::EXPLODED;
        }
    }
}

void Bomb::draw()
{
    sprite.update();
    sprite.draw();
}

BombState Bomb::getState()
{
    return this->state;
}

Sprite& Bomb::getSprite()
{
    return this->sprite;
}
