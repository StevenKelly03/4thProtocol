#ifndef GAME_RULES_HPP
#define GAME_RULES_HPP

#include "Game.h"

// ============================================================
// Win checking / board state
// ============================================================

int Game::checkWinner() const
{
    // Horizontal (4 in a row)
    for (int r = 0; r < 5; ++r)
    {
        for (int c = 0; c <= 1; ++c)
        {
            int    idx0 = r * gridCols + c;
            Piece* p0 = board[idx0];
            if (!p0) continue;

            int  owner = p0->getOwner();
            bool line = true;

            for (int k = 1; k < 4; ++k)
            {
                int    idx = r * gridCols + (c + k);
                Piece* p = board[idx];
                if (!p || p->getOwner() != owner)
                {
                    line = false;
                    break;
                }
            }
            if (line)
                return owner;
        }
    }

    // Vertical
    for (int c = 0; c < 5; ++c)
    {
        for (int r = 0; r <= 1; ++r)
        {
            int    idx0 = r * gridCols + c;
            Piece* p0 = board[idx0];
            if (!p0) continue;

            int  owner = p0->getOwner();
            bool line = true;

            for (int k = 1; k < 4; ++k)
            {
                int    idx = (r + k) * gridCols + c;
                Piece* p = board[idx];
                if (!p || p->getOwner() != owner)
                {
                    line = false;
                    break;
                }
            }
            if (line)
                return owner;
        }
    }

    // Diagonal down-right
    for (int r = 0; r <= 1; ++r)
    {
        for (int c = 0; c <= 1; ++c)
        {
            int    idx0 = r * gridCols + c;
            Piece* p0 = board[idx0];
            if (!p0) continue;

            int  owner = p0->getOwner();
            bool line = true;

            for (int k = 1; k < 4; ++k)
            {
                int    idx = (r + k) * gridCols + (c + k);
                Piece* p = board[idx];
                if (!p || p->getOwner() != owner)
                {
                    line = false;
                    break;
                }
            }
            if (line)
                return owner;
        }
    }

    // Diagonal up-right
    for (int r = 3; r < 5; ++r)
    {
        for (int c = 0; c <= 1; ++c)
        {
            int    idx0 = r * gridCols + c;
            Piece* p0 = board[idx0];
            if (!p0) continue;

            int  owner = p0->getOwner();
            bool line = true;

            for (int k = 1; k < 4; ++k)
            {
                int    idx = (r - k) * gridCols + (c + k);
                Piece* p = board[idx];
                if (!p || p->getOwner() != owner)
                {
                    line = false;
                    break;
                }
            }
            if (line)
                return owner;
        }
    }

    return 0; // no winner
}

bool Game::isBoardFull() const
{
    for (int i = 0; i < 25; ++i)
        if (board[i] == nullptr)
            return false;
    return true;
}

void Game::checkGameOver()
{
    int winner = checkWinner();
    if (winner != 0)
    {
        gameOver = true;
        if (winner == 1)
            displayedMessage.setString("Player 1 (Yellow) wins!");
        else
            displayedMessage.setString("Player 2 (Red / AI) wins!");
        return;
    }

    if (isBoardFull())
    {
        gameOver = true;
        displayedMessage.setString("Draw! Board is full.");
    }
}

#endif // GAME_RULES_HPP
