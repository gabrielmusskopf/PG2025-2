#ifndef BOMB_H
#define BOMB_H

#include "Sprite.h"
#include <GLFW/glfw3.h>

enum class BombState {
    TICKING,
    EXPLODED
};

class Bomb
{
public:
    Bomb();
    ~Bomb();

    void initialize(GLuint shaderID, GLuint texID, vec3 pos);
    void update();
    void draw();

    BombState getState();
    Sprite& getSprite();

private:
    Sprite sprite;
    BombState state;
    double placementTime;
    float fuseTime;
};

#endif
