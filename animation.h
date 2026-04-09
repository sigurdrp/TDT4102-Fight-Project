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
    public:
        TDT4102::Image& getCurrentImage();
        void nextFrame();
        void setAnimation(Animation& anim);
        void update(double dt);

        Animator(Animation& anim) 
            : animation(&anim), currentFrame(0), frameDuration(0.1), timeSinceLastFrame(0) {};

};

class Entity {
    private:
        TDT4102::Point position;
        std::map<std::string, Animation> animations;
        Animator animator; // Rekkefølge er kritisk
    public:
        void draw(TDT4102::AnimationWindow& window);
        TDT4102::Point getPosition();
        void update(double dt);
        void setAnimation(std::string name);
        void move(int dx, int dy);

        Entity(TDT4102::Point pos, std::map<std::string, Animation> animMap)
            : position(pos), animations(animMap), animator(animations.at("idle")) {};  
};

