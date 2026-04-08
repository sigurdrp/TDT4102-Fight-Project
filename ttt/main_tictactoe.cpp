/**
 * @file main.cpp
 * @brief Entry point for the Tic Tac Toe game.
 * 
 * @details This file uses the following libraries and resources:
 * - TicTacToeWindow.h for the game window class
 */

#include "TicTacToeWindow.h"

/**
 * @brief Initializes and starts the Tic Tac Toe game.
 */
void play_tick_tack_toe() {
    int x = 100;      ///< X-coordinate of the window.
    int y = 50;       ///< Y-coordinate of the window.
    int width = 640;  ///< Width of the window.
    int height = 660; ///< Height of the window.
    std::string title = "Tic Tac Toe"; ///< Title of the game window.

    TicTacToeWindow game(x, y, width, height, title);
    game.play();
}

/**
 * @brief Main function, starts the game.
 * @return int Exit status.
 */
int main() {
    play_tick_tack_toe();
    return 0;
}
