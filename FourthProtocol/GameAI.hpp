#ifndef GAME_AI_HPP
#define GAME_AI_HPP

#include "Game.h"

// ============================================================
// AI entry point
// ============================================================

void Game::updateAI()
{
    if (!vsAI)
        return;
    if (gameOver)
        return;

    if (currentTurn != aiPlayer)
        return;

    if (placementPhase)
        aiPlacePiece();
    else
        aiMakeMove();
}

// ------------------------------------------------------------
// AI placement – very simple
// ------------------------------------------------------------

void Game::aiPlacePiece()
{
    auto& pieces = (aiPlayer == 1 ? player1Pieces : player2Pieces);

    int pieceIndex = -1;
    for (int i = 0; i < static_cast<int>(pieces.size()); ++i)
    {
        if (!pieces[i]->isPlaced())
        {
            pieceIndex = i;
            break;
        }
    }
    if (pieceIndex == -1)
        return;

    int destIndex = -1;
    for (int i = 0; i < static_cast<int>(board.size()); ++i)
    {
        if (board[i] == nullptr)
        {
            destIndex = i;
            break;
        }
    }
    if (destIndex == -1)
        return;

    Piece* piece = pieces[pieceIndex].get();

    sf::Vector2f cellPos = grid[destIndex].getPosition();
    piece->getSprite().setPosition(
        { cellPos.x + cellSize / 2.f,
         cellPos.y + cellSize / 2.f });

    piece->setPlaced(true);
    board[destIndex] = piece;

    checkGameOver();
    if (gameOver)
        return;

    bool allPlaced = true;
    for (auto& p : player1Pieces) if (!p->isPlaced()) allPlaced = false;
    for (auto& p : player2Pieces) if (!p->isPlaced()) allPlaced = false;

    if (allPlaced)
        placementPhase = false;

    currentTurn = (currentTurn == 1 ? 2 : 1);
    displayedMessage.setString(currentTurn == 1 ? player1Turn : player2Turn);
}

// ------------------------------------------------------------
// AI movement with minimax
// ------------------------------------------------------------

void Game::aiMakeMove()
{
    const int MAX_DEPTH = 3;

    std::vector<Move> moves;
    generateAllMoves(aiPlayer, moves);

    if (moves.empty())
    {
        gameOver = true;
        displayedMessage.setString("No moves left. Draw!");
        return;
    }

    int bestScore = -1000000;
    Move bestMove = moves[0];

    for (int i = 0; i < static_cast<int>(moves.size()); ++i)
    {
        applyMoveToBoardOnly(moves[i].from, moves[i].to);
        int score = minimax(0, MAX_DEPTH, false, aiPlayer, -1000000, 1000000);
        undoMoveOnBoardOnly(moves[i].from, moves[i].to);

        if (score > bestScore)
        {
            bestScore = score;
            bestMove = moves[i];
        }
    }

    applyMoveToGame(bestMove.from, bestMove.to);
    checkGameOver();
    if (gameOver)
        return;

    currentTurn = (currentTurn == 1 ? 2 : 1);
    displayedMessage.setString(currentTurn == 1 ? player1Turn : player2Turn);
}

// ------------------------------------------------------------
// Evaluation helpers
// ------------------------------------------------------------

int Game::evaluateLine(int a, int b, int c, int d, int aiPlayer) const
{
    int indices[4] = { a, b, c, d };

    int aiCount = 0;
    int oppCount = 0;

    for (int i = 0; i < 4; ++i)
    {
        Piece* p = board[indices[i]];
        if (!p) continue;
        if (p->getOwner() == aiPlayer)
            ++aiCount;
        else
            ++oppCount;
    }

    if (aiCount > 0 && oppCount > 0)
        return 0; // blocked line

    int score = 0;

    if (aiCount > 0)
    {
        if (aiCount == 1) score = 1;
        else if (aiCount == 2) score = 10;
        else if (aiCount == 3) score = 50;
        else if (aiCount == 4) score = 1000;
    }
    else if (oppCount > 0)
    {
        if (oppCount == 1) score = -1;
        else if (oppCount == 2) score = -10;
        else if (oppCount == 3) score = -50;
        else if (oppCount == 4) score = -1000;
    }

    return score;
}

