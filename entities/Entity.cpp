#include "Entity.h"

#include <iostream>
#include <algorithm>
#include <cmath>

constexpr int WINDOW_WIDTH = 1080;
constexpr int SIDE_COLLISION_MARGIN = 60;

int Box::width() {
    return right - left;
}

int Box::height() {
    return bottom - top;
}

Entity::Entity(TDT4102::Point pos, std::map<std::string, Animation> animMap,
               int spriteWidth, int spriteHeight, float scl, Box hrtbx, Box htbx, int groundY)
    : position{static_cast<float>(pos.x), static_cast<float>(pos.y)},
      animations(animMap),
      animator(animations.at("idle")),
      size{spriteWidth, spriteHeight},
      scale(scl),
      hurtbox(hrtbx),
      hitbox(htbx),
      groundY(groundY),
      health(100),
      maxHealth(100) {
    setScale(scale);
    resolveGroundCollision();
    resolveWorldBounds();
}

bool Entity::playAnimationIfExists(const std::string& name, bool shouldLoop) {
    auto it = animations.find(name);
    if (it == animations.end()) {
        std::cout << "Animation missing: " << name << '\n';
        return false;
    }

    animator.setAnimation(it->second, shouldLoop);
    currentAnimation = name;
    return true;
}

bool Entity::isCurrentAnimation(const std::string& name) {
    return currentAnimation == name;
}

std::string Entity::getCurrentAnimationName() const {
    return currentAnimation;
}

Position Entity::getPosition() const {
    return position;
}

float Entity::getCenterX() const {
    return position.x + static_cast<float>(size.width) * 0.5f;
}

int Entity::getSpriteWidth() const {
    return size.width;
}

int Entity::getSpriteHeight() const {
    return size.height;
}

TDT4102::Image& Entity::getCurrentImage() {
    return animator.getCurrentImage();
}

Animation& Entity::getAnimationByName(const std::string& name) {
    return animations.at(name);
}

void Entity::setCanBeStunned(bool value) {
    canBeStunned = value;
}

void Entity::setMaxHealth(int value) {
    maxHealth = value;
    if (maxHealth < 1) {
        maxHealth = 1;
    }
    if (health > maxHealth) {
        health = maxHealth;
    }
}

void Entity::setHealthToMax() {
    health = maxHealth;
}

int Entity::getMaxHealth() const {
    return maxHealth;
}

int Entity::getCurrentFrame() const {
    return const_cast<Animator&>(animator).getCurrentFrame();
}

bool Entity::beginSingleAttackAnimation(const std::string& animationName) {
    if (!onGround) {
        return false;
    }
    if (actionState != ActionState::None) {
        return false;
    }

    actionState = ActionState::Attack;
    attackQueued = false;
    hasHitDuringCurrentAttack = false;
    setAnimation(animationName, false);
    return true;
}

void Entity::applyKnockback(Direction attackerDirection) {
    moveIntent = MoveIntent::None;
    attackQueued = false;
    hasHitDuringCurrentAttack = false;

    if (attackerDirection == Direction::Right) {
        velocity.x = hurtKnockbackSpeed;
    }
    else {
        velocity.x = -hurtKnockbackSpeed;
    }
}

void Entity::draw(TDT4102::AnimationWindow& window) {
    int x = static_cast<int>(position.x);
    int y = static_cast<int>(position.y);
    TDT4102::Point newPosition{x, y};

    TDT4102::FlipImage flip =
        (direction == Direction::Right)
            ? TDT4102::FlipImage::NONE
            : TDT4102::FlipImage::HORIZONTAL;

    window.draw_image_region(
        newPosition,
        animator.getCurrentImage(),
        size.width,
        size.height,
        {0, 0},
        size.width,
        size.height,
        flip
    );
}

void Entity::update(double dt) {
    if (actionState == ActionState::Death) {
        bool animatorFinished = animator.update(dt);
        if (animatorFinished && !deathAnimationFinished) {
            handleFinishedAnimation();
        }
        return;
    }

    updateMovement(dt);
    updateAnimationState();

    bool animatorFinished = animator.update(dt);
    if (animatorFinished) {
        handleFinishedAnimation();
    }
}

