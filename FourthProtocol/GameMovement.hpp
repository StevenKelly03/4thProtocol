#ifndef GAME_MOVEMENT_HPP
#define GAME_MOVEMENT_HPP

#include "Game.h"

// ============================================================
// Movement helpers (board + highlights)
// ============================================================

int Game::getGridIndexAtPosition(const sf::Vector2f& p)
{
    for (int i = 0; i < 25; ++i)
        if (grid[i].getGlobalBounds().contains(p))
            return i;
    return -1;
}

void Game::generateMoveHighlights(int index, bool p1)
{
    clearMoveHighlights();

    auto& pieces = (p1 ? player1Pieces : player2Pieces);
    Piece* piece = pieces[index].get();

    int fromIdx = getGridIndexAtPosition(piece->getSprite().getPosition());
    int fr = fromIdx / gridCols;
    int fc = fromIdx % gridCols;
    sf::Vector2i from(fc, fr);

    for (int i = 0; i < 25; ++i)
    {
        if (board[i] != nullptr) continue;

        int tr = i / gridCols;
        int tc = i % gridCols;
        sf::Vector2i to(tc, tr);

        bool valid = false;

        if (piece->canMove(from, to))
            valid = true;
        else if (piece->getType() == PieceType::Frog)
        {
            if (canFrogJumpBoard(fromIdx, i))
                valid = true;
        }

        if (!valid) continue;

        sf::RectangleShape h;
        h.setSize({ cellSize - 10.f, cellSize - 10.f });
        h.setFillColor(sf::Color(0, 255, 0, 100));
        h.setPosition(grid[i].getPosition() + sf::Vector2f{ 5.f, 5.f });

        moveHighlights.push_back(h);
        moveHighlightIndices.push_back(i);
    }
}

bool Game::frogJump(int index, bool p1, const sf::Vector2f& dest)
{
    auto& pieces = (p1 ? player1Pieces : player2Pieces);

    Piece* piece = pieces[index].get();
    if (piece->getType() != PieceType::Frog)
        return false;

    int fromIdx = getGridIndexAtPosition(piece->getSprite().getPosition());
    int toIdx = getGridIndexAtPosition(dest);

    return canFrogJumpBoard(fromIdx, toIdx);
}

void Game::clearMoveHighlights()
{
    moveHighlights.clear();
    moveHighlightIndices.clear();
}

// ------------------------------------------------------------
// Board-only helpers (used by AI and movement)
// ------------------------------------------------------------

bool Game::canFrogJumpBoard(int fromIdx, int toIdx) const
{
    if (fromIdx < 0 || toIdx < 0)
        return false;
    if (board[toIdx] != nullptr)
        return false;

    int fr = fromIdx / gridCols;
    int fc = fromIdx % gridCols;
    int tr = toIdx / gridCols;
    int tc = toIdx % gridCols;

    int dx = tc - fc;
    int dy = tr - fr;

    bool straight = (dx == 0 || dy == 0);
    bool diagonal = (std::abs(dx) == std::abs(dy));

    if (!straight && !diagonal)
        return false;

    int sx = (dx > 0 ? 1 : dx < 0 ? -1 : 0);
    int sy = (dy > 0 ? 1 : dy < 0 ? -1 : 0);

    int x = fc + sx;
    int y = fr + sy;

    bool jumped = false;

    while (x != tc || y != tr)
    {
        int idx = y * gridCols + x;
        if (board[idx] != nullptr)
            jumped = true;

        x += sx;
        y += sy;
    }

    return jumped;
}

void Game::generateAllMoves(int player, std::vector<Move>& moves) const
{
    moves.clear();

    for (int from = 0; from < 25; ++from)
    {
        Piece* piece = board[from];
        if (!piece) continue;
        if (piece->getOwner() != player) continue;

        int fr = from / gridCols;
        int fc = from % gridCols;
        sf::Vector2i fromPos(fc, fr);

        for (int to = 0; to < 25; ++to)
        {
            if (board[to] != nullptr)
                continue;

            int tr = to / gridCols;
            int tc = to % gridCols;
            sf::Vector2i toPos(tc, tr);

            bool valid = piece->canMove(fromPos, toPos);
            if (!valid && piece->getType() == PieceType::Frog)
            {
                if (canFrogJumpBoard(from, to))
                    valid = true;
            }

            if (valid)
            {
                moves.push_back({ from, to });
            }
        }
    }
}

void Game::applyMoveToBoardOnly(int from, int to)
{
    board[to] = board[from];
    board[from] = nullptr;
}

void Game::undoMoveOnBoardOnly(int from, int to)
{
    board[from] = board[to];
    board[to] = nullptr;
}

void Game::applyMoveToGame(int from, int to)
{
    Piece* piece = board[from];
    if (!piece)
        return;

    sf::Vector2f cellPos = grid[to].getPosition();
    piece->getSprite().setPosition(
        { cellPos.x + cellSize / 2.f,
         cellPos.y + cellSize / 2.f });

    board[to] = piece;
    board[from] = nullptr;
}

#endif // GAME_MOVEMENT_HPP
