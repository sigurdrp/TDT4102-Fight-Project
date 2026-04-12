#include "Player.h"

bool Player::resolveInput(TDT4102::AnimationWindow& window) {
    bool aPressed = window.is_key_down(KeyboardKey::A);
    bool dPressed = window.is_key_down(KeyboardKey::D);
    bool spacePressed = window.is_key_down(KeyboardKey::SPACE);
    bool wPressed = window.is_key_down(KeyboardKey::W);
    bool shiftPressed = window.is_key_down(KeyboardKey::LEFT_SHIFT);

    bool spacePressedNow = spacePressed && !spaceWasPressed;
    bool wPressedNow = wPressed && !wWasPressed;
    bool shiftPressedNow = shiftPressed && !shiftWasPressed;

    bool startedAttack = false;

    if (isDead()) {
        setMoveIntent(MoveIntent::None);
        spaceWasPressed = spacePressed;
        wWasPressed = wPressed;
        shiftWasPressed = shiftPressed;
        return false;
    }

    if (isStunned()) {
        setMoveIntent(MoveIntent::None);
        spaceWasPressed = spacePressed;
        wWasPressed = wPressed;
        shiftWasPressed = shiftPressed;
        return false;
    }

    if (isRolling() || isJumping()) {
        setMoveIntent(MoveIntent::None);
        spaceWasPressed = spacePressed;
        wWasPressed = wPressed;
        shiftWasPressed = shiftPressed;
        return false;
    }

    if (aPressed && !dPressed) {
        setMoveIntent(MoveIntent::Left);
    }
    else if (!aPressed && dPressed) {
        setMoveIntent(MoveIntent::Right);
    }
    else {
        setMoveIntent(MoveIntent::None);
    }

    if (wPressedNow) {
        jump();
    }

    if (shiftPressedNow) {
        roll();
    }

    if (spacePressedNow) {
        startedAttack = attack();
    }

    spaceWasPressed = spacePressed;
    wWasPressed = wPressed;
    shiftWasPressed = shiftPressed;

    return startedAttack;
}