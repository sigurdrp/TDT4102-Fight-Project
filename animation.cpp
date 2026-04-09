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

void Animator::update(double dt) {
    timeSinceLastFrame += dt;
    if (timeSinceLastFrame >= frameDuration) {
        nextFrame();
        timeSinceLastFrame = 0;
    }
}

void Entity::draw(TDT4102::AnimationWindow& window) {
    window.draw_image(position, animator.getCurrentImage());
}

TDT4102::Point Entity::getPosition() {
    return position;
}

void Entity::update(double dt) {
    animator.update(dt);
}

void Entity::setAnimation(std::string name) {
    animator.setAnimation(animations.at(name));
}

void Entity::move(int dx, int dy) {
    position.x += dx;
    position.y += dy;
}