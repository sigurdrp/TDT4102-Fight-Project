#include "AnimationWindow.h"
#include "animation.h"

TDT4102::Image& Animation::getFrame(int index) {
    return frames.at(index);
}

int Animation::size() {
    return frames.size();
}

TDT4102::Image& Animator::getCurrentImage() {
    return animation.getFrame(currentFrame);
}

void Animator::nextFrame() {
    currentFrame = (currentFrame + 1) % animation.size();
}

void Entity::draw(TDT4102::AnimationWindow& window) {
    window.draw_image(pos, animator.getCurrentImage());
}

TDT4102::Point Entity::getPosition() {
    return pos;
}

void Entity::update() {
    animator.nextFrame();
}