#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <iostream>
#include <string>
#include <assert.h>

using namespace std;

#include "Sprite.h"

#include <GLFW/glfw3.h>

using namespace glm;

class Explosion : public Sprite
{
    public:
        Explosion();
        ~Explosion();

        void initialize(GLuint shaderID, GLuint texID, int nAnimations, int nFrames, vec3 pos, vec3 dimensions, double duration);
        bool hasExpired();

    private:
        double creationTime;
        double duration;
};

#endif
