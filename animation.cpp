#include "animation.h"
#include "AnimationWindow.h"
#include <iostream>
#include <cmath>
#include <chrono>

constexpr int WINDOW_WIDTH = 1080;
constexpr int SIDE_COLLISION_MARGIN = 60;

// Juster disse hvis eruption-spriten ser for stor/liten ut
constexpr int ERUPTION_DRAW_WIDTH = 160;
constexpr int ERUPTION_DRAW_HEIGHT = 120;
constexpr double ERUPTION_FRAME_DURATION = 0.055;
constexpr int ERUPTION_ACTIVE_FRAME_START = 4;
constexpr int ERUPTION_ACTIVE_FRAME_END = 9;
constexpr float ERUPTION_SPACING = 150.0f;
constexpr double ERUPTION_INITIAL_DELAY = 0.33;
constexpr double ERUPTION_CHAIN_DELAY = 0.33;

TDT4102::Image& Animation::getFrame(int index) {
    return frames.at(index);
}

int Animation::size() {
    return static_cast<int>(frames.size());
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

bool Player::resolveInput(TDT4102::AnimationWindow& window) {
    bool aPressed = window.is_key_down(KeyboardKey::A);
    bool dPressed = window.is_key_down(KeyboardKey::D);
    bool spacePressed = window.is_key_down(KeyboardKey::SPACE);
    bool wPressed = window.is_key_down(KeyboardKey::W);
    bool shiftPressed = window.is_key_down(KeyboardKey::LEFT_SHIFT);

    bool spacePressedNow = spacePressed && !spaceWasPressed;
    bool wPressedNow = wPressed && !wWasPressed;
    bool shiftPressedNow = shiftPressed && !shiftWasPressed;

    bool startedAttack = false;

    if (isDead()) {
        setMoveIntent(MoveIntent::None);
        spaceWasPressed = spacePressed;
        wWasPressed = wPressed;
        shiftWasPressed = shiftPressed;
        return false;
    }

    if (isStunned()) {
        setMoveIntent(MoveIntent::None);
        spaceWasPressed = spacePressed;
        wWasPressed = wPressed;
        shiftWasPressed = shiftPressed;
        return false;
    }

    if (isRolling() || isJumping()) {
        setMoveIntent(MoveIntent::None);
        spaceWasPressed = spacePressed;
        wWasPressed = wPressed;
        shiftWasPressed = shiftPressed;
        return false;
    }

    if (aPressed && !dPressed) {
        setMoveIntent(MoveIntent::Left);
    }
    else if (!aPressed && dPressed) {
        setMoveIntent(MoveIntent::Right);
    }
    else {
        setMoveIntent(MoveIntent::None);
    }

    if (wPressedNow) {
        jump();
    }

    if (shiftPressedNow) {
        roll();
    }

    if (spacePressedNow) {
        startedAttack = attack();
    }

    spaceWasPressed = spacePressed;
    wWasPressed = wPressed;
    shiftWasPressed = shiftPressed;

    return startedAttack;
}

Boss::Boss(TDT4102::Point pos, std::map<std::string, Animation> animMap,
           int spriteWidth, int spriteHeight, float scl,
           Box hrtbx, Box heavyComboHtbx, Box tornadoHtbx, Box stompHtbx, Box eruptionHtbx, int groundY)
    : Entity(pos, animMap, spriteWidth, spriteHeight, scl, hrtbx, heavyComboHtbx, groundY),
      rng(static_cast<unsigned int>(
          std::chrono::steady_clock::now().time_since_epoch().count())),
      heavyComboHitbox(heavyComboHtbx),
      tornadoHitbox(tornadoHtbx),
      stompHitbox(stompHtbx),
      eruptionHitbox(eruptionHtbx) {
    setDirection(Direction::Left);
    setCanBeStunned(false);
    setMaxHealth(200);
    setHealthToMax();
    targetX = getCenterX();

    heavyComboHitbox.left   = static_cast<int>(heavyComboHitbox.left * scl);
    heavyComboHitbox.top    = static_cast<int>(heavyComboHitbox.top * scl);
    heavyComboHitbox.right  = static_cast<int>(heavyComboHitbox.right * scl);
    heavyComboHitbox.bottom = static_cast<int>(heavyComboHitbox.bottom * scl);

    tornadoHitbox.left   = static_cast<int>(tornadoHitbox.left * scl);
    tornadoHitbox.top    = static_cast<int>(tornadoHitbox.top * scl);
    tornadoHitbox.right  = static_cast<int>(tornadoHitbox.right * scl);
    tornadoHitbox.bottom = static_cast<int>(tornadoHitbox.bottom * scl);

    stompHitbox.left   = static_cast<int>(stompHitbox.left * scl);
    stompHitbox.top    = static_cast<int>(stompHitbox.top * scl);
    stompHitbox.right  = static_cast<int>(stompHitbox.right * scl);
    stompHitbox.bottom = static_cast<int>(stompHitbox.bottom * scl);

    eruptionHitbox.left   = static_cast<int>(eruptionHitbox.left * scl);
    eruptionHitbox.top    = static_cast<int>(eruptionHitbox.top * scl);
    eruptionHitbox.right  = static_cast<int>(eruptionHitbox.right * scl);
    eruptionHitbox.bottom = static_cast<int>(eruptionHitbox.bottom * scl);
}

void Boss::startEntry() {
    inEntry = true;
    entryFinished = false;
    setMoveIntent(MoveIntent::None);
    playAnimationIfExists("entry", false);
}

bool Boss::hasFinishedEntry() {
    return entryFinished;
}

void Boss::setTargetX(float x) {
    targetX = x;
}

bool Boss::heavyAttack1() {
    return beginSingleAttackAnimation("heavy_combo");
}

bool Boss::heavyAttack2() {
    return beginSingleAttackAnimation("heavy_combo");
}

bool Boss::heavyAttack3() {
    return beginSingleAttackAnimation("heavy_combo");
}

bool Boss::tornadoAttack() {
    bool started = beginSingleAttackAnimation("tornado");
    if (started) {
        tornadoStartedThisFrame = true;
    }
    return started;
}

bool Boss::stompAttack() {
    bool started = beginSingleAttackAnimation("stomp");
    if (started) {
        stompStartedThisFrame = true;
        spawnStompEruptionWave();
    }
    return started;
}

bool Boss::consumeTornadoStarted() {
    bool started = tornadoStartedThisFrame;
    tornadoStartedThisFrame = false;
    return started;
}

bool Boss::consumeStompStarted() {
    bool started = stompStartedThisFrame;
    stompStartedThisFrame = false;
    return started;
}

void Boss::spawnStompEruptionWave() {
    eruptions.clear();

    float sign = (getDirection() == Direction::Right) ? 1.0f : -1.0f;

    Box hurt = getWorldHurtBox();

    float firstX;
    if (getDirection() == Direction::Right) {
        firstX = static_cast<float>(hurt.right - ERUPTION_DRAW_WIDTH / 3);
    }
    else {
        firstX = static_cast<float>(hurt.left - (ERUPTION_DRAW_WIDTH * 2 / 3));
    }

    for (int i = 0; i < 3; ++i) {
        EruptionInstance e;
        e.x = firstX + sign * ERUPTION_SPACING * static_cast<float>(i);
        e.startDelay = ERUPTION_INITIAL_DELAY + ERUPTION_CHAIN_DELAY * static_cast<double>(i);
        eruptions.push_back(e);
    }
}

void Boss::updateEruptions(double dt) {
    Animation& eruptionAnim = getAnimationByName("eruption");
    double totalAnimTime = static_cast<double>(eruptionAnim.size()) * ERUPTION_FRAME_DURATION;

    for (auto& e : eruptions) {
        if (e.finished) {
            continue;
        }

        e.elapsed += dt;

        if (e.elapsed < e.startDelay) {
            continue;
        }

        double localTime = e.elapsed - e.startDelay;
        if (localTime >= totalAnimTime) {
            e.finished = true;
        }
    }
}

void Boss::drawEruptions(TDT4102::AnimationWindow& window) {
    Animation& eruptionAnim = getAnimationByName("eruption");

    TDT4102::FlipImage flip =
        (getDirection() == Direction::Right)
            ? TDT4102::FlipImage::NONE
            : TDT4102::FlipImage::HORIZONTAL;

    for (const auto& e : eruptions) {
        if (e.finished || e.elapsed < e.startDelay) {
            continue;
        }

        double localTime = e.elapsed - e.startDelay;
        int frame = static_cast<int>(localTime / ERUPTION_FRAME_DURATION);
        if (frame < 0) {
            frame = 0;
        }
        if (frame >= eruptionAnim.size()) {
            frame = eruptionAnim.size() - 1;
        }

        int x = static_cast<int>(e.x);
        int y = getWorldHurtBox().bottom - ERUPTION_DRAW_HEIGHT;

        window.draw_image_region(
            {x, y},
            eruptionAnim.getFrame(frame),
            ERUPTION_DRAW_WIDTH,
            ERUPTION_DRAW_HEIGHT,
            {0, 0},
            ERUPTION_DRAW_WIDTH,
            ERUPTION_DRAW_HEIGHT,
            flip
        );
    }
}

bool Boss::tryHitPlayerWithEruptions(Player& player) {
    if (isDead()) {
        return false;
    }

    bool didHit = false;
    Animation& eruptionAnim = getAnimationByName("eruption");

    for (auto& e : eruptions) {
        if (e.finished || e.hasHitPlayer || e.elapsed < e.startDelay) {
            continue;
        }

        double localTime = e.elapsed - e.startDelay;
        int frame = static_cast<int>(localTime / ERUPTION_FRAME_DURATION);
        if (frame < ERUPTION_ACTIVE_FRAME_START || frame > ERUPTION_ACTIVE_FRAME_END) {
            continue;
        }
        if (frame >= eruptionAnim.size()) {
            continue;
        }

        int baseX = static_cast<int>(e.x);
        int baseY = getWorldHurtBox().bottom - ERUPTION_DRAW_HEIGHT;

        Box worldBox{
            baseX + eruptionHitbox.left,
            baseY + eruptionHitbox.top,
            baseX + eruptionHitbox.right,
            baseY + eruptionHitbox.bottom
        };

        if (intersects(worldBox, player.getWorldHurtBox())) {
            e.hasHitPlayer = true;
            player.takeDamage(1, getDirection());
            didHit = true;
        }
    }

    return didHit;
}

Box Boss::getCurrentWorldHitBox() {
    if (getCurrentAnimationName() == "heavy_combo") {
        return getWorldBox(heavyComboHitbox);
    }
    if (getCurrentAnimationName() == "tornado") {
        return getWorldBox(tornadoHitbox);
    }
    if (getCurrentAnimationName() == "stomp") {
        return getWorldBox(stompHitbox);
    }
    return Entity::getCurrentWorldHitBox();
}

Box Boss::getCurrentWorldHitBoxForDirection(Direction dir) {
    if (getCurrentAnimationName() == "heavy_combo") {
        return getWorldBoxForDirection(heavyComboHitbox, dir);
    }
    if (getCurrentAnimationName() == "tornado") {
        return getWorldBoxForDirection(tornadoHitbox, dir);
    }
    if (getCurrentAnimationName() == "stomp") {
        return getWorldBoxForDirection(stompHitbox, dir);
    }
    return Entity::getCurrentWorldHitBoxForDirection(dir);
}

void Boss::updateAI(double dt) {
    if (isDead()) {
        setMoveIntent(MoveIntent::None);
        return;
    }

    if (aiAttackCooldown > 0.0) {
        aiAttackCooldown -= dt;
    }

    if (inEntry) {
        setMoveIntent(MoveIntent::None);
        return;
    }

    if (isRolling() || isJumping() || isAttacking()) {
        setMoveIntent(MoveIntent::None);
        return;
    }

    float dx = targetX - getCenterX();
    float absDx = std::abs(dx);

    if (dx > 1.0f) {
        setDirection(Direction::Right);
    }
    else if (dx < -1.0f) {
        setDirection(Direction::Left);
    }

    constexpr float desiredDistance = 230.0f;
    constexpr float attackDistance = 300.0f;

    if (absDx > desiredDistance) {
        if (dx > 0.0f) {
            setMoveIntent(MoveIntent::Right);
        }
        else {
            setMoveIntent(MoveIntent::Left);
        }
        return;
    }

    setMoveIntent(MoveIntent::None);

    if (absDx <= attackDistance && aiAttackCooldown <= 0.0) {
        int chosenAttack = attackChoice(rng);

        if (chosenAttack == 0) {
            heavyAttack1();
        }
        else if (chosenAttack == 1) {
            tornadoAttack();
        }
        else {
            stompAttack();
        }

        aiAttackCooldown = 1.2 + cooldownJitter(rng);
    }
}

void Boss::updateAnimationState() {
    if (inEntry) {
        return;
    }

    Entity::updateAnimationState();
}

void Boss::handleFinishedAnimation() {
    if (inEntry && isCurrentAnimation("entry")) {
        inEntry = false;
        entryFinished = true;
        setAnimation("idle", true);
        return;
    }

    Entity::handleFinishedAnimation();
}

void Boss::update(double dt) {
    updateAI(dt);
    updateEruptions(dt);
    Entity::update(dt);
}

bool intersects(Box a, Box b) {
    return !(a.right < b.left ||
             a.left > b.right ||
             a.bottom < b.top ||
             a.top > b.bottom);
}