#include "BattleRenderer.h"

constexpr int WINDOW_WIDTH = 1080;
constexpr int WINDOW_HEIGHT = 640;
constexpr int GROUND_Y = WINDOW_HEIGHT - 95;
constexpr int MIRROR_LINE_Y = GROUND_Y - 10;
constexpr int VISUAL_FLOOR_Y = MIRROR_LINE_Y;

BattleRenderer::BattleRenderer()
    : battleBackgroundAnim(loadAnimation("Sprites/Menu", "battle_bg", 1)),
      bossHealthbarFrames{
          TDT4102::Image("Sprites/Menu/healthbar1.png"),
          TDT4102::Image("Sprites/Menu/healthbar2.png"),
          TDT4102::Image("Sprites/Menu/healthbar3.png"),
          TDT4102::Image("Sprites/Menu/healthbar4.png"),
          TDT4102::Image("Sprites/Menu/healthbar5.png"),
          TDT4102::Image("Sprites/Menu/healthbar6.png"),
          TDT4102::Image("Sprites/Menu/healthbar7.png"),
          TDT4102::Image("Sprites/Menu/healthbar8.png")
      },
      playerHealthbarFrames{
          TDT4102::Image("Sprites/PlayerHealthbar/healthbar1.png"),
          TDT4102::Image("Sprites/PlayerHealthbar/healthbar2.png"),
          TDT4102::Image("Sprites/PlayerHealthbar/healthbar3.png"),
          TDT4102::Image("Sprites/PlayerHealthbar/healthbar4.png")
      }
{
}

void BattleRenderer::drawSingleFrameImage(TDT4102::AnimationWindow& window,
                                          TDT4102::Image& image,
                                          TDT4102::Point position,
                                          int width,
                                          int height,
                                          TDT4102::FlipImage flip) {
    window.draw_image_region(position, image, width, height, {0, 0}, width, height, flip);
}

void BattleRenderer::drawBattleBackground(TDT4102::AnimationWindow& window) {
    drawSingleFrameImage(window,
                         battleBackgroundAnim.getFrame(0),
                         {0, 0},
                         WINDOW_WIDTH,
                         WINDOW_HEIGHT);
}

void BattleRenderer::drawMirrorFloor(TDT4102::AnimationWindow& window) {
    const int floorTop = VISUAL_FLOOR_Y;
    const int floorHeight = WINDOW_HEIGHT - floorTop;

    window.draw_rectangle({0, floorTop}, WINDOW_WIDTH, floorHeight, TDT4102::Color{14, 19, 39});
}

void BattleRenderer::drawReflectedEntity(TDT4102::AnimationWindow& window, Entity& entity) {
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

int BattleRenderer::getBossHealthbarFrameIndex(const Boss& boss) {
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

int BattleRenderer::getPlayerHealthbarFrameIndex(const Player& player) {
    int hp = player.getHealth();

    if (hp >= 4) return 3;
    if (hp == 3) return 2;
    if (hp == 2) return 1;
    return 0;
}

void BattleRenderer::drawBossHealthbar(TDT4102::AnimationWindow& window, const Boss& boss) {
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

void BattleRenderer::drawPlayerHealthbar(TDT4102::AnimationWindow& window, const Player& player) {
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

void BattleRenderer::drawAttemptCounter(TDT4102::AnimationWindow& window, int totalAttempts) {
    window.draw_text(
        {860, 20},
        "Attempt: " + std::to_string(totalAttempts),
        TDT4102::Color::white,
        20
    );
}

void BattleRenderer::drawMenu(TDT4102::AnimationWindow& window, Player& player, Boss& boss, int totalAttempts) {
    drawBattleBackground(window);
    drawMirrorFloor(window);

    drawReflectedEntity(window, player);
    drawReflectedEntity(window, boss);

    drawBossHealthbar(window, boss);
    drawPlayerHealthbar(window, player);

    player.draw(window);
    boss.draw(window);

    drawAttemptCounter(window, totalAttempts);
}

void BattleRenderer::drawBattleScene(TDT4102::AnimationWindow& window, Player& player, Boss& boss, int totalAttempts) {
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