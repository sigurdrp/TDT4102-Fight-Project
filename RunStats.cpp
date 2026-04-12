#include "RunStats.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>

namespace RunStats {

int countAttempts() {
    std::ifstream file("attempts.txt");
    if (!file) {
        return 0;
    }

    int count = 0;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            ++count;
        }
    }
    return count;
}

void appendAttempt() {
    std::ofstream file("attempts.txt", std::ios::app);
    if (!file) {
        std::cout << "Could not open attempts.txt\n";
        return;
    }

    file << "attempt\n";
}

void appendWinTime(double seconds) {
    std::ofstream file("timesToWin.txt", std::ios::app);
    if (!file) {
        std::cout << "Could not open timesToWin.txt for writing.\n";
        return;
    }

    file << std::fixed << std::setprecision(2)
         << "Win time: " << seconds << " seconds\n";
}

}