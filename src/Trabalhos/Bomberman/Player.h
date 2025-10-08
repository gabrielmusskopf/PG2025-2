#ifndef PLAYER_H
#define PLAYER_H

#include "DestructibleBarrier.h"
#include "IndestructibleBarrier.h"
#include "Sprite.h"
#include <vector>

enum class PlayerState {
    IDLE,
    WALKING,
    DEAD
};

class Player
{
public:
    Player();
    ~Player();

    void initialize(float cellSize, GLuint shaderID, GLuint texID, vec3 startPos);
    void handleInput(bool keys[]);
    void update(const vector<IndestructibleBarrier>& indestructibleBarriers, const vector<DestructibleBarrier>& destructibleBarriers);
    
    void draw();

    void die();
    void reset();
    bool isAlive();
    void increaseBombs();
    bool hasBomb();
    void placeBomb();
    void restoreBomb();

    Sprite& getSprite(); 

private:
    Sprite sprite;
    PlayerState state;
    vec3 startPosition;
    float moveSpeed;
    vec2 moveDirection;
    int nBombs;
    int nActiveBombs;
};

#endif
