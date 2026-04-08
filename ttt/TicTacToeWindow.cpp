/**
 * @file TicTacToeWindow.cpp
 * @brief Contains the implementation of the TicTacToeWindow class.
 * 
 * @details This file uses the following libraries and resources:
 * - TicTacToeWindow.h for class definition
 * - iostream for standard input/output operations
 */

#include "TicTacToeWindow.h"
#include <iostream>

constexpr int BUTTON_WIDTH = 100;
constexpr int BUTTON_HEIGHT = 30;
constexpr int RESET_BUTTON_X = 10;
constexpr int RESET_BUTTON_Y = 10;
constexpr int QUIT_BUTTON_X = 120;
constexpr int QUIT_BUTTON_Y = 10;
constexpr int RESULT_TEXT_X = 240;
constexpr int RESULT_TEXT_Y = 10;

constexpr int BOARD_SIZE = 600;
constexpr int CELL_SIZE = BOARD_SIZE / 3;
constexpr int BOARD_X = 20;
constexpr int BOARD_Y = 60;
constexpr int FONT_SIZE = 100;

/**
 * @brief Constructs the TicTacToeWindow object and initializes the game.
 * @param x X-coordinate of the window.
 * @param y Y-coordinate of the window.
 * @param width Width of the window.
 * @param height Height of the window.
 * @param title Title of the window.
 */
TicTacToeWindow::TicTacToeWindow(int x, int y, int width, int height, const std::string& title)
    : AnimationWindow(x, y, width, height, title), current_player('X'), reset_button({RESET_BUTTON_X, RESET_BUTTON_Y}, BUTTON_WIDTH, BUTTON_HEIGHT, "Reset"), quit_button({QUIT_BUTTON_X, QUIT_BUTTON_Y}, BUTTON_WIDTH, BUTTON_HEIGHT, "Quit"), game_over(false) { 
    reset_game();
    add(reset_button);
    add(quit_button);
    reset_button.setCallback(std::bind(&TicTacToeWindow::cb_reset, this));
    quit_button.setCallback(std::bind(&TicTacToeWindow::cb_quit, this));
}

/**
 * @brief Starts the game loop.
 */
void TicTacToeWindow::play() {
    while (!should_close()) {
        if (is_left_mouse_button_down() && !game_over) {
            auto [x, y] = get_mouse_coordinates();
            if (x >= RESET_BUTTON_X && x <= RESET_BUTTON_X + BUTTON_WIDTH && y >= RESET_BUTTON_Y && y <= RESET_BUTTON_Y + BUTTON_HEIGHT) {
                cb_reset();
            } else if (x >= QUIT_BUTTON_X && x <= QUIT_BUTTON_X + BUTTON_WIDTH && y >= QUIT_BUTTON_Y && y <= QUIT_BUTTON_Y + BUTTON_HEIGHT) {
                cb_quit();
            } else {
                try {
                    handle_click(x, y);
                } catch (const std::out_of_range& e) {
                    std::cerr << "Error: " << e.what() << std::endl;
                }
            }
        }

        if (check_winner()) {
            result_text = "Player " + std::string(1, last_player) + " wins!";
            draw_winning_line();
            if (!game_over) {
                write_result_to_file(result_text);
            }
            game_over = true;
        } else if (check_draw()) {
            result_text = "The game is a draw!";
            if (!game_over) {
                write_result_to_file(result_text);
            }
            game_over = true;
        }

        draw_board();
        draw_marks();
        draw_text({RESULT_TEXT_X, RESULT_TEXT_Y}, result_text, TDT4102::Color::black, 30);

        next_frame();
    }
}

/**
 * @brief Draws the Tic Tac Toe board grid.
 */
void TicTacToeWindow::draw_board() {
    for (int i = 1; i < 3; ++i) {
        draw_line({BOARD_X + i * CELL_SIZE, BOARD_Y}, {BOARD_X + i * CELL_SIZE, BOARD_Y + BOARD_SIZE}, TDT4102::Color::black);
        draw_line({BOARD_X, BOARD_Y + i * CELL_SIZE}, {BOARD_X + BOARD_SIZE, BOARD_Y + i * CELL_SIZE}, TDT4102::Color::black);
    }
}

/**
 * @brief Draws the marks (X and O) on the board.
 */
