#include "Animation.h"

TDT4102::Image& Animation::getFrame(int index) {
    return frames.at(index);
}

int Animation::size() {
    return static_cast<int>(frames.size());
}

Animator::Animator(Animation& anim)
    : animation(&anim), currentFrame(0), frameDuration(0.066), timeSinceLastFrame(0), isLooping(true) {
}

TDT4102::Image& Animator::getCurrentImage() {
    return animation->getFrame(currentFrame);
}

int Animator::getCurrentFrame() {
    return currentFrame;
}

void Animator::nextFrame() {
    currentFrame = (currentFrame + 1) % animation->size();
}

void Animator::setAnimation(Animation& anim, bool shouldLoop) {
    animation = &anim;
    currentFrame = 0;
    timeSinceLastFrame = 0;
    isLooping = shouldLoop;
}

bool Animator::update(double dt) {
    bool isLastFrame = (currentFrame == animation->size() - 1);

    timeSinceLastFrame += dt;
    if (timeSinceLastFrame >= frameDuration) {
        timeSinceLastFrame = 0;
        if (isLastFrame && !isLooping) {
            return true;
        }
        nextFrame();
    }

    return false;
}