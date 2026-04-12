#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <random>
#include <fstream>
#include <iomanip>
#include <memory>
#include "AnimationWindow.h"
#include "animation.h"
#include "utils.h"

constexpr int WINDOW_X = 200;
constexpr int WINDOW_Y = 100;
constexpr int WINDOW_WIDTH = 1080;
constexpr int WINDOW_HEIGHT = 640;
constexpr std::string TITLE = "Moonlight VS";

constexpr int GROUND_Y = WINDOW_HEIGHT - 95;

// Separert fysisk gulv og visuell speillinje
constexpr int MIRROR_LINE_Y = GROUND_Y - 10;
constexpr int VISUAL_FLOOR_Y = MIRROR_LINE_Y;

constexpr double RESTART_DELAY_SECONDS = 1.2;

// ---------- Background ----------
Animation battleBackgroundAnim = loadAnimation("Sprites/Menu", "battle_bg", 1);

// ---------- Boss healthbar ----------
std::vector<TDT4102::Image> bossHealthbarFrames{
    TDT4102::Image("Sprites/Menu/healthbar1.png"),
    TDT4102::Image("Sprites/Menu/healthbar2.png"),
    TDT4102::Image("Sprites/Menu/healthbar3.png"),
    TDT4102::Image("Sprites/Menu/healthbar4.png"),
    TDT4102::Image("Sprites/Menu/healthbar5.png"),
    TDT4102::Image("Sprites/Menu/healthbar6.png"),
    TDT4102::Image("Sprites/Menu/healthbar7.png"),
    TDT4102::Image("Sprites/Menu/healthbar8.png")
};

// ---------- Player healthbar ----------
std::vector<TDT4102::Image> playerHealthbarFrames{
    TDT4102::Image("Sprites/PlayerHealthbar/healthbar1.png"),
    TDT4102::Image("Sprites/PlayerHealthbar/healthbar2.png"),
    TDT4102::Image("Sprites/PlayerHealthbar/healthbar3.png"),
    TDT4102::Image("Sprites/PlayerHealthbar/healthbar4.png")
};

// ---------- Audio ----------
constexpr const char* MENU_WIND_PATH = "Sound/wind.mp3";
constexpr const char* BGM_PATH       = "Sound/bgm.mp3";
constexpr int AUDIO_LOOP_COUNT = 99;

// ---------- Knight animations ----------
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

enum class GameState {
    Menu,
    BossEntry,
    Playing,
    PlayerDead,
    BossDead
};

void appendWinTimeToFile(double seconds) {
    std::ofstream file("timesToWin.txt", std::ios::app);
    if (!file) {
        std::cout << "Could not open timesToWin.txt for writing.\n";
        return;
    }

    file << std::fixed << std::setprecision(2)
         << "Win time: " << seconds << " seconds\n";
}

void drawSingleFrameImage(TDT4102::AnimationWindow& window,
                          TDT4102::Image& image,
                          TDT4102::Point position,
                          int width,
                          int height,
                          TDT4102::FlipImage flip = TDT4102::FlipImage::NONE) {
    window.draw_image_region(position, image, width, height, {0, 0}, width, height, flip);
}

void drawBattleBackground(TDT4102::AnimationWindow& window) {
    drawSingleFrameImage(window,
                         battleBackgroundAnim.getFrame(0),
                         {0, 0},
                         WINDOW_WIDTH,
                         WINDOW_HEIGHT);
}

void drawMirrorFloor(TDT4102::AnimationWindow& window) {
    const int floorTop = VISUAL_FLOOR_Y;
    const int floorHeight = WINDOW_HEIGHT - floorTop;

    window.draw_rectangle({0, floorTop}, WINDOW_WIDTH, floorHeight, TDT4102::Color{14, 19, 39});
}

void drawReflectedEntity(TDT4102::AnimationWindow& window, Entity& entity) {
    int x = static_cast<int>(entity.getPosition().x);
    int w = entity.getSpriteWidth();
    int h = entity.getSpriteHeight();

    int reflectedY = 2 * MIRROR_LINE_Y - (static_cast<int>(entity.getPosition().y) + h);

    TDT4102::FlipImage flip =
        (entity.getDirection() == Direction::Right)
            ? TDT4102::FlipImage::VERTICAL
            : TDT4102::FlipImage::BOTH;

    drawSingleFrameImage(window,
                         entity.getCurrentImage(),
                         {x, reflectedY},
                         w,
                         static_cast<int>(h * 0.8f),
                         flip);
}

