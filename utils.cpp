#include "animation.h"
#include "utils.h"

Animation loadAnimation(std::string folder, std::string prefix, int frameCount) {
    // Trenger feildetektering
    std::vector<TDT4102::Image> frames;
    for (int i = 1; i <= frameCount; i++) {
        std::string pathToPush{folder + '/' + prefix + std::to_string(i) + ".png"};
        std::cout << "Path: " << pathToPush << std::endl;
        TDT4102::Image imageToPush(pathToPush);
        frames.push_back(imageToPush);
    }
    return Animation(frames);
}