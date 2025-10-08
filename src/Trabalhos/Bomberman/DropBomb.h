#ifndef DROPBOMB_H
#define DROPBOMB_H

#include "Sprite.h"
#include <GLFW/glfw3.h>

class DropBomb : public Sprite
{
public:
    DropBomb();
    ~DropBomb();

    void initialize(GLuint shaderID, GLuint texID, int nAnimations, int nFrames, vec3 pos, vec3 dimensions);
};

#endif