void Entity::setAnimation(std::string name, bool shouldLoop) {
    if (currentAnimation == name) {
        return;
    }

    playAnimationIfExists(name, shouldLoop);
}

void Entity::move(int dx, int dy) {
    position.x += dx;
    position.y += dy;
}

void Entity::setScale(float newScale) {
    size.width = static_cast<int>(size.width * newScale);
    size.height = static_cast<int>(size.height * newScale);

    hurtbox.left   = static_cast<int>(hurtbox.left * newScale);
    hurtbox.top    = static_cast<int>(hurtbox.top * newScale);
    hurtbox.right  = static_cast<int>(hurtbox.right * newScale);
    hurtbox.bottom = static_cast<int>(hurtbox.bottom * newScale);

    hitbox.left   = static_cast<int>(hitbox.left * newScale);
    hitbox.top    = static_cast<int>(hitbox.top * newScale);
    hitbox.right  = static_cast<int>(hitbox.right * newScale);
    hitbox.bottom = static_cast<int>(hitbox.bottom * newScale);
}

void Entity::resolveAnimation() {
    if (actionState == ActionState::Death) {
        return;
    }

    if (actionState != ActionState::None) {
        return;
    }

    if (movementState == MovementState::Idle) {
        setAnimation("idle", true);
    }
    else if (movementState == MovementState::Turning) {
        setAnimation("turn", false);
    }
    else if (movementState == MovementState::Running) {
        setAnimation("run", true);
    }
    else if (movementState == MovementState::Airborne) {
        setAnimation("jump_loop", true);
        actionState = ActionState::JumpLoop;
    }
}

void Entity::setMovementState(MovementState m) {
    movementState = m;
}

void Entity::setActionState(ActionState a) {
    actionState = a;
}

void Entity::setMoveIntent(MoveIntent intent) {
    moveIntent = intent;
}

void Entity::setDirection(Direction dir) {
    direction = dir;
}

Direction Entity::getDirection() const {
    return direction;
}

bool Entity::attack() {
    if (actionState == ActionState::Death) {
        return false;
    }

    if (actionState == ActionState::Hurt ||
        actionState == ActionState::Roll ||
        actionState == ActionState::JumpStart ||
        actionState == ActionState::JumpLoop ||
        actionState == ActionState::JumpLand ||
        !onGround) {
        return false;
    }

    if (actionState == ActionState::None) {
        actionState = ActionState::Attack;
        currentAttackVariant = AttackVariant::First;
        attackQueued = false;
        hasHitDuringCurrentAttack = false;
        playCurrentAttackAnimation();
        return true;
    }
    else if (actionState == ActionState::Attack) {
        attackQueued = true;
    }

    return false;
}

void Entity::hurt(Direction attackerDirection) {
    if (actionState == ActionState::Death) {
        return;
    }

    if (actionState == ActionState::Hurt) {
        return;
    }

    actionState = ActionState::Hurt;
    applyKnockback(attackerDirection);
    setAnimation("hurt", false);
}

void Entity::jump() {
    if (actionState == ActionState::Death) {
        return;
    }

    if (!onGround) {
        return;
    }
    if (actionState != ActionState::None) {
        return;
    }

    onGround = false;
    actionState = ActionState::JumpStart;
    velocity.y = -jumpSpeed;
    setAnimation("jump_start", false);
}

void Entity::roll() {
    if (actionState == ActionState::Death) {
        return;
    }

    if (!onGround) {
        return;
    }
    if (actionState != ActionState::None) {
        return;
    }
    if (std::abs(velocity.x) < 0.01f) {
        return;
    }

    actionState = ActionState::Roll;
    moveIntent = MoveIntent::None;
    hasHitDuringCurrentAttack = false;

    float rollSpeed = maxSpeed * rollSpeedMultiplier;
    if (velocity.x > 0) {
        velocity.x = rollSpeed;
    }
    else {
        velocity.x = -rollSpeed;
    }

    setAnimation("roll", false);
}

