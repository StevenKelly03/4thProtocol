#ifndef PIECES_HPP
#define PIECES_HPP

#include "Game.h"
#include <cmath>

// ============================================================
// Piece implementation
// ============================================================

Piece::Piece(PieceType type, const sf::Texture& texture, int owner)
    : m_type(type)
    , m_sprite(texture)
    , m_placed(false)
    , m_owner(owner)
{
}

// Frog
FrogPiece::FrogPiece(const sf::Texture& tex, int owner)
    : Piece(PieceType::Frog, tex, owner)
{
}

bool FrogPiece::canMove(const sf::Vector2i& f, const sf::Vector2i& t) const
{
    int dx = std::abs(f.x - t.x);
    int dy = std::abs(f.y - t.y);
    return dx <= 1 && dy <= 1 && !(dx == 0 && dy == 0);
}

bool FrogPiece::canJump(const sf::Vector2i& f, const sf::Vector2i& t) const
{
    int dx = t.x - f.x;
    int dy = t.y - f.y;

    bool straight = (dx == 0 || dy == 0);
    bool diagonal = (std::abs(dx) == std::abs(dy));

    return straight || diagonal;
}

// Snake
SnakePiece::SnakePiece(const sf::Texture& tex, int owner)
    : Piece(PieceType::Snake, tex, owner)
{
}

bool SnakePiece::canMove(const sf::Vector2i& f, const sf::Vector2i& t) const
{
    int dx = std::abs(f.x - t.x);
    int dy = std::abs(f.y - t.y);
    return dx <= 1 && dy <= 1 && !(dx == 0 && dy == 0);
}

// Donkey
DonkeyPiece::DonkeyPiece(const sf::Texture& tex, int owner)
    : Piece(PieceType::Donkey, tex, owner)
{
}

bool DonkeyPiece::canMove(const sf::Vector2i& f, const sf::Vector2i& t) const
{
    int dx = std::abs(f.x - t.x);
    int dy = std::abs(f.y - t.y);
    return dx + dy == 1;
}

#endif // PIECES_HPP
