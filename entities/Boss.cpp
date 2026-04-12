#include "Boss.h"

#include <chrono>
#include <cmath>
#include "Player.h"
#include "Collision.h"

constexpr int ERUPTION_DRAW_WIDTH = 160;
constexpr int ERUPTION_DRAW_HEIGHT = 120;
constexpr double ERUPTION_FRAME_DURATION = 0.055;
constexpr int ERUPTION_ACTIVE_FRAME_START = 4;
constexpr int ERUPTION_ACTIVE_FRAME_END = 9;
constexpr float ERUPTION_SPACING = 150.0f;
constexpr double ERUPTION_INITIAL_DELAY = 0.33;
constexpr double ERUPTION_CHAIN_DELAY = 0.33;

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