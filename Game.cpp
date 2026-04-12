#include "Game.h"
#include "RunStats.h"

#include <iostream>

constexpr int WINDOW_X = 200;
constexpr int WINDOW_Y = 100;
constexpr int WINDOW_WIDTH = 1080;
constexpr int WINDOW_HEIGHT = 640;
constexpr const char* TITLE = "Moonlight VS";

constexpr int GROUND_Y = WINDOW_HEIGHT - 95;
constexpr double RESTART_DELAY_SECONDS = 1.2;

// ---------- Audio ----------
constexpr const char* MENU_WIND_PATH = "Sound/wind.mp3";
constexpr const char* BGM_PATH       = "Sound/bgm.mp3";
constexpr int AUDIO_LOOP_COUNT = 99;

// ---------- Knight animations ----------
namespace {
Animation knightIdleAnim      = loadAnimation("Sprites/Knight/Idle",       "idle",        10);
Animation knightAtkAnim       = loadAnimation("Sprites/Knight/attack1",    "attack",       5);
Animation knightAtk2Anim      = loadAnimation("Sprites/Knight/attack2",    "attack",       5);
Animation knightRunAnim       = loadAnimation("Sprites/Knight/run",        "run",         10);
Animation knightTurnAnim      = loadAnimation("Sprites/Knight/turn",       "turn",         3);
Animation knightHurtAnim      = loadAnimation("Sprites/Knight/Hurt",       "Hurt",         3);
Animation knightJumpStartAnim = loadAnimation("Sprites/Knight/jump_start", "jump_start",   3);
Animation knightJumpLoopAnim  = loadAnimation("Sprites/Knight/jump_loop",  "jump_loop",    3);
Animation knightJumpLandAnim  = loadAnimation("Sprites/Knight/jump_land",  "jump_land",    3);
Animation knightRollAnim      = loadAnimation("Sprites/Knight/roll",       "roll",        12);
Animation knightDeathAnim     = loadAnimation("Sprites/Knight/death",      "death",       10);

std::map<std::string, Animation> knightAnimMap{
    {"idle",       knightIdleAnim},
    {"attack1",    knightAtkAnim},
    {"attack2",    knightAtk2Anim},
    {"run",        knightRunAnim},
    {"turn",       knightTurnAnim},
    {"hurt",       knightHurtAnim},
    {"jump_start", knightJumpStartAnim},
    {"jump_loop",  knightJumpLoopAnim},
    {"jump_land",  knightJumpLandAnim},
    {"roll",       knightRollAnim},
    {"death",      knightDeathAnim}
};

// ---------- Boss animations ----------
Animation bossIdleAnim       = loadAnimation("Sprites/Boss/Idle(TwoHands)", "Idle(TwoHands)", 13);
Animation bossRunAnim        = loadAnimation("Sprites/Boss/Run/Full", "Run", 29);
Animation bossHurtAnim       = loadAnimation("Sprites/Boss/Hurt", "hurt", 3);
Animation bossEntryAnim      = loadAnimation("Sprites/Boss/EntrySecondPhase/Full", "EntrySecondPhase", 20);
Animation bossHeavyComboAnim = loadAnimation("Sprites/Boss/Attacks/HeavyAtks/ComboFull", "HeavyAtks", 64);
Animation bossTornadoAnim    = loadAnimation("Sprites/Boss/Attacks/TornadoAtk/Full", "TornadoAtk", 23);
Animation bossStompAnim      = loadAnimation("Sprites/Boss/Attacks/StompAtk", "StompAtk", 25);
Animation earthEruptionAnim  = loadAnimation("Sprites/Boss/Attacks/StompAtk/EarthEruption", "EarthEruption", 16);
Animation bossDeathAnim      = loadAnimation("Sprites/Boss/Death", "death", 27);

std::map<std::string, Animation> bossAnimMap{
    {"idle",        bossIdleAnim},
    {"run",         bossRunAnim},
    {"hurt",        bossHurtAnim},
    {"entry",       bossEntryAnim},
    {"heavy_combo", bossHeavyComboAnim},
    {"tornado",     bossTornadoAnim},
    {"stomp",       bossStompAnim},
    {"eruption",    earthEruptionAnim},
    {"death",       bossDeathAnim}
};

// ---------- Boxes ----------
Box knightHurtbox{50, 43, 61, 79};
Box knightHitbox {62, 43, 105, 79};

Box bossHurtbox{108, 55, 148, 110};
Box bossHeavyComboHitbox{120, 55, 220, 150};
Box bossTornadoHitbox   {50,  60, 200, 155};
Box bossStompHitbox{92, 110, 100, 155};
Box bossEruptionHitbox{40, 20, 120, 110};
}

