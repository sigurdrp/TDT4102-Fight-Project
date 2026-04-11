#include <iostream>
#include <thread>
#include "AnimationWindow.h"
#include "animation.h"
#include "utils.h"


constexpr int WINDOW_X = 300;
constexpr int WINDOW_Y = 300;
constexpr int WINDOW_WIDTH = 500;
constexpr int WINDOW_HEIGHT = 500;
constexpr std::string TITLE = "Minotaur VS";


Animation samuraiIdleAnim = loadAnimation("Sprites/Samurai/Idle", "idle", 10);
Animation samuraiRunAnim = loadAnimation("Sprites/Samurai/Run", "run", 16);

std::map<std::string, Animation> samuraiAnimMap{
    {"idle", samuraiIdleAnim},
    {"run", samuraiRunAnim}
};

Animation knightIdleAnim = loadAnimation("Sprites/Knight/Idle", "idle", 10);
Animation knightAtkAnim = loadAnimation("Sprites/Knight/attack1", "attack", 5);
Animation knightAtk2Anim = loadAnimation("Sprites/Knight/attack2", "attack", 5);
Animation knightRunAnim = loadAnimation("Sprites/Knight/run", "run", 10);
Animation knightTurnAnim = loadAnimation("Sprites/Knight/turn", "turn", 3);

std::map<std::string, Animation> knightAnimMap{
    {"idle", knightIdleAnim},
    {"attack1", knightAtkAnim},
    {"attack2", knightAtk2Anim},
    {"run", knightRunAnim},
    {"turn", knightTurnAnim}
};

Box knightHurtbox{50, 43, 61, 79}; // ingen bedre måte å gjøre dette ;-; magic numbers


int main() {

    TDT4102::AnimationWindow window(WINDOW_X, WINDOW_Y, WINDOW_WIDTH, WINDOW_HEIGHT, TITLE);
    TDT4102::Point point(0, 100);
    TDT4102::Point point2(200, 100);

    Entity knightEntity{point, knightAnimMap, 120, 80, 2, knightHurtbox};

    bool spaceWasPressed = false;

    auto lastTime = std::chrono::steady_clock::now();
    while(!window.should_close()) {
        bool aPressed = window.is_key_down(KeyboardKey::A);
        bool dPressed = window.is_key_down(KeyboardKey::D);
        bool spacePressed = window.is_key_down(KeyboardKey::SPACE);

        bool spacePressedNow = (spacePressed && !spaceWasPressed);
        
        // --- Deltatime-management --- //
        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> deltaTime = currentTime - lastTime;
        double dt = deltaTime.count();
        lastTime = currentTime;
        // --- Deltatime-management --- //

        if (aPressed && !dPressed) {
            knightEntity.setMoveIntent(MoveIntent::Left);
        }
        else if (!aPressed && dPressed) {
            knightEntity.setMoveIntent(MoveIntent::Right);
        }
        else {
            knightEntity.setMoveIntent(MoveIntent::None);
        }

        if(spacePressedNow) {
            knightEntity.attack();
        }

        knightEntity.update(dt);
        knightEntity.draw(window);
        // knightEntity.drawHurtbox(window);


        window.next_frame();
        spaceWasPressed = spacePressed;
    }


    return 0;
}

