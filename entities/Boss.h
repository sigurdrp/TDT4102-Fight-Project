#pragma once

#include <vector>
#include <random>
#include "Entity.h"

class Player;

class Boss : public Entity {
private:
    struct EruptionInstance {
        float x = 0.0f;
        double startDelay = 0.0;
        double elapsed = 0.0;
        bool finished = false;
        bool hasHitPlayer = false;
    };

    bool inEntry = false;
    bool entryFinished = false;
    bool tornadoStartedThisFrame = false;
    bool stompStartedThisFrame = false;

    float targetX = 0.0f;
    double aiAttackCooldown = 0.0;
    std::mt19937 rng;
    std::uniform_int_distribution<int> attackChoice{0, 2};
    std::uniform_real_distribution<double> cooldownJitter{0.0, 0.6};

    Box heavyComboHitbox;
    Box tornadoHitbox;
    Box stompHitbox;
    Box eruptionHitbox;

    std::vector<EruptionInstance> eruptions;

    void updateAI(double dt);
    void updateEruptions(double dt);
    void spawnStompEruptionWave();

public:
    Boss(TDT4102::Point pos, std::map<std::string, Animation> animMap,
         int spriteWidth, int spriteHeight, float scl,
         Box hrtbx, Box heavyComboHtbx, Box tornadoHtbx, Box stompHtbx, Box eruptionHtbx, int groundY);

    void startEntry();
    bool hasFinishedEntry();

    void setTargetX(float x);

    bool heavyAttack1();
    bool heavyAttack2();
    bool heavyAttack3();
    bool tornadoAttack();
    bool stompAttack();

    bool consumeTornadoStarted();
    bool consumeStompStarted();

    void drawEruptions(TDT4102::AnimationWindow& window);
    bool tryHitPlayerWithEruptions(Player& player);

    Box getCurrentWorldHitBox() override;
    Box getCurrentWorldHitBoxForDirection(Direction dir) override;

    void updateAnimationState() override;
    void handleFinishedAnimation() override;
    void update(double dt) override;
};