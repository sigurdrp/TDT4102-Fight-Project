#include "animation.h"
#include "utils.h"

Animation loadAnimation(std::string folder, std::string prefix, int frameCount) {
    // Trenger feildetektering
    std::vector<TDT4102::Image> frames;
    for (int i = 1; i <= frameCount; i++) {
        std::string pathToPush{folder + '/' + prefix + std::to_string(i) + ".png"};
        // std::cout << "Path: " << pathToPush << std::endl;
        TDT4102::Image imageToPush(pathToPush);
        frames.push_back(imageToPush);
    }
    return Animation(frames);
}

// // ---------- Boxes ---------- /*
// Box knightHurtbox{50, 43, 61, 79};
// Box knightHitbox {62, 43, 105, 79};

// Box bossHurtbox{108, 55, 148, 111};

// // Manuell justering av bossens angreps-hitbokser:
// Box bossHeavyComboHitbox{120, 55, 220, 150};
// Box bossTornadoHitbox   {50,  40, 200, 155};