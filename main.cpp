#include <iostream>
#include <thread>
#include "AnimationWindow.h"
#include "animation.h"


constexpr int WINDOW_X = 300;
constexpr int WINDOW_Y = 300;
constexpr int WINDOW_WIDTH = 500;
constexpr int WINDOW_HEIGHT = 500;
constexpr std::string TITLE = "Window title";

TDT4102::Image idle1("Sprites/Brawler-Girl/Idle/idle1.png");
TDT4102::Image idle2("Sprites/Brawler-Girl/Idle/idle2.png");
TDT4102::Image idle3("Sprites/Brawler-Girl/Idle/idle3.png");
TDT4102::Image idle4("Sprites/Brawler-Girl/Idle/idle4.png");

TDT4102::Image walk1("Sprites/Brawler-Girl/Walk/walk1.png");
TDT4102::Image walk2("Sprites/Brawler-Girl/Walk/walk2.png");
TDT4102::Image walk3("Sprites/Brawler-Girl/Walk/walk3.png");
TDT4102::Image walk4("Sprites/Brawler-Girl/Walk/walk4.png");
TDT4102::Image walk5("Sprites/Brawler-Girl/Walk/walk5.png");
TDT4102::Image walk6("Sprites/Brawler-Girl/Walk/walk6.png");
TDT4102::Image walk7("Sprites/Brawler-Girl/Walk/walk7.png");
TDT4102::Image walk8("Sprites/Brawler-Girl/Walk/walk8.png");
TDT4102::Image walk9("Sprites/Brawler-Girl/Walk/walk9.png");
TDT4102::Image walk10("Sprites/Brawler-Girl/Walk/walk10.png");


std::vector<TDT4102::Image> idleVec{idle1, idle2, idle3, idle4};
std::vector<TDT4102::Image> walkVec{walk1, walk2, walk3, walk4, walk5, walk6, walk7, walk8, walk9, walk10};

Animation girlIdleAnim{idleVec};
Animation girlWalkAnim{walkVec};
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

    while(!window.should_close()) {

        bool rKeyIsPressed = window.is_key_down(KeyboardKey::R);
        bool tKeyIsPressed = window.is_key_down(KeyboardKey::T);
        bool yKeyIsPressed = window.is_key_down(KeyboardKey::Y);
        if (rKeyIsPressed) {
            girlEntity.setAnimation("walk");
        }
        if (tKeyIsPressed) {
            girlEntity.setAnimation("idle");
        }   
        if (yKeyIsPressed) {
            girlEntity.setAnimation("punch");
        }


        girlEntity.draw(window);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        girlEntity.update();
        window.next_frame();
    }


    return 0;
}

