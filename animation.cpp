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
        if (isLastFrame && !isLooping) {return true;}
        nextFrame();
    }
    return false;
}

void Entity::draw(TDT4102::AnimationWindow& window) {
    int x = static_cast<int>(position.x);
    int y = static_cast<int>(position.y);
    TDT4102::Point newPosition{x, y};
    TDT4102::FlipImage flip = (direction == Direction::Right) ? TDT4102::FlipImage::NONE : TDT4102::FlipImage::HORIZONTAL;
    window.draw_image_region(newPosition, animator.getCurrentImage(), size.width, size.height, 
                             {0, 0}, size.width, size.height, flip);
}



void Entity::update(double dt) {
    updateVelocity(dt);
    updateMovementState();
    updatePosition(dt);
    resolveAnimation(); 

    bool animatorFinished = animator.update(dt);
    if (animatorFinished) {
        if (actionState == ActionState::Attack) {
            if (!attackQueued) {
                actionState = ActionState::None;
            }
            else {
                attackQueued = false;
                currentAttackIndex = (currentAttackIndex == 1) ? 2 : 1;
                std::string attackToAnimate = "attack" + std::to_string(currentAttackIndex);
                setAnimation(attackToAnimate, false);
            }
        }
    }

}

void Entity::setAnimation(std::string name, bool shouldLoop) {
    if (currentAnimation == name) { return; }
    animator.setAnimation(animations.at(name), shouldLoop);
    currentAnimation = name;
}

void Entity::move(int dx, int dy) {
    position.x += dx;
    position.y += dy;
}

void Entity::setScale(float newScale) {
    size.width *= newScale;
    size.height *= newScale;
    hurtbox.left *= newScale;
    hurtbox.top *= newScale;
    hurtbox.right *= newScale;
    hurtbox.bottom *= newScale;
}

void Entity::resolveAnimation() {
    if (actionState != ActionState::None) {
        return;
    }
    if (movementState == MovementState::Idle) {
        setAnimation("idle", true);
    }
    if (movementState == MovementState::Turning) {
        setAnimation("turn", false);
    }
    if (movementState == MovementState::Running) {
        setAnimation("run", true);
    }
}

void Entity::setMovementState(MovementState m) {movementState = m;}
void Entity::setActionState(ActionState a) {actionState = a;}

void Entity::attack() {
    if (actionState == ActionState::None) {
        actionState = ActionState::Attack;
        setAnimation("attack1", false);
    }
    else if (actionState == ActionState::Attack) {
        attackQueued = true;
    }
}

void Entity::setMoveIntent(MoveIntent intent) {
    moveIntent = intent;
}

void Entity::updateVelocity(double dt) {
    if (actionState == ActionState::Attack) {
                if (velocity.x < 0) {
            velocity.x += deceleration * dt;
            velocity.x = std::clamp(velocity.x, -maxSpeed, 0.0f);
        }
        else if (velocity.x > 0) {
            velocity.x -= deceleration * dt;
            velocity.x = std::clamp(velocity.x, 0.0f, maxSpeed);
        }
    }
    if (moveIntent == MoveIntent::Right) {
        velocity.x += (acceleration * dt);
        velocity.x = std::clamp(velocity.x, -maxSpeed, maxSpeed);
    }
    else if (moveIntent == MoveIntent::Left) {
        velocity.x -= (acceleration * dt);
        velocity.x = std::clamp(velocity.x, -maxSpeed, maxSpeed);
    }
    else if (moveIntent == MoveIntent::None) {
        if (velocity.x < 0) {
            velocity.x += deceleration * dt;
            velocity.x = std::clamp(velocity.x, -maxSpeed, 0.0f);
        }
        else if (velocity.x > 0) {
            velocity.x -= deceleration * dt;
            velocity.x = std::clamp(velocity.x, 0.0f, maxSpeed);
        }
    }
    if (velocity.x > 0) {
        setDirection(Direction::Right);
    }
    else if (velocity.x < 0) {
        setDirection(Direction::Left);
    }
}

void Entity::updatePosition(double dt) {
    position.x += velocity.x * dt;
}

void Entity::updateMovementState() {
    if (velocity.x == 0) {
        movementState = MovementState::Idle;
    }
    else if (velocity.x > 0 && moveIntent == MoveIntent::Left || 
             velocity.x < 0 && moveIntent == MoveIntent::Right) {
        movementState = MovementState::Turning;
    }
    else {
        movementState = MovementState::Running;
    }
}

void Entity::setDirection(Direction dir) {
    direction = dir;
}

void Entity::drawHurtbox(TDT4102::AnimationWindow& window) {
    Box worldHurtBox = getWorldHurtbox();
    TDT4102::Point topLeftCorner{worldHurtBox.left, worldHurtBox.right};
    TDT4102::Color color = TDT4102::Color::red;
    // window.draw_rectangle(topLeftCorner, worldHurtBox.width(), worldHurtBox.height());
}

bool intersects(Box a, Box b) {
    return !(a.right < b.left ||
             a.left > b.right ||
             a.bottom < b.top ||
             a.top > b.bottom);
}

Box Entity::getWorldHurtbox() {
    return Box{static_cast<int>(position.x) + hurtbox.left, 
               static_cast<int>(position.y) + hurtbox.top, 
               static_cast<int>(position.x) + hurtbox.right,
               static_cast<int>(position.y) + hurtbox.bottom
            };
}