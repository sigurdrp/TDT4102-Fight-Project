#pragma once

#include <map>
#include <string>
#include "AnimationWindow.h"
#include "../Animation.h"
#include "../Types.h"

class Entity {
private:
    Position position;
    std::map<std::string, Animation> animations;
    Animator animator;
    Size size;
    float scale = 1.0f;
    std::string currentAnimation = "idle";
    Box hurtbox;
    Box hitbox;

    MovementState movementState = MovementState::Idle;
    ActionState actionState = ActionState::None;
    AttackVariant currentAttackVariant = AttackVariant::First;
    bool attackQueued = false;
    bool hasHitDuringCurrentAttack = false;
    bool onGround = false;
    bool canBeStunned = true;
    bool deathAnimationFinished = false;

    Velocity velocity{0.0f, 0.0f};
    Direction direction = Direction::Right;
    MoveIntent moveIntent = MoveIntent::None;

    float maxSpeed = 300.0f;
    float acceleration = 1500.0f;
    float deceleration = 3000.0f;
    float hurtKnockbackSpeed = 250.0f;
    float rollSpeedMultiplier = 1.3f;

    float gravity = 1800.0f;
    float jumpSpeed = 800.0f;
    int groundY;

    int health = 100;
    int maxHealth = 100;

protected:
    bool playAnimationIfExists(const std::string& name, bool shouldLoop);
    bool isCurrentAnimation(const std::string& name);
    int getCurrentFrame() const;
    bool beginSingleAttackAnimation(const std::string& animationName);
    void applyKnockback(Direction attackerDirection);

public:
    Entity(TDT4102::Point pos, std::map<std::string, Animation> animMap,
           int spriteWidth, int spriteHeight, float scl, Box hrtbx, Box htbx, int groundY);

    void draw(TDT4102::AnimationWindow& window);
    virtual void update(double dt);
    void setAnimation(std::string name, bool shouldLoop);
    void move(int dx, int dy);
    void setScale(float newScale);

    void resolveAnimation();
    void setMovementState(MovementState m);
    void setActionState(ActionState a);
    void setMoveIntent(MoveIntent intent);
    void setDirection(Direction direction);
    Direction getDirection() const;
    std::string getCurrentAnimationName() const;

    Position getPosition() const;
    float getCenterX() const;
    int getSpriteWidth() const;
    int getSpriteHeight() const;
    TDT4102::Image& getCurrentImage();
    Animation& getAnimationByName(const std::string& name);

    void setCanBeStunned(bool value);
    void setMaxHealth(int value);
    void setHealthToMax();
    int getMaxHealth() const;

    bool attack();
    void hurt(Direction attackerDirection);
    void jump();
    void roll();
    void die();

    void updateVelocity(double dt);
    void updatePosition(double dt);
    void updateMovementState();
    void updateMovement(double dt);
    virtual void updateAnimationState();
    void updateDirectionFromVelocity();
    void applyGravity(double dt);
    void resolveGroundCollision();
    void resolveWorldBounds();
    void onLanded();

    virtual void handleFinishedAnimation();
    void applyDeceleration(double dt);
    void continueAttackCombo();
    void playCurrentAttackAnimation();

    void drawBox(TDT4102::AnimationWindow& window, Box box);

    Box getFacingBox(Box box);
    Box getFacingBoxForDirection(Box box, Direction dir);
    Box getWorldBox(Box box);
    Box getWorldBoxForDirection(Box box, Direction dir);
    Box getWorldHurtBox();
    Box getWorldHitBox();
    Box getWorldHitBoxForDirection(Direction dir);
    Box getHurtBox();
    Box getHitBox();

    virtual Box getCurrentWorldHitBox();
    virtual Box getCurrentWorldHitBoxForDirection(Direction dir);

    bool isHitboxActive();
    bool hasAlreadyHit();
    void markHit();
    bool isStunned();
    bool isGrounded();
    bool isRolling();
    bool isJumping();
    bool isAttacking();
    bool isDead() const;
    bool hasDeathAnimationFinished() const;
    bool hasActiveHurtbox();

    void takeDamage(int damage, Direction attackerDirection);
    void takeDamage(int damage);
    int getHealth() const;

    float getMaxSpeed();
    void updateAnimationOnly(double dt);
    void forceIdleLoop();
};