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
    updateMovement(dt);
    updateAnimationState();

    bool animatorFinished = animator.update(dt);
    if (animatorFinished) {
        handleFinishedAnimation();
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
    hitbox.left *= newScale;
    hitbox.top *= newScale;
    hitbox.right *= newScale;
    hitbox.bottom *= newScale;
}

void Entity::resolveAnimation() {
    if (actionState != ActionState::None) {
        return;
    }
    else if (movementState == MovementState::Idle) {
        setAnimation("idle", true);
    }
    else if (movementState == MovementState::Turning) {
        setAnimation("turn", false);
    }
    else if (movementState == MovementState::Running) {
        setAnimation("run", true);
    }
}

void Entity::setMovementState(MovementState m) {movementState = m;}
void Entity::setActionState(ActionState a) {actionState = a;}

void Entity::attack() {
    if (actionState == ActionState::None) {
        actionState = ActionState::Attack;
        currentAttackVariant = AttackVariant::First;
        attackQueued = false;
        playCurrentAttackAnimation();
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
        applyDeceleration(dt);
    }
    else if (moveIntent == MoveIntent::Right) {
        velocity.x += acceleration * dt;
        velocity.x = std::clamp(velocity.x, -maxSpeed, maxSpeed);
    }
    else if (moveIntent == MoveIntent::Left) {
        velocity.x -= acceleration * dt;
        velocity.x = std::clamp(velocity.x, -maxSpeed, maxSpeed);
    }
    else {
        applyDeceleration(dt);
    }

    updateDirectionFromVelocity();
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

void Entity::drawBox(TDT4102::AnimationWindow& window, Box box) {
    TDT4102::Point topLeftCorner{box.left, box.top};
    window.draw_rectangle(topLeftCorner, box.width(), box.height());
}

bool intersects(Box a, Box b) {
    return !(a.right < b.left ||
             a.left > b.right ||
             a.bottom < b.top ||
             a.top > b.bottom);
}

Box Entity::getWorldBox(Box box) {
    return Box{static_cast<int>(position.x) + box.left, 
               static_cast<int>(position.y) + box.top, 
               static_cast<int>(position.x) + box.right,
               static_cast<int>(position.y) + box.bottom
    };
}

int Box::width() {
    return right - left;
}
int Box::height() {
    return bottom - top;
}

void Entity::updateMovement(double dt) {
    updateVelocity(dt);
    updateMovementState();
    updatePosition(dt);
}

void Entity::updateAnimationState() {
    resolveAnimation();
}

void Entity::handleFinishedAnimation() {
    if (actionState != ActionState::Attack) {
        return;
    }

    if (attackQueued) {
        continueAttackCombo();
    }
    else {
        actionState = ActionState::None;
    }
}

void Entity::applyDeceleration(double dt) {
    if (velocity.x < 0) {
        velocity.x += deceleration * dt;
        velocity.x = std::clamp(velocity.x, -maxSpeed, 0.0f);
    }
    else if (velocity.x > 0) {
        velocity.x -= deceleration * dt;
        velocity.x = std::clamp(velocity.x, 0.0f, maxSpeed);
    }
}

void Entity::continueAttackCombo() {
    attackQueued = false;

    if (currentAttackVariant == AttackVariant::First) {
        currentAttackVariant = AttackVariant::Second;
    }
    else {
        currentAttackVariant = AttackVariant::First;
    }

    playCurrentAttackAnimation();
}

void Entity::playCurrentAttackAnimation() {
    if (currentAttackVariant == AttackVariant::First) {
        setAnimation("attack1", false);
    }
    else {
        setAnimation("attack2", false);
    }
}

void Entity::updateDirectionFromVelocity() {
    if (velocity.x > 0) {
        setDirection(Direction::Right);
    }
    else if (velocity.x < 0) {
        setDirection(Direction::Left);
    }
}

Box Entity::getHurtBox() {
    return hurtbox;
}

Box Entity::getHitBox() {
    return hitbox;
}