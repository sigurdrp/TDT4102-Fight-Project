#include <iostream>
#include <thread>
#include "AnimationWindow.h"


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

class Animation {
    private:
        std::vector<TDT4102::Image> frames;
    public:
        TDT4102::Image& getFrame(int index) {
            return frames.at(index);
        }
        
        int size() {
            return frames.size();
        }
        Animation(std::vector<TDT4102::Image> frames) : frames{frames} {};
};

class Animator {
    private:
        Animation& animation;
        int currentFrame;
    public:
        TDT4102::Image& getCurrentImage() {
            return animation.getFrame(currentFrame);
        }

        void nextFrame() {
            currentFrame = (currentFrame + 1) % animation.size();
        }

        Animator(Animation& anim) : animation(anim), currentFrame(0) {} // Initializer list 

};


int main() {

    TDT4102::AnimationWindow window(WINDOW_X, WINDOW_Y, WINDOW_WIDTH, WINDOW_HEIGHT, TITLE);
    Animation girlAnim{vec};
    Animator girlAnimator{girlAnim};

    while(!window.should_close()) {
        TDT4102::Point topLeftCorner {50, 100};

        window.draw_image(topLeftCorner, girlAnimator.getCurrentImage());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        girlAnimator.nextFrame();

        window.next_frame();
    }


    return 0;
}

