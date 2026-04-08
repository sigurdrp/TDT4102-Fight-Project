#pragma once
#include <vector>
#include "AnimationWindow.h"


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
        Animation& animation;
        int currentFrame;
    public:
        TDT4102::Image& getCurrentImage();

        void nextFrame();

        Animator(Animation& anim) : animation(anim), currentFrame(0) {} // Initializer list 

};

class Entity {
    private:
        Animator animator;
        TDT4102::Point pos;
    public:
        void draw(TDT4102::AnimationWindow& window);
        TDT4102::Point getPosition();
        void update();

        Entity(Animator animr) : animator(animr), pos(100, 100) {}
};