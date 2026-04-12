#pragma once

#include <vector>
#include "AnimationWindow.h"
#include "animation.h"
#include "utils.h"

class BattleRenderer {
public:
    BattleRenderer();

    void drawMenu(TDT4102::AnimationWindow& window, Player& player, Boss& boss, int totalAttempts);
    void drawBattleScene(TDT4102::AnimationWindow& window, Player& player, Boss& boss, int totalAttempts);

private:
    Animation battleBackgroundAnim;
    std::vector<TDT4102::Image> bossHealthbarFrames;
    std::vector<TDT4102::Image> playerHealthbarFrames;

    void drawSingleFrameImage(TDT4102::AnimationWindow& window,
                              TDT4102::Image& image,
                              TDT4102::Point position,
                              int width,
                              int height,
                              TDT4102::FlipImage flip = TDT4102::FlipImage::NONE);

    void drawBattleBackground(TDT4102::AnimationWindow& window);
    void drawMirrorFloor(TDT4102::AnimationWindow& window);
    void drawReflectedEntity(TDT4102::AnimationWindow& window, Entity& entity);

    int getBossHealthbarFrameIndex(const Boss& boss);
    int getPlayerHealthbarFrameIndex(const Player& player);

    void drawBossHealthbar(TDT4102::AnimationWindow& window, const Boss& boss);
    void drawPlayerHealthbar(TDT4102::AnimationWindow& window, const Player& player);
    void drawAttemptCounter(TDT4102::AnimationWindow& window, int totalAttempts);
};