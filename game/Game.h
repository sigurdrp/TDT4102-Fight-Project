#pragma once

#include <memory>
#include <vector>
#include <random>
#include <chrono>
#include <map>
#include <string>

#include "AnimationWindow.h"
#include "../Animation.h"
#include "../entities/Player.h"
#include "../entities/Boss.h"
#include "../Types.h"
#include "../utils/utils.h"
#include "../rendering/BattleRenderer.h"

enum class GameState {
    Menu,
    BossEntry,
    Playing,
    PlayerDead,
    BossDead
};

class Game {
public:
    Game();
    void run();

private:
    TDT4102::AnimationWindow window;
    BattleRenderer renderer;

    TDT4102::Audio menuWind;
    TDT4102::Audio bgm;

    std::vector<TDT4102::Audio> swordAttackSounds;
    std::vector<TDT4102::Audio> swordHitSounds;

    TDT4102::Audio spinSound;
    TDT4102::Audio stompSound;

    std::mt19937 rng;

    std::unique_ptr<Player> knightEntity;
    std::unique_ptr<Boss> bossEntity;

    GameState gameState = GameState::Menu;

    bool hWasPressed = false;
    bool jWasPressed = false;
    bool kWasPressed = false;
    bool lWasPressed = false;
    bool uWasPressed = false;

    bool menuWindStarted = false;
    bool bgmStarted = false;
    bool winTimeWritten = false;

    double endStateTimer = 0.0;
    int totalAttempts = 0;

    std::chrono::steady_clock::time_point battleStartTime;

    std::unique_ptr<Player> makePlayer();
    std::unique_ptr<Boss> makeBoss();

    void restartMatch();

    void updateMenu(double dt, std::chrono::steady_clock::time_point currentTime);
    void updateBossEntry(double dt);
    void updatePlaying(double dt, std::chrono::steady_clock::time_point currentTime);
    void updateBossDead(double dt);
    void updatePlayerDead(double dt);

    int randomIndex3();
    bool anyGameplayInputPressed();
};