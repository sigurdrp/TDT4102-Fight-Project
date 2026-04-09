#pragma once
#include <vector>
#include "AnimationWindow.h"
#include <map>
#include <string>
#include <iostream>

Animation loadAnimation(std::string folder, std::string prefix, int frameCount);

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
    public:
        TDT4102::Image& getCurrentImage();

        void nextFrame();
        void setAnimation(Animation& anim);

        Animator(Animation& anim) : animation(&anim), currentFrame(0) {};

};

class Entity {
    private:
        TDT4102::Point position;
        std::map<std::string, Animation> animations;
        Animator animator; // Rekkefølge er kritisk
    public:
        void draw(TDT4102::AnimationWindow& window);
        TDT4102::Point getPosition();
        void update();
        void setAnimation(std::string name);

        Entity(TDT4102::Point pos, std::map<std::string, Animation> animMap)
            : position(pos), animations(animMap), animator(animations.at("idle")) {};  
};