Game::Game()
    : window(WINDOW_X, WINDOW_Y, WINDOW_WIDTH, WINDOW_HEIGHT, TITLE),
      renderer(),
      menuWind(MENU_WIND_PATH),
      bgm(BGM_PATH),
      swordAttackSounds{
          TDT4102::Audio("Sound/swordattack1.wav"),
          TDT4102::Audio("Sound/swordattack2.wav"),
          TDT4102::Audio("Sound/swordattack3.wav")
      },
      swordHitSounds{
          TDT4102::Audio("Sound/swordhit1.wav"),
          TDT4102::Audio("Sound/swordhit2.wav"),
          TDT4102::Audio("Sound/swordhit3.wav")
      },
      spinSound("Sound/spin.wav"),
      stompSound("Sound/stomp.wav"),
      rng(static_cast<unsigned int>(
          std::chrono::steady_clock::now().time_since_epoch().count()))
{
    knightEntity = makePlayer();
    bossEntity = makeBoss();
    totalAttempts = RunStats::countAttempts();
    battleStartTime = std::chrono::steady_clock::now();
}

std::unique_ptr<Player> Game::makePlayer() {
    TDT4102::Point playerSpawn{200, GROUND_Y - 80};

    auto player = std::make_unique<Player>(
        playerSpawn, knightAnimMap,
        120, 80, 2.0f,
        knightHurtbox, knightHitbox,
        GROUND_Y
    );
    player->setMaxHealth(4);
    player->setHealthToMax();
    return player;
}

std::unique_ptr<Boss> Game::makeBoss() {
    TDT4102::Point bossSpawn{400, GROUND_Y - 111};

    auto boss = std::make_unique<Boss>(
        bossSpawn, bossAnimMap,
        256, 156, 2.0f,
        bossHurtbox,
        bossHeavyComboHitbox,
        bossTornadoHitbox,
        bossStompHitbox,
        bossEruptionHitbox,
        GROUND_Y
    );
    return boss;
}

int Game::randomIndex3() {
    std::uniform_int_distribution<int> dist(0, 2);
    return dist(rng);
}

bool Game::anyGameplayInputPressed() {
    return
        window.is_key_down(KeyboardKey::A) ||
        window.is_key_down(KeyboardKey::D) ||
        window.is_key_down(KeyboardKey::W) ||
        window.is_key_down(KeyboardKey::SPACE) ||
        window.is_key_down(KeyboardKey::LEFT_SHIFT) ||
        window.is_key_down(KeyboardKey::ENTER) ||
        window.is_key_down(KeyboardKey::H) ||
        window.is_key_down(KeyboardKey::J) ||
        window.is_key_down(KeyboardKey::K) ||
        window.is_key_down(KeyboardKey::L) ||
        window.is_key_down(KeyboardKey::U);
}

void Game::restartMatch() {
    RunStats::appendAttempt();
    ++totalAttempts;

    knightEntity = makePlayer();
    bossEntity = makeBoss();

    hWasPressed = false;
    jWasPressed = false;
    kWasPressed = false;
    lWasPressed = false;
    uWasPressed = false;

    winTimeWritten = false;
    endStateTimer = 0.0;

    battleStartTime = std::chrono::steady_clock::now();
    bossEntity->startEntry();
    gameState = GameState::BossEntry;
}

void Game::updateMenu(double dt, std::chrono::steady_clock::time_point currentTime) {
    if (!menuWindStarted) {
        window.play_audio(menuWind, AUDIO_LOOP_COUNT);
        menuWindStarted = true;
    }

    knightEntity->updateAnimationOnly(dt);
    bossEntity->updateAnimationOnly(dt);

    renderer.drawMenu(window, *knightEntity, *bossEntity, totalAttempts);

    if (anyGameplayInputPressed()) {
        RunStats::appendAttempt();
        ++totalAttempts;

        if (!bgmStarted) {
            window.play_audio(bgm, AUDIO_LOOP_COUNT);
            bgmStarted = true;
        }

        battleStartTime = currentTime;
        bossEntity->startEntry();
        gameState = GameState::BossEntry;
    }
}

void Game::updateBossEntry(double dt) {
    knightEntity->update(dt);
    bossEntity->update(dt);

    if (bossEntity->consumeTornadoStarted()) {
        window.play_audio(spinSound, 1);
    }
    if (bossEntity->consumeStompStarted()) {
        window.play_audio(stompSound, 1);
    }

    renderer.drawBattleScene(window, *knightEntity, *bossEntity, totalAttempts);

    if (bossEntity->hasFinishedEntry()) {
        gameState = GameState::Playing;
    }
}

