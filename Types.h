#pragma once

enum class MovementState {
    Idle,
    Running,
    Turning,
    Airborne
};

enum class ActionState {
    None,
    Attack,
    Hurt,
    Roll,
    JumpStart,
    JumpLoop,
    JumpLand,
    Death
};

struct Size {
    int width;
    int height;
};

enum class Direction {
    Right,
    Left
};

struct Velocity {
    float x;
    float y;
};

enum class MoveIntent {
    Right,
    None,
    Left
};

struct Position {
    float x;
    float y;
};

struct Box {
    int left;
    int top;
    int right;
    int bottom;

    int width();
    int height();
};

enum class AttackVariant {
    First,
    Second
};