int Game::evaluateBoard(int aiPlayer) const
{
    int score = 0;
    int opponent = (aiPlayer == 1 ? 2 : 1);

    // --- Piece type values ---
    auto pieceValue = [&](Piece* p)
        {
            if (!p) return 0;
            switch (p->getType())
            {
            case PieceType::Frog:   return 15;
            case PieceType::Snake:  return 10;
            case PieceType::Donkey: return 3;
            }
            return 0;
        };

    // --- Center control values ---
    const int centerSquares[5] = { 6, 7, 8, 11, 12 };

    // --- Mobility: number of available moves ---
    std::vector<Move> aiMoves, oppMoves;
    generateAllMoves(aiPlayer, aiMoves);
    generateAllMoves(opponent, oppMoves);

    score += aiMoves.size() * 2;
    score -= oppMoves.size() * 2;

    // --- Piece values & center control ---
    for (int i = 0; i < 25; i++)
    {
        Piece* p = board[i];
        if (!p) continue;

        int base = pieceValue(p);

        if (p->getOwner() == aiPlayer)
        {
            score += base;

            // center bonus
            for (int c : centerSquares)
                if (i == c)
                    score += 3;
        }
        else
        {
            score -= base;

            for (int c : centerSquares)
                if (i == c)
                    score -= 3;
        }
    }

    // ---- Advanced line scoring (4-in-a-row windows) ----
    auto scoreLine = [&](int a, int b, int c, int d)
        {
            return evaluateLine(a, b, c, d, aiPlayer);
        };

    // horizontal
    for (int r = 0; r < 5; r++)
        for (int c = 0; c <= 1; c++)
            score += scoreLine(r * 5 + c, r * 5 + c + 1, r * 5 + c + 2, r * 5 + c + 3);

    // vertical
    for (int c = 0; c < 5; c++)
        for (int r = 0; r <= 1; r++)
            score += scoreLine(r * 5 + c, (r + 1) * 5 + c, (r + 2) * 5 + c, (r + 3) * 5 + c);

    // diag down-right
    for (int r = 0; r <= 1; r++)
        for (int c = 0; c <= 1; c++)
            score += scoreLine(r * 5 + c, (r + 1) * 5 + c + 1, (r + 2) * 5 + c + 2, (r + 3) * 5 + c + 3);

    // diag up-right
    for (int r = 3; r < 5; r++)
        for (int c = 0; c <= 1; c++)
            score += scoreLine(r * 5 + c, (r - 1) * 5 + c + 1, (r - 2) * 5 + c + 2, (r - 3) * 5 + c + 3);

    return score;
}


// ------------------------------------------------------------
// Minimax
// ------------------------------------------------------------

int Game::minimax(int depth, int maxDepth, bool maximizingPlayer, int aiPlayer,
    int alpha, int beta)
{
    int winner = checkWinner();
    if (winner != 0)
    {
        if (winner == aiPlayer)
            return 100000 - depth;
        else
            return -100000 + depth;
    }

    if (depth >= maxDepth || isBoardFull())
        return evaluateBoard(aiPlayer);

    int currentPlayer = maximizingPlayer ? aiPlayer : (aiPlayer == 1 ? 2 : 1);

    std::vector<Move> moves;
    generateAllMoves(currentPlayer, moves);

    if (moves.empty())
        return evaluateBoard(aiPlayer);

    if (maximizingPlayer)
    {
        int best = -1000000;

        for (auto& m : moves)
        {
            applyMoveToBoardOnly(m.from, m.to);
            int val = minimax(depth + 1, maxDepth, false, aiPlayer, alpha, beta);
            undoMoveOnBoardOnly(m.from, m.to);

            best = std::max(best, val);
            alpha = std::max(alpha, best);

            // PRUNE HERE
            if (beta <= alpha)
                break;
        }

        return best;
    }
    else
    {
        int best = 1000000;

        for (auto& m : moves)
        {
            applyMoveToBoardOnly(m.from, m.to);
            int val = minimax(depth + 1, maxDepth, true, aiPlayer, alpha, beta);
            undoMoveOnBoardOnly(m.from, m.to);

            best = std::min(best, val);
            beta = std::min(beta, best);

            // PRUNE HERE
            if (beta <= alpha)
                break;
        }

        return best;
    }
}


#endif // GAME_AI_HPP
