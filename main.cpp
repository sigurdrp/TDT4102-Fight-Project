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
std::vector<TDT4102::Image> vec{idle1, idle2, idle3, idle4};

int main() {

    TDT4102::AnimationWindow window(WINDOW_X, WINDOW_Y, WINDOW_WIDTH, WINDOW_HEIGHT, TITLE);
    TDT4102::Point point(100, 100);
    Animation girlAnim{vec};
    Animator girlAnimator{girlAnim};
    Entity girlEntity{girlAnimator, point};

    while(!window.should_close()) {
        girlEntity.draw(window);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        girlEntity.update();
        window.next_frame();
    }


    return 0;
}

