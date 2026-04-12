#pragma once

#include "Entity.h"

class Player : public Entity {
private:
    bool spaceWasPressed = false;
    bool wWasPressed = false;
    bool shiftWasPressed = false;

public:
    using Entity::Entity;

    bool resolveInput(TDT4102::AnimationWindow& window);
};