void TicTacToeWindow::draw_marks() {
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            if (board[row][col] == 'X') {
                draw_text({BOARD_X * 3 / 2 + col * CELL_SIZE + FONT_SIZE / 2, BOARD_Y + row * CELL_SIZE + FONT_SIZE / 2}, "X", TDT4102::Color::red, FONT_SIZE);
            } else if (board[row][col] == 'O') {
                draw_text({BOARD_X * 3 / 2 + col * CELL_SIZE + FONT_SIZE / 2, BOARD_Y + row * CELL_SIZE + FONT_SIZE / 2}, "O", TDT4102::Color::blue, FONT_SIZE);
            }
        }
    }
}

/**
 * @brief Draws the line indicating the winning combination on the board.
 */
void TicTacToeWindow::draw_winning_line() {
    draw_line(winning_line.first, winning_line.second, TDT4102::Color::yellow);
}

/**
 * @brief Handles mouse click events and updates the board with marks.
 * @param x X-coordinate of the mouse click.
 * @param y Y-coordinate of the mouse click.
 * @throws std::out_of_range if the clicked position is outside the board.
 */
void TicTacToeWindow::handle_click(int x, int y) {
    int row = (y - BOARD_Y) / CELL_SIZE;
    int col = (x - BOARD_X) / CELL_SIZE;

    if (row < 0 || row >= 3 || col < 0 || col >= 3) {
        throw std::out_of_range("Invalid board position");
    }

    if (board[row][col] == ' ') {
        board[row][col] = current_player;
        last_player = current_player; 
        current_player = (current_player == 'X') ? 'O' : 'X';
    }
}

/**
 * @brief Checks if there is a winner in the game.
 * @return true if there is a winner, false otherwise.
 */
bool TicTacToeWindow::check_winner() {
    // Check rows and columns
    for (int i = 0; i < 3; ++i) {
        if (board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] != ' ') {
            winning_line = {{BOARD_X, BOARD_Y + i * CELL_SIZE + CELL_SIZE / 2}, {BOARD_X + BOARD_SIZE, BOARD_Y + i * CELL_SIZE + CELL_SIZE / 2}};
            return true;
        }
        if (board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] != ' ') {
            winning_line = {{BOARD_X + i * CELL_SIZE + CELL_SIZE / 2, BOARD_Y}, {BOARD_X + i * CELL_SIZE + CELL_SIZE / 2, BOARD_Y + BOARD_SIZE}};
            return true;
        }
    }

    // Check diagonals
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != ' ') {
        winning_line = {{BOARD_X, BOARD_Y}, {BOARD_X + BOARD_SIZE, BOARD_Y + BOARD_SIZE}};
        return true;
    }
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[0][2] != ' ') {
        winning_line = {{BOARD_X + BOARD_SIZE, BOARD_Y}, {BOARD_X, BOARD_Y + BOARD_SIZE}};
        return true;
    }

    return false;
}

/**
 * @brief Checks if the game has ended in a draw.
 * @return true if the game is a draw, false otherwise.
 */
bool TicTacToeWindow::check_draw() {
    for (const auto& row : board) {
        for (char cell : row) {
            if (cell == ' ') {
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief Resets the game to its initial state.
 */
void TicTacToeWindow::reset_game() {
    std::cout << "Resetting game..." << std::endl; 
    board = std::vector<std::vector<char>>(3, std::vector<char>(3, ' '));
    current_player = 'X';
    game_over = false;
    result_text.clear();
    std::cout << "Game reset complete." << std::endl;
}

/**
 * @brief Writes the game result to a file.
 * @param result The result string to write.
 */
void TicTacToeWindow::write_result_to_file(const std::string& result) {
    try {
        std::ofstream file("game_results.txt", std::ios::app);
        if (!file) {
            throw std::ios_base::failure("Failed to open file");
        }
        file << result << std::endl;
    } catch (const std::ios_base::failure& e) {
        show_error_dialog("Error writing to file: " + std::string(e.what()));
    }
}

/**
 * @brief Callback function for the reset button.
 */
void TicTacToeWindow::cb_reset() {
    std::cout << "Reset button pressed." << std::endl;
    reset_game();
}

/**
 * @brief Callback function for the quit button.
 */
void TicTacToeWindow::cb_quit() {
    std::cout << "Quit button pressed." << std::endl; 
    close();
}
