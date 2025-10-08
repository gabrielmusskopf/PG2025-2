#ifndef SPRITE_H
#define SPRITE_H

#include <iostream>
#include <string>
#include <assert.h>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW - porque inclui a OpenGL
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

class Sprite
{
    public:
        Sprite();
        ~Sprite();
        void initialize(GLuint shaderID, GLuint texID, int nAnimations, int nFrames, vec3 pos, vec3 dimensions, float angle = 0.0);
        void update();
        void draw();
        void standBy();
        void moveUp();
        void moveDown();
        void moveRight();
        void moveLeft();
        void setPos(vec3 newPos);
        vec3 getPos();
        vec3 getDimensions();
        float getVelX();
        float getVelY();

    public:
        GLuint VAO; 
        GLuint texID;
        vec3 pos; 
        vec3 dimensions; 
        float angle; 
        GLuint shaderID; 
        int nAnimations, nFrames, iAnimations, iFrames;
        vec2 d;

        float lastTime, FPS;
        float velX;
        float velY;
        int remainingBombs;

        GLuint setupGeometry();

};

#endif
