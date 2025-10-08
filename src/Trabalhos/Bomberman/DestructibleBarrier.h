#ifndef DESTRUCTIBLEBARRIER_H
#define DESTRUCTIBLEBARRIER_H

#include "Sprite.h"

class DestructibleBarrier : public Sprite
{
public:
    DestructibleBarrier();
    ~DestructibleBarrier();

    void initialize(float cellSize, GLuint shaderID, GLuint texID, vec3 pos);
};

#endif