int getBossHealthbarFrameIndex(const Boss& boss) {
    float ratio = static_cast<float>(boss.getHealth()) / static_cast<float>(boss.getMaxHealth());

    if (ratio > 7.0f / 8.0f) return 7;
    if (ratio > 6.0f / 8.0f) return 6;
    if (ratio > 5.0f / 8.0f) return 5;
    if (ratio > 4.0f / 8.0f) return 4;
    if (ratio > 3.0f / 8.0f) return 3;
    if (ratio > 2.0f / 8.0f) return 2;
    if (ratio > 1.0f / 8.0f) return 1;
    return 0;
}

int getPlayerHealthbarFrameIndex(const Player& player) {
    int hp = player.getHealth();

    if (hp >= 4) return 3;
    if (hp == 3) return 2;
    if (hp == 2) return 1;
    return 0;
}

void drawBossHealthbar(TDT4102::AnimationWindow& window, const Boss& boss) {
    constexpr int BAR_WIDTH = 360;
    constexpr int BAR_HEIGHT = 64;

    const int frameIndex = getBossHealthbarFrameIndex(boss);
    TDT4102::Point position{
        (WINDOW_WIDTH - BAR_WIDTH) / 2,
        20
    };

    drawSingleFrameImage(window,
                         const_cast<TDT4102::Image&>(bossHealthbarFrames.at(frameIndex)),
                         position,
                         BAR_WIDTH,
                         BAR_HEIGHT);
}

void drawPlayerHealthbar(TDT4102::AnimationWindow& window, const Player& player) {
    constexpr int BAR_WIDTH = 160;
    constexpr int BAR_HEIGHT = 18;

    const int frameIndex = getPlayerHealthbarFrameIndex(player);
    TDT4102::Point position{
        (WINDOW_WIDTH - BAR_WIDTH) / 2,
        WINDOW_HEIGHT - 28
    };

    drawSingleFrameImage(window,
                         const_cast<TDT4102::Image&>(playerHealthbarFrames.at(frameIndex)),
                         position,
                         BAR_WIDTH,
                         BAR_HEIGHT);
}

void drawAttemptCounter(TDT4102::AnimationWindow& window, int totalAttempts) {
    window.draw_text(
        {860, 20},
        "Attempt: " + std::to_string(totalAttempts),
        TDT4102::Color::white,
        20
    );
}

void drawBattleScene(TDT4102::AnimationWindow& window, Player& player, Boss& boss, int totalAttempts) {
    drawBattleBackground(window);
    drawMirrorFloor(window);

    drawReflectedEntity(window, player);
    drawReflectedEntity(window, boss);

    player.draw(window);
    boss.draw(window);

    boss.drawEruptions(window);

    drawBossHealthbar(window, boss);
    drawPlayerHealthbar(window, player);
    drawAttemptCounter(window, totalAttempts);
}

int randomIndex3(std::mt19937& rng) {
    std::uniform_int_distribution<int> dist(0, 2);
    return dist(rng);
}

bool anyGameplayInputPressed(TDT4102::AnimationWindow& window) {
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

int countAttemptsFromFile() {
    std::ifstream file("attempts.txt");
    if (!file) {
        return 0;
    }

    int count = 0;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            ++count;
        }
    }
    return count;
}

void appendAttemptToFile() {
    std::ofstream file("attempts.txt", std::ios::app);
    if (!file) {
        std::cout << "Could not open attempts.txt\n";
        return;
    }

    file << "attempt\n";
}


