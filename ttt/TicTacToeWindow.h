/**
 * @file TicTacToeWindow.h
 * @brief Defines the TicTacToeWindow class for handling game logic and UI.
 * 
 * @details This file uses the following libraries and resources:
 * - AnimationWindow from TDT4102
 * - Button widget from TDT4102
 * - Standard C++ libraries: vector, fstream, stdexcept
 */

#pragma once

#include "AnimationWindow.h"
#include "widgets/Button.h"
#include <vector>
#include <fstream>
#include <stdexcept>

using namespace TDT4102;

/**
 * @class TicTacToeWindow
 * @brief Handles the Tic Tac Toe game logic and user interface.
 */
class TicTacToeWindow : public TDT4102::AnimationWindow {
public:
    /**
     * @brief Constructs a TicTacToeWindow object.
     * @param x X-coordinate of the window.
     * @param y Y-coordinate of the window.
     * @param width Width of the window.
     * @param height Height of the window.
     * @param title Title of the window.
     */
    TicTacToeWindow(int x, int y, int width, int height, const std::string& title);

    /**
     * @brief Starts the game loop.
     */
    void play();

private:
    /**
     * @brief Draws the Tic Tac Toe board.
     */
    void draw_board();

    /**
     * @brief Draws the marks (X and O) on the board.
     */
    void draw_marks();

    /**
     * @brief Draws a line to indicate the winning combination.
     */
    void draw_winning_line();

    /**
     * @brief Handles mouse click events to place marks.
     * @param x X-coordinate of the click.
     * @param y Y-coordinate of the click.
     */
    void handle_click(int x, int y);

    /**
     * @brief Checks if there is a winner.
     * @return true if a player has won, false otherwise.
     */
    bool check_winner();

    /**
     * @brief Checks if the game is a draw.
     * @return true if the game is a draw, false otherwise.
     */
    bool check_draw();

    /**
     * @brief Resets the game to its initial state.
     */
    void reset_game();

    /**
     * @brief Writes the game result to a file.
     * @param result The result string to write.
     */
    void write_result_to_file(const std::string& result);

    /**
     * @brief Callback function for the reset button.
     */
    void cb_reset();

    /**
     * @brief Callback function for the quit button.
     */
    void cb_quit();

    std::vector<std::vector<char>> board; ///< 3x3 board representation.
    char current_player;  ///< Current player ('X' or 'O').
    char last_player;     ///< Last player to have made a move.
    Button reset_button;  ///< Button to reset the game.
    Button quit_button;   ///< Button to quit the game.
    bool game_over;       ///< Flag indicating if the game is over.
    std::string result_text; ///< Text indicating the winner.
    std::pair<Point, Point> winning_line; ///< Coordinates for the winning line.
};