void Game::updatePlaying(double dt, std::chrono::steady_clock::time_point currentTime) {
    bool hPressed = window.is_key_down(KeyboardKey::H);
    bool hPressedNow = hPressed && !hWasPressed;

    bool jPressed = window.is_key_down(KeyboardKey::J);
    bool kPressed = window.is_key_down(KeyboardKey::K);
    bool lPressed = window.is_key_down(KeyboardKey::L);
    bool uPressed = window.is_key_down(KeyboardKey::U);

    bool jPressedNow = jPressed && !jWasPressed;
    bool kPressedNow = kPressed && !kWasPressed;
    bool lPressedNow = lPressed && !lWasPressed;
    bool uPressedNow = uPressed && !uWasPressed;

    if (hPressedNow) {
        knightEntity->takeDamage(1, bossEntity->getDirection());
    }

    if (jPressedNow) {
        bossEntity->heavyAttack1();
    }
    if (kPressedNow) {
        bossEntity->tornadoAttack();
    }
    if (lPressedNow) {
        bossEntity->stompAttack();
    }
    if (uPressedNow) {
        bossEntity->heavyAttack1();
    }

    bool playerAttackStarted = knightEntity->resolveInput(window);
    if (playerAttackStarted) {
        window.play_audio(swordAttackSounds.at(randomIndex3()), 1);
    }

    bossEntity->setTargetX(knightEntity->getCenterX());

    knightEntity->update(dt);
    bossEntity->update(dt);

    if (bossEntity->consumeTornadoStarted()) {
        window.play_audio(spinSound, 1);
    }
    if (bossEntity->consumeStompStarted()) {
        window.play_audio(stompSound, 1);
    }

    if (knightEntity->isHitboxActive() &&
        !knightEntity->hasAlreadyHit() &&
        bossEntity->hasActiveHurtbox() &&
        intersects(knightEntity->getWorldHitBox(), bossEntity->getWorldHurtBox())) {
        knightEntity->markHit();
        bossEntity->takeDamage(10, knightEntity->getDirection());
        window.play_audio(swordHitSounds.at(randomIndex3()), 1);
        std::cout << "Boss HP: " << bossEntity->getHealth() << '\n';
    }

    if (bossEntity->isHitboxActive() &&
        !bossEntity->hasAlreadyHit() &&
        knightEntity->hasActiveHurtbox()) {

        bool hit = false;

        if (bossEntity->getCurrentAnimationName() == "tornado") {
            hit =
                intersects(bossEntity->getCurrentWorldHitBoxForDirection(Direction::Right), knightEntity->getWorldHurtBox()) ||
                intersects(bossEntity->getCurrentWorldHitBoxForDirection(Direction::Left), knightEntity->getWorldHurtBox());
        }
        else {
            hit = intersects(bossEntity->getCurrentWorldHitBox(), knightEntity->getWorldHurtBox());
        }

        if (hit) {
            bossEntity->markHit();
            knightEntity->takeDamage(1, bossEntity->getDirection());
            window.play_audio(swordHitSounds.at(randomIndex3()), 1);
            std::cout << "Player HP: " << knightEntity->getHealth() << '\n';
        }
    }

    if (knightEntity->hasActiveHurtbox()) {
        if (bossEntity->tryHitPlayerWithEruptions(*knightEntity)) {
            window.play_audio(swordHitSounds.at(randomIndex3()), 1);
            std::cout << "Player HP: " << knightEntity->getHealth() << '\n';
        }
    }

    if (bossEntity->getHealth() <= 0 && !bossEntity->isDead()) {
        bossEntity->die();

        if (!winTimeWritten) {
            std::chrono::duration<double> fightDuration = currentTime - battleStartTime;
            RunStats::appendWinTime(fightDuration.count());
            winTimeWritten = true;
        }

        knightEntity->forceIdleLoop();
        endStateTimer = 0.0;
        gameState = GameState::BossDead;
    }
    else if (knightEntity->getHealth() <= 0 && !knightEntity->isDead()) {
        knightEntity->die();
        bossEntity->forceIdleLoop();
        endStateTimer = 0.0;
        gameState = GameState::PlayerDead;
    }

    renderer.drawBattleScene(window, *knightEntity, *bossEntity, totalAttempts);

    hWasPressed = hPressed;
    jWasPressed = jPressed;
    kWasPressed = kPressed;
    lWasPressed = lPressed;
    uWasPressed = uPressed;
}

void Game::updateBossDead(double dt) {
    endStateTimer += dt;

    knightEntity->updateAnimationOnly(dt);
    bossEntity->update(dt);

    renderer.drawBattleScene(window, *knightEntity, *bossEntity, totalAttempts);

    if (endStateTimer >= RESTART_DELAY_SECONDS && anyGameplayInputPressed()) {
        restartMatch();
    }
}

void Game::updatePlayerDead(double dt) {
    endStateTimer += dt;

    knightEntity->update(dt);
    bossEntity->updateAnimationOnly(dt);

    renderer.drawBattleScene(window, *knightEntity, *bossEntity, totalAttempts);

    if (endStateTimer >= RESTART_DELAY_SECONDS && anyGameplayInputPressed()) {
        restartMatch();
    }
}

void Game::run() {
    auto lastTime = std::chrono::steady_clock::now();

    while (!window.should_close()) {
        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> deltaTime = currentTime - lastTime;
        double dt = deltaTime.count();
        lastTime = currentTime;

        switch (gameState) {
        case GameState::Menu:
            updateMenu(dt, currentTime);
            break;
        case GameState::BossEntry:
            updateBossEntry(dt);
            break;
        case GameState::Playing:
            updatePlaying(dt, currentTime);
            break;
        case GameState::BossDead:
            updateBossDead(dt);
            break;
        case GameState::PlayerDead:
            updatePlayerDead(dt);
            break;
        }

        window.next_frame();
    }
}