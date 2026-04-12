#pragma once

namespace RunStats {
    int countAttempts();
    void appendAttempt();
    void appendWinTime(double seconds);
}