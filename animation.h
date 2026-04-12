#pragma once

#include <vector>
#include "AnimationWindow.h"

class Animation {
private:
    std::vector<TDT4102::Image> frames;

public:
    Animation(std::vector<TDT4102::Image> frames) : frames{frames} {}

    TDT4102::Image& getFrame(int index);
    int size();
};

class Animator {
private:
    Animation* animation;
    int currentFrame;
    double frameDuration;
    double timeSinceLastFrame;
    bool isLooping;

public:
    Animator(Animation& anim);

    TDT4102::Image& getCurrentImage();
    int getCurrentFrame();
    void nextFrame();
    void setAnimation(Animation& anim, bool shouldLoop);
    bool update(double dt);
};