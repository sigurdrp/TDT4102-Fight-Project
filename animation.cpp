#include "AnimationWindow.h"
#include "animation.h"
#include <iostream>

TDT4102::Image& Animation::getFrame(int index) {
    return frames.at(index);
}

int Animation::size() {
    return frames.size();
}

TDT4102::Image& Animator::getCurrentImage() {
    return animation->getFrame(currentFrame);
}

void Animator::nextFrame() {
    currentFrame = (currentFrame + 1) % animation->size();
}

void Animator::setAnimation(Animation& anim) {
    animation = &anim;
    currentFrame = 0;
}

void Entity::draw(TDT4102::AnimationWindow& window) {
    window.draw_image(position, animator.getCurrentImage());
}

TDT4102::Point Entity::getPosition() {
    return position;
}

void Entity::update() {
    animator.nextFrame();
}

void Entity::setAnimation(std::string name) {
    animator.setAnimation(animations.at(name));
}

Animation loadAnimation(std::string folder, std::string prefix, int frameCount) {
    // Trenger feildetektering
    std::vector<TDT4102::Image> frames;
    for (int i = 1; i <= frameCount; i++) {
        std::string pathToPush{folder + prefix + '/' + std::to_string(frameCount) + ".png"};
        TDT4102::Image imageToPush(pathToPush);
        frames.push_back(imageToPush);
    }
    return Animation(frames);
}