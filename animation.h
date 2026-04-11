#pragma once
#include <vector>
#include "AnimationWindow.h"
#include <map>
#include <string>
#include <iostream>

class Animation {
    private:
        std::vector<TDT4102::Image> frames;
    public:
        TDT4102::Image& getFrame(int index);
        
        int size();
        Animation(std::vector<TDT4102::Image> frames) : frames{frames} {};
};

class Animator {
    private:
        Animation* animation;
        int currentFrame;
        double frameDuration;
        double timeSinceLastFrame;
        bool isLooping;
    public:
        TDT4102::Image& getCurrentImage();
        void nextFrame();
        void setAnimation(Animation& anim, bool shouldLoop);
        bool update(double dt);

        Animator(Animation& anim) 
            : animation(&anim), currentFrame(0), frameDuration(0.066), timeSinceLastFrame(0), isLooping(true) {};

};

enum class MovementState {
    Idle,
    Running,
    Turning,
    Jumping,
    Hurt
};

enum class ActionState {
    None,   
    Attack
};

struct Size {
    int width;
    int height;
};

enum class Direction {
    Right,
    Left
};

struct Velocity {
    float x;
    float y;
};

enum class MoveIntent {
    Right,
    None,
    Left
};

struct Position {
    float x;
    float y;
};

struct Box {
    int left;
    int top;
    int right;
    int bottom;
    
    int width();
    int height();
};

enum class AttackVariant {
    First,
    Second
};

class Entity {
    private:
        Position position;
        std::map<std::string, Animation> animations;
        Animator animator; // Rekkefølge er kritisk
        Size size;
        float scale = 1.0f;
        std::string currentAnimation = "idle";
        Box hurtbox;
        Box hitbox;

        MovementState movementState = MovementState::Idle;
        ActionState actionState = ActionState::None;
        AttackVariant currentAttackVariant = AttackVariant::First;
        bool attackQueued = false;

        Velocity velocity{0, 0};
        Direction direction = Direction::Right;
        MoveIntent moveIntent = MoveIntent::None;

        float maxSpeed = 300.0f;
        float acceleration = 1500.0f;
        float deceleration = 3000.0f;

    public:

        void draw(TDT4102::AnimationWindow& window);
        void update(double dt);
        void setAnimation(std::string name, bool shouldLoop);
        void move(int dx, int dy);
        void resolveAnimation();
        void setScale(float newScale);
        void setMovementState(MovementState m);
        void setActionState(ActionState a);
        void setMoveIntent(MoveIntent intent);
        void updateVelocity(double dt);
        void updateMovementState();
        void updatePosition(double dt);
        void setDirection(Direction direction);
        void attack();
        void drawBox(TDT4102::AnimationWindow& window, Box box);
        Box getWorldBox(Box box);
        Box getHurtBox();
        Box getHitBox();

        void updateMovement(double dt);
        void updateAnimationState();
        void handleFinishedAnimation();
        void applyDeceleration(double dt);
        void continueAttackCombo();
        void playCurrentAttackAnimation();
        void updateDirectionFromVelocity();
        

        Entity(TDT4102::Point pos, std::map<std::string, Animation> animMap, 
               int spriteWidth, int spriteHeight, float scl, Box hrtbx, Box htbx)
            : position{pos.x, pos.y}, 
              animations(animMap), 
              animator(animations.at("idle")), 
              size{spriteWidth, spriteHeight}, 
              scale(scl),
              hurtbox(hrtbx),
              hitbox(htbx)
            {
                setScale(scale);
            };  
};

bool intersects(Box a, Box b);

