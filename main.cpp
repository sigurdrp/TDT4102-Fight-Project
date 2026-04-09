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


Animation girlIdleAnim = loadAnimation("Sprites/Brawler-Girl/Idle", "idle", 4);
Animation girlWalkAnim = loadAnimation("Sprites/Brawler-Girl/Walk", "walk", 10);
Animation girlPunchAnim = loadAnimation("Sprites/Brawler-Girl/Punch", "punch", 3);

std::map<std::string, Animation> animationMap{
    {"idle", girlIdleAnim},
    {"walk", girlWalkAnim},
    {"punch", girlPunchAnim}
};



int main() {

    TDT4102::AnimationWindow window(WINDOW_X, WINDOW_Y, WINDOW_WIDTH, WINDOW_HEIGHT, TITLE);
    TDT4102::Point point(100, 100);

    Entity girlEntity{point, animationMap};

    

    
    auto lastTime = std::chrono::steady_clock::now();
    while(!window.should_close()) {
        bool rKeyIsPressed = window.is_key_down(KeyboardKey::R);
        bool tKeyIsPressed = window.is_key_down(KeyboardKey::T);
        bool yKeyIsPressed = window.is_key_down(KeyboardKey::Y);
        bool dKeyIsPressed = window.is_key_down(KeyboardKey::D);
        
        if (rKeyIsPressed) {
            girlEntity.setAnimation("walk");
        }
        if (tKeyIsPressed) {
            girlEntity.setAnimation("idle");
        }   
        if (yKeyIsPressed) {
            girlEntity.setAnimation("punch");
        }
        if (dKeyIsPressed) {
            girlEntity.move(1, 0);
        }

        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> deltaTime = currentTime - lastTime;
        double dt = deltaTime.count();
        lastTime = currentTime;

        girlEntity.draw(window);
        girlEntity.update(dt);


        window.next_frame();
    }


    return 0;
}

