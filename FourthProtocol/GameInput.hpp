#ifndef GAME_INPUT_HPP
#define GAME_INPUT_HPP

#include "Game.h"

// ============================================================
// Mouse input routing
// ============================================================

void Game::processMouseClicks()
{
    if (gameOver)
        return;

    sf::Vector2i mp = sf::Mouse::getPosition(window);
    sf::Vector2f p = window.mapPixelToCoords(mp);

    if (placementPhase)
        handlePlacementClick(p);
    else
        handleMovementClick(p);
}

// ------------------------------------------------------------
// Placement phase
// ------------------------------------------------------------

void Game::handlePlacementClick(const sf::Vector2f& p)
{
    if (gameOver)
        return;

    auto& pieces = (currentTurn == 1 ? player1Pieces : player2Pieces);

    if (selectedPieceIndex == -1)
    {
        // select piece in container
        for (int i = 0; i < static_cast<int>(pieces.size()); ++i)
        {
            if (!pieces[i]->isPlaced() &&
                pieces[i]->getSprite().getGlobalBounds().contains(p))
            {
                selectedPieceIndex = i;
                selectedPlayer = currentTurn;
                return;
            }
        }
        return;
    }

    // place into grid
    for (int i = 0; i < 25; ++i)
    {
        if (grid[i].getGlobalBounds().contains(p))
        {
            if (board[i] != nullptr)
                return;

            Piece* piece = pieces[selectedPieceIndex].get();
            sf::Vector2f cellPos = grid[i].getPosition();

            sf::Sprite& s = piece->getSprite();
            s.setPosition({ cellPos.x + cellSize / 2.f,
                            cellPos.y + cellSize / 2.f });

            piece->setPlaced(true);
            board[i] = piece;

            checkGameOver();
            selectedPieceIndex = -1;
            selectedPlayer = 0;

            if (gameOver)
                return;

            bool allPlaced = true;
            for (auto& pp : player1Pieces) if (!pp->isPlaced()) allPlaced = false;
            for (auto& pp : player2Pieces) if (!pp->isPlaced()) allPlaced = false;

            if (allPlaced)
                placementPhase = false;

            currentTurn = (currentTurn == 1 ? 2 : 1);
            displayedMessage.setString(currentTurn == 1 ? player1Turn : player2Turn);
            return;
        }
    }
}

// ------------------------------------------------------------
// Movement phase
// ------------------------------------------------------------

void Game::handleMovementClick(const sf::Vector2f& p)
{
    if (gameOver)
        return;

    auto& pieces = (currentTurn == 1 ? player1Pieces : player2Pieces);

    if (selectedPieceIndex == -1)
    {
        // select a placed piece
        for (int i = 0; i < static_cast<int>(pieces.size()); ++i)
        {
            if (pieces[i]->isPlaced() &&
                pieces[i]->getSprite().getGlobalBounds().contains(p))
            {
                selectedPieceIndex = i;
                selectedPlayer = currentTurn;
                generateMoveHighlights(i, currentTurn == 1);
                return;
            }
        }
        return;
    }

    // attempt move to one of the highlighted cells
    for (int h = 0; h < static_cast<int>(moveHighlights.size()); ++h)
    {
        if (moveHighlights[h].getGlobalBounds().contains(p))
        {
            int   dest = moveHighlightIndices[h];
            Piece* piece = pieces[selectedPieceIndex].get();
            int   from = getGridIndexAtPosition(piece->getSprite().getPosition());

            board[from] = nullptr;

            sf::Vector2f cellPos = grid[dest].getPosition();
            piece->getSprite().setPosition(
                { cellPos.x + cellSize / 2.f,
                  cellPos.y + cellSize / 2.f });

            board[dest] = piece;

            checkGameOver();
            clearMoveHighlights();
            selectedPieceIndex = -1;
            selectedPlayer = 0;

            if (gameOver)
                return;

            currentTurn = (currentTurn == 1 ? 2 : 1);
            displayedMessage.setString(currentTurn == 1 ? player1Turn : player2Turn);
            return;
        }
    }

    // click on different own piece: change selection
    for (int i = 0; i < static_cast<int>(pieces.size()); ++i)
    {
        if (pieces[i]->isPlaced() &&
            pieces[i]->getSprite().getGlobalBounds().contains(p))
        {
            selectedPieceIndex = i;
            selectedPlayer = currentTurn;
            generateMoveHighlights(i, currentTurn == 1);
            return;
        }
    }

    // clicked elsewhere: clear selection
    clearMoveHighlights();
    selectedPieceIndex = -1;
    selectedPlayer = 0;
}

#endif // GAME_INPUT_HPP
