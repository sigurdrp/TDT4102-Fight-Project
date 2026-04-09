#include <iostream>
#include <thread>
#include "AnimationWindow.h"
#include "animation.h"
#include "utils.h"


constexpr int WINDOW_X = 300;
constexpr int WINDOW_Y = 300;
constexpr int WINDOW_WIDTH = 500;
constexpr int WINDOW_HEIGHT = 500;
constexpr std::string TITLE = "Window title";


Animation samuraiIdleAnim = loadAnimation("Sprites/Samurai/Idle", "idle", 10);
Animation samuraiRunAnim = loadAnimation("Sprites/Samurai/Run", "run", 16);

std::map<std::string, Animation> samuraiAnimMap{
    {"idle", samuraiIdleAnim},
    {"run", samuraiRunAnim}
};

Animation knightIdleAnim = loadAnimation("Sprites/Knight/Idle", "idle", 10);

std::map<std::string, Animation> knightAnimMap{
    {"idle", knightIdleAnim}
};


int main() {

    TDT4102::AnimationWindow window(WINDOW_X, WINDOW_Y, WINDOW_WIDTH, WINDOW_HEIGHT, TITLE);
    TDT4102::Point point(100, 100);
    TDT4102::Point point2(200, 100);

    Entity knightEntity{point, knightAnimMap};
    Entity samuraiEntity{point2, samuraiAnimMap};
    

    
    auto lastTime = std::chrono::steady_clock::now();
    while(!window.should_close()) {
        bool rKeyIsPressed = window.is_key_down(KeyboardKey::R);
        bool tKeyIsPressed = window.is_key_down(KeyboardKey::T);
        bool yKeyIsPressed = window.is_key_down(KeyboardKey::Y);
        bool dKeyIsPressed = window.is_key_down(KeyboardKey::D);
        
        if (rKeyIsPressed) {
            knightEntity.setAnimation("walk");
        }
        if (tKeyIsPressed) {
            knightEntity.setAnimation("idle");
        }   
        if (yKeyIsPressed) {
            knightEntity.setAnimation("punch");
        }
        if (dKeyIsPressed) {
            knightEntity.move(1, 0);
        }

        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> deltaTime = currentTime - lastTime;
        double dt = deltaTime.count();
        lastTime = currentTime;


        samuraiEntity.draw(window);
        knightEntity.draw(window);

        samuraiEntity.update(dt);
        knightEntity.update(dt);


        window.next_frame();
    }


    return 0;
}