void Entity::die() {
    if (actionState == ActionState::Death) {
        return;
    }

    health = 0;
    deathAnimationFinished = false;
    actionState = ActionState::Death;
    movementState = MovementState::Idle;
    moveIntent = MoveIntent::None;
    attackQueued = false;
    hasHitDuringCurrentAttack = false;
    velocity.x = 0.0f;
    velocity.y = 0.0f;

    playAnimationIfExists("death", false);
}

void Entity::updateVelocity(double dt) {
    if (actionState == ActionState::Death) {
        return;
    }

    if (actionState == ActionState::Hurt) {
        applyDeceleration(dt);
    }
    else if (actionState == ActionState::Attack) {
        if (currentAnimation == "heavy_combo") {
            int frame = getCurrentFrame();

            if (frame >= 39 && frame <= 45) {
                float backdashSpeed = maxSpeed * 0.85f;
                if (direction == Direction::Right) {
                    velocity.x = -backdashSpeed;
                }
                else {
                    velocity.x = backdashSpeed;
                }
            }
            else {
                applyDeceleration(dt);
            }
        }
        else {
            applyDeceleration(dt);
        }
    }
    else if (actionState == ActionState::Roll) {
    }
    else if (actionState == ActionState::JumpStart ||
             actionState == ActionState::JumpLoop ||
             actionState == ActionState::JumpLand) {
    }
    else if (moveIntent == MoveIntent::Right) {
        velocity.x += acceleration * static_cast<float>(dt);
        velocity.x = std::clamp(velocity.x, -maxSpeed, maxSpeed);
    }
    else if (moveIntent == MoveIntent::Left) {
        velocity.x -= acceleration * static_cast<float>(dt);
        velocity.x = std::clamp(velocity.x, -maxSpeed, maxSpeed);
    }
    else {
        applyDeceleration(dt);
    }

    if (actionState != ActionState::Hurt &&
        actionState != ActionState::Roll &&
        actionState != ActionState::JumpStart &&
        actionState != ActionState::JumpLoop &&
        actionState != ActionState::JumpLand &&
        actionState != ActionState::Attack &&
        actionState != ActionState::Death) {
        updateDirectionFromVelocity();
    }
}

void Entity::applyGravity(double dt) {
    if (actionState == ActionState::Death) {
        return;
    }

    if (!onGround) {
        velocity.y += gravity * static_cast<float>(dt);
    }
}

void Entity::updatePosition(double dt) {
    position.x += velocity.x * static_cast<float>(dt);
    position.y += velocity.y * static_cast<float>(dt);
}

void Entity::resolveGroundCollision() {
    int worldBottom = getWorldHurtBox().bottom;

    if (worldBottom >= groundY) {
        int penetration = worldBottom - groundY;
        position.y -= static_cast<float>(penetration);
        velocity.y = 0.0f;

        bool wasAirborne = !onGround;
        onGround = true;

        if (wasAirborne) {
            onLanded();
        }
    }
    else {
        onGround = false;
    }
}

void Entity::resolveWorldBounds() {
    int leftLimit = -SIDE_COLLISION_MARGIN;
    int rightLimit = WINDOW_WIDTH + SIDE_COLLISION_MARGIN;

    Box worldHurt = getWorldHurtBox();

    if (worldHurt.left < leftLimit) {
        int correction = leftLimit - worldHurt.left;
        position.x += static_cast<float>(correction);
        velocity.x = 0.0f;
    }
    else if (worldHurt.right > rightLimit) {
        int correction = worldHurt.right - rightLimit;
        position.x -= static_cast<float>(correction);
        velocity.x = 0.0f;
    }
}

void Entity::onLanded() {
    if (actionState == ActionState::JumpStart || actionState == ActionState::JumpLoop) {
        actionState = ActionState::JumpLand;
        setAnimation("jump_land", false);
    }
}

