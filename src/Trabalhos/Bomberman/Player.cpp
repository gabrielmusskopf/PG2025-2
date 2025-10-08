#include "Player.h"
#include "DestructibleBarrier.h"
#include "IndestructibleBarrier.h"

bool checkCollision(Sprite &one, Sprite &two);

Player::Player()
{
    this->state = PlayerState::IDLE;
    this->moveSpeed = 2.0f;
}

Player::~Player() {}

void Player::initialize(float cellSize, GLuint shaderID, GLuint texID, vec3 startPos)
{
    this->startPosition = startPos;
    this->sprite.initialize(shaderID, texID, 5, 3, startPos, vec3(cellSize, cellSize, 1.0f));
    this->sprite.velX = this->moveSpeed;
    this->sprite.velY = this->moveSpeed; 
    this->state = PlayerState::IDLE;
    this->nActiveBombs = 0;
    this->nBombs = 1;
}

void Player::handleInput(bool keys[])
{
    if (this->state == PlayerState::DEAD) {
        this->moveDirection = vec2(0.0f);
        return;
    }

    this->moveDirection = vec2(0.0f);
    
    bool isMoving = false;

    if (keys[GLFW_KEY_W]) {
        sprite.moveUp();
        this->moveDirection.y = 1.0f;
        isMoving = true;
    } else if (keys[GLFW_KEY_S]) {
        sprite.moveDown();
        this->moveDirection.y = -1.0f;
        isMoving = true;
    }
    
    if (keys[GLFW_KEY_A]) {
        sprite.moveLeft();
        this->moveDirection.x = -1.0f;
        isMoving = true;
    } else if (keys[GLFW_KEY_D]) {
        sprite.moveRight();
        this->moveDirection.x = 1.0f;
        isMoving = true;
    }

    if (isMoving) {
        this->state = PlayerState::WALKING;
    } else {
        sprite.standBy();
        this->state = PlayerState::IDLE;
    }
}

void Player::update(const vector<IndestructibleBarrier>& indestructibleBarriers, const vector<DestructibleBarrier>& destructibleBarriers)
{
    if (this->state == PlayerState::DEAD || this->state == PlayerState::IDLE) {
        return;
    }
    
    vec3 currentPos = sprite.getPos();

    // Normaliza a direção para evitar movimento mais rápido na diagonal, e então aplica a velocidade
    vec3 finalDirection = vec3(normalize(this->moveDirection), 0.0f);
    vec3 nextPos = currentPos + finalDirection * this->moveSpeed;
    
    Sprite futurePlayer = this->sprite;
    futurePlayer.setPos(nextPos);

    bool hasCollided = false;

    for (Sprite barrier : indestructibleBarriers) {
        if (checkCollision(futurePlayer, barrier)) {
            hasCollided = true;
            break;
        }
    }

    if (!hasCollided) {
        for (Sprite brick : destructibleBarriers) {
            if (checkCollision(futurePlayer, brick)) {
                hasCollided = true;
                break;
            }
        }
    }

    if (!hasCollided) {
        sprite.setPos(nextPos);
    }
}

void Player::draw()
{
    this->sprite.update();
    this->sprite.draw();
}

void Player::die()
{
    this->state = PlayerState::DEAD;
}

void Player::reset()
{
    this->state = PlayerState::IDLE;
    this->sprite.setPos(this->startPosition);
    this->nBombs = 1;
    this->nActiveBombs = 0;
}

bool Player::isAlive()
{
    return this->state != PlayerState::DEAD;
}

Sprite& Player::getSprite()
{
    return this->sprite;
}

void Player::increaseBombs()
{
    cout << "Player aumentou uma bomba" << endl;
    this->nBombs++;
}

void Player::placeBomb()
{
    cout << "Player colocou uma bomba" << endl;
    this->nActiveBombs++;
}

void Player::restoreBomb()
{
    this->nActiveBombs--;
    cout << "Player restaurou uma bomba. Tem " << this->nBombs << " e " << this->nActiveBombs << " ativa"<< endl;
}

bool Player::hasBomb()
{
    return this->nActiveBombs < this->nBombs;
}
