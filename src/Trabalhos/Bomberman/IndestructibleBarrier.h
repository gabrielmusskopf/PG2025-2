#ifndef INDESTRUCTIBLEBARRIER_H
#define INDESTRUCTIBLEBARRIER_H

#include "Sprite.h"

class IndestructibleBarrier : public Sprite
{
public:
    IndestructibleBarrier();
    ~IndestructibleBarrier();

    void initialize(float cellSize, GLuint shaderID, GLuint texID, vec3 pos);
};

#endif