void Entity::updateMovementState() {
    if (actionState == ActionState::Death) {
        movementState = MovementState::Idle;
        return;
    }

    if (!onGround) {
        movementState = MovementState::Airborne;
        return;
    }

    if (actionState == ActionState::Hurt ||
        actionState == ActionState::Roll ||
        actionState == ActionState::Attack ||
        actionState == ActionState::JumpLand) {
        movementState = MovementState::Idle;
        return;
    }

    if (std::abs(velocity.x) < 0.01f) {
        movementState = MovementState::Idle;
    }
    else if ((velocity.x > 0 && moveIntent == MoveIntent::Left) ||
             (velocity.x < 0 && moveIntent == MoveIntent::Right)) {
        movementState = MovementState::Turning;
    }
    else {
        movementState = MovementState::Running;
    }
}

void Entity::updateMovement(double dt) {
    updateVelocity(dt);
    applyGravity(dt);
    updatePosition(dt);
    resolveGroundCollision();
    resolveWorldBounds();
    updateMovementState();
}

void Entity::updateAnimationState() {
    if (actionState == ActionState::Death) {
        return;
    }

    resolveAnimation();

    if (!onGround && actionState == ActionState::None) {
        actionState = ActionState::JumpLoop;
        setAnimation("jump_loop", true);
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

void Entity::handleFinishedAnimation() {
    if (actionState == ActionState::Death) {
        deathAnimationFinished = true;
        velocity.x = 0.0f;
        velocity.y = 0.0f;
        return;
    }

    if (actionState == ActionState::Attack) {
        if (attackQueued) {
            continueAttackCombo();
        }
        else {
            actionState = ActionState::None;
        }
    }
    else if (actionState == ActionState::Hurt) {
        actionState = ActionState::None;
    }
    else if (actionState == ActionState::Roll) {
        actionState = ActionState::None;
    }
    else if (actionState == ActionState::JumpStart) {
        if (!onGround) {
            actionState = ActionState::JumpLoop;
            setAnimation("jump_loop", true);
        }
        else {
            actionState = ActionState::JumpLand;
            setAnimation("jump_land", false);
        }
    }
    else if (actionState == ActionState::JumpLand) {
        actionState = ActionState::None;
    }
}

void Entity::applyDeceleration(double dt) {
    if (velocity.x < 0) {
        velocity.x += deceleration * static_cast<float>(dt);
        velocity.x = std::clamp(velocity.x, -maxSpeed * rollSpeedMultiplier, 0.0f);
    }
    else if (velocity.x > 0) {
        velocity.x -= deceleration * static_cast<float>(dt);
        velocity.x = std::clamp(velocity.x, 0.0f, maxSpeed * rollSpeedMultiplier);
    }
}

void Entity::continueAttackCombo() {
    attackQueued = false;
    hasHitDuringCurrentAttack = false;

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

void Entity::drawBox(TDT4102::AnimationWindow& window, Box box) {
    TDT4102::Point topLeftCorner{box.left, box.top};
    window.draw_rectangle(topLeftCorner, box.width(), box.height());
}

Box Entity::getFacingBox(Box box) {
    return getFacingBoxForDirection(box, direction);
}

Box Entity::getFacingBoxForDirection(Box box, Direction dir) {
    if (dir == Direction::Right) {
        return box;
    }

    return Box{
        size.width - box.right,
        box.top,
        size.width - box.left,
        box.bottom
    };
}

Box Entity::getWorldBox(Box box) {
    return getWorldBoxForDirection(box, direction);
}

Box Entity::getWorldBoxForDirection(Box box, Direction dir) {
    Box facingBox = getFacingBoxForDirection(box, dir);

    return Box{
        static_cast<int>(position.x) + facingBox.left,
        static_cast<int>(position.y) + facingBox.top,
        static_cast<int>(position.x) + facingBox.right,
        static_cast<int>(position.y) + facingBox.bottom
    };
}

Box Entity::getWorldHurtBox() {
    return getWorldBox(hurtbox);
}

Box Entity::getWorldHitBox() {
    return getWorldBox(hitbox);
}

Box Entity::getWorldHitBoxForDirection(Direction dir) {
    return getWorldBoxForDirection(hitbox, dir);
}

Box Entity::getHurtBox() {
    return hurtbox;
}

Box Entity::getHitBox() {
    return hitbox;
}

Box Entity::getCurrentWorldHitBox() {
    return getWorldHitBox();
}

Box Entity::getCurrentWorldHitBoxForDirection(Direction dir) {
    return getWorldHitBoxForDirection(dir);
}

bool Entity::isHitboxActive() {
    if (actionState == ActionState::Death) {
        return false;
    }

    if (actionState != ActionState::Attack) {
        return false;
    }

    int frame = getCurrentFrame();

    if (currentAnimation == "attack1" || currentAnimation == "attack2") {
        return frame == 2 || frame == 3;
    }
    if (currentAnimation == "heavy_combo") {
        return frame == 8 || frame == 15 || frame == 23;
    }
    if (currentAnimation == "tornado") {
        return frame >= 9 && frame <= 15;
    }
    if (currentAnimation == "stomp") {
        return frame >= 10 && frame <= 13;
    }

    return false;
}

bool Entity::hasAlreadyHit() {
    return hasHitDuringCurrentAttack;
}

void Entity::markHit() {
    hasHitDuringCurrentAttack = true;
}

bool Entity::isStunned() {
    return actionState == ActionState::Hurt;
}

bool Entity::isGrounded() {
    return onGround;
}

bool Entity::isRolling() {
    return actionState == ActionState::Roll;
}

bool Entity::isJumping() {
    return actionState == ActionState::JumpStart ||
           actionState == ActionState::JumpLoop ||
           actionState == ActionState::JumpLand;
}

bool Entity::isAttacking() {
    return actionState == ActionState::Attack;
}

bool Entity::isDead() const {
    return actionState == ActionState::Death;
}

bool Entity::hasDeathAnimationFinished() const {
    return deathAnimationFinished;
}

bool Entity::hasActiveHurtbox() {
    if (actionState == ActionState::Death) {
        return false;
    }

    if (actionState == ActionState::Roll) {
        return false;
    }

    if (currentAnimation == "heavy_combo") {
        int frame = getCurrentFrame();
        if (frame >= 39 && frame <= 45) {
            return false;
        }
    }

    return true;
}

void Entity::takeDamage(int damage, Direction attackerDirection) {
    if (actionState == ActionState::Death) {
        return;
    }

    health -= damage;
    if (health < 0) {
        health = 0;
    }

    if (canBeStunned) {
        hurt(attackerDirection);
    }
    else {
        applyKnockback(attackerDirection);
    }
}

void Entity::takeDamage(int damage) {
    if (actionState == ActionState::Death) {
        return;
    }

    health -= damage;
    if (health < 0) {
        health = 0;
    }

    if (canBeStunned) {
        hurt(Direction::Left);
    }
    else {
        applyKnockback(Direction::Left);
    }
}

int Entity::getHealth() const {
    return health;
}

float Entity::getMaxSpeed() {
    return maxSpeed;
}

void Entity::updateAnimationOnly(double dt) {
    if (actionState == ActionState::Death) {
        bool animatorFinished = animator.update(dt);
        if (animatorFinished && !deathAnimationFinished) {
            handleFinishedAnimation();
        }
        return;
    }

    updateAnimationState();
    bool animatorFinished = animator.update(dt);
    if (animatorFinished) {
        handleFinishedAnimation();
    }
}

void Entity::forceIdleLoop() {
    if (actionState == ActionState::Death) {
        return;
    }

    velocity.x = 0.0f;
    velocity.y = 0.0f;
    moveIntent = MoveIntent::None;
    attackQueued = false;
    hasHitDuringCurrentAttack = false;
    movementState = MovementState::Idle;
    actionState = ActionState::None;
    setAnimation("idle", true);
}