int main() {
    TDT4102::AnimationWindow window(WINDOW_X, WINDOW_Y, WINDOW_WIDTH, WINDOW_HEIGHT, TITLE);

    TDT4102::Audio menuWind(MENU_WIND_PATH);
    TDT4102::Audio bgm(BGM_PATH);

    std::vector<TDT4102::Audio> swordAttackSounds{
        TDT4102::Audio("Sound/swordattack1.wav"),
        TDT4102::Audio("Sound/swordattack2.wav"),
        TDT4102::Audio("Sound/swordattack3.wav")
    };

    std::vector<TDT4102::Audio> swordHitSounds{
        TDT4102::Audio("Sound/swordhit1.wav"),
        TDT4102::Audio("Sound/swordhit2.wav"),
        TDT4102::Audio("Sound/swordhit3.wav")
    };

    TDT4102::Audio spinSound("Sound/spin.wav");
    TDT4102::Audio stompSound("Sound/stomp.wav");

    std::mt19937 rng(static_cast<unsigned int>(
        std::chrono::steady_clock::now().time_since_epoch().count()));

    auto makePlayer = [&]() -> std::unique_ptr<Player> {
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
    };

    auto makeBoss = [&]() -> std::unique_ptr<Boss> {
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
    };

    std::unique_ptr<Player> knightEntity = makePlayer();
    std::unique_ptr<Boss> bossEntity = makeBoss();

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

    auto battleStartTime = std::chrono::steady_clock::now();

    int totalAttempts = countAttemptsFromFile();
    auto restartMatch = [&]() {
        appendAttemptToFile();
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
    };

    auto lastTime = std::chrono::steady_clock::now();
    while (!window.should_close()) {
        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> deltaTime = currentTime - lastTime;
        double dt = deltaTime.count();
        lastTime = currentTime;

        if (gameState == GameState::Menu && !menuWindStarted) {
            window.play_audio(menuWind, AUDIO_LOOP_COUNT);
            menuWindStarted = true;
        }

        if (gameState == GameState::Menu) {
            knightEntity->updateAnimationOnly(dt);
            bossEntity->updateAnimationOnly(dt);

            drawBattleBackground(window);
            drawMirrorFloor(window);

            drawReflectedEntity(window, *knightEntity);
            drawReflectedEntity(window, *bossEntity);

            drawBossHealthbar(window, *bossEntity);
            drawPlayerHealthbar(window, *knightEntity);

            knightEntity->draw(window);
            bossEntity->draw(window);

            drawAttemptCounter(window, totalAttempts);

            if (anyGameplayInputPressed(window)) {
                appendAttemptToFile();
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
        else if (gameState == GameState::BossEntry) {
            knightEntity->update(dt);
            bossEntity->update(dt);

            if (bossEntity->consumeTornadoStarted()) {
                window.play_audio(spinSound, 1);
            }
            if (bossEntity->consumeStompStarted()) {
                window.play_audio(stompSound, 1);
            }

            drawBattleScene(window, *knightEntity, *bossEntity, totalAttempts);

            if (bossEntity->hasFinishedEntry()) {
                gameState = GameState::Playing;
            }
        }
        else if (gameState == GameState::Playing) {
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
                window.play_audio(swordAttackSounds.at(randomIndex3(rng)), 1);
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
                window.play_audio(swordHitSounds.at(randomIndex3(rng)), 1);
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
                    window.play_audio(swordHitSounds.at(randomIndex3(rng)), 1);
                    std::cout << "Player HP: " << knightEntity->getHealth() << '\n';
                }
            }

            if (knightEntity->hasActiveHurtbox()) {
                if (bossEntity->tryHitPlayerWithEruptions(*knightEntity)) {
                    window.play_audio(swordHitSounds.at(randomIndex3(rng)), 1);
                    std::cout << "Player HP: " << knightEntity->getHealth() << '\n';
                }
            }

            if (bossEntity->getHealth() <= 0 && !bossEntity->isDead()) {
                bossEntity->die();

                if (!winTimeWritten) {
                    std::chrono::duration<double> fightDuration = currentTime - battleStartTime;
                    appendWinTimeToFile(fightDuration.count());
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

            drawBattleScene(window, *knightEntity, *bossEntity, totalAttempts);

            hWasPressed = hPressed;
            jWasPressed = jPressed;
            kWasPressed = kPressed;
            lWasPressed = lPressed;
            uWasPressed = uPressed;
        }
        else if (gameState == GameState::BossDead) {
            endStateTimer += dt;

            knightEntity->updateAnimationOnly(dt);
            bossEntity->update(dt);

            drawBattleScene(window, *knightEntity, *bossEntity, totalAttempts);

            if (endStateTimer >= RESTART_DELAY_SECONDS && anyGameplayInputPressed(window)) {
                restartMatch();
            }
        }
        else if (gameState == GameState::PlayerDead) {
            endStateTimer += dt;

            knightEntity->update(dt);
            bossEntity->updateAnimationOnly(dt);

            drawBattleScene(window, *knightEntity, *bossEntity, totalAttempts);

            if (endStateTimer >= RESTART_DELAY_SECONDS && anyGameplayInputPressed(window)) {
                restartMatch();
            }
        }

        window.next_frame();
    }

    return 0;
}