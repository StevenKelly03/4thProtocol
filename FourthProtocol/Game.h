#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>
#include <optional>

// ------------------------------------------------------
// Piece hierarchy
// ------------------------------------------------------

enum class PieceType
{
    Frog,
    Snake,
    Donkey
};

class Piece
{
public:
    Piece(PieceType type, const sf::Texture& texture, int owner);
    virtual ~Piece() = default;

    virtual bool canMove(const sf::Vector2i& from, const sf::Vector2i& to) const = 0;
    virtual bool canJump(const sf::Vector2i& from, const sf::Vector2i& to) const { return false; }

    sf::Sprite& getSprite() { return m_sprite; }
    const sf::Sprite& getSprite() const { return m_sprite; }

    bool isPlaced() const { return m_placed; }
    void setPlaced(bool p) { m_placed = p; }

    int       getOwner() const { return m_owner; }
    PieceType getType()  const { return m_type; }

protected:
    PieceType  m_type;
    sf::Sprite m_sprite;
    bool       m_placed;
    int        m_owner;
};

class FrogPiece : public Piece
{
public:
    FrogPiece(const sf::Texture& tex, int owner);
    bool canMove(const sf::Vector2i& from, const sf::Vector2i& to) const override;
    bool canJump(const sf::Vector2i& from, const sf::Vector2i& to) const override;
};

class SnakePiece : public Piece
{
public:
    SnakePiece(const sf::Texture& tex, int owner);
    bool canMove(const sf::Vector2i& from, const sf::Vector2i& to) const override;
};

class DonkeyPiece : public Piece
{
public:
    DonkeyPiece(const sf::Texture& tex, int owner);
    bool canMove(const sf::Vector2i& from, const sf::Vector2i& to) const override;
};

// ------------------------------------------------------
// Simple move struct for the AI
// ------------------------------------------------------

struct Move
{
    int from;
    int to;
};

// ------------------------------------------------------
// Game class
// ------------------------------------------------------

class Game
{
public:
    Game();
    ~Game();

    void run();

private:
    // Core / main loop stuff (in Game.cpp)
    void initGraphics();
    void initPieces();
    void processEvents();
    void processKeys(const std::optional<sf::Event> ev);
    void update(sf::Time dt);
    void render();

    // ------------ input (GameInput.hpp) ------------
    void processMouseClicks();
    void handlePlacementClick(const sf::Vector2f& p);
    void handleMovementClick(const sf::Vector2f& p);

    // ------------ movement helpers (GameMovement.hpp) ------------
    int  getGridIndexAtPosition(const sf::Vector2f& p);
    void generateMoveHighlights(int index, bool p1);
    void clearMoveHighlights();
    bool frogJump(int index, bool p1, const sf::Vector2f& dest);

    // board-only helpers used by both movement + AI
    bool canFrogJumpBoard(int fromIdx, int toIdx) const;
    void generateAllMoves(int player, std::vector<Move>& moves) const;
    void applyMoveToBoardOnly(int from, int to);
    void undoMoveOnBoardOnly(int from, int to);
    void applyMoveToGame(int from, int to);

    // ------------ AI (GameAI.hpp) ------------
    void updateAI();
    void aiPlacePiece();
    void aiMakeMove();
    int  minimax(int depth, int maxDepth, bool maximizingPlayer, int aiPlayer);
    int  evaluateBoard(int aiPlayer) const;
    int  evaluateLine(int a, int b, int c, int d, int aiPlayer) const;

    // ------------ rules / end game (GameRules.hpp) ------------
    int  checkWinner() const;
    bool isBoardFull() const;
    void checkGameOver();

private:
    sf::RenderWindow window;

    sf::Font font;
    sf::Text displayedMessage;       // must be constructed with font (see Game ctor)

    std::string player1Turn = "It's Player 1's Turn! (Yellow)";
    std::string player2Turn = "It's Player 2's Turn! (Red / AI)";

    std::vector<sf::RectangleShape> grid;
    std::vector<sf::RectangleShape> characterContainers;

    std::vector<sf::Texture> player1Textures;
    std::vector<sf::Texture> player2Textures;

    std::string player1Files[3] =
    {
        "ASSETS/IMAGES/Yellow/Frog.png",
        "ASSETS/IMAGES/Yellow/Snake.png",
        "ASSETS/IMAGES/Yellow/Donkey.png"
    };
    std::string player2Files[3] =
    {
        "ASSETS/IMAGES/Red/Frog.png",
        "ASSETS/IMAGES/Red/Snake.png",
        "ASSETS/IMAGES/Red/Donkey.png"
    };

    sf::RectangleShape messageBoard;

    int   containerIndexMap[5] = { 0, 6, 4, 2, 8 };
    float cellSize;
    float scale;
    int   gridCols;

    std::vector<std::unique_ptr<Piece>> player1Pieces;
    std::vector<std::unique_ptr<Piece>> player2Pieces;

    std::vector<Piece*> board;  // 25 entries, nullptr if empty

    int  currentTurn;
    bool placementPhase;

    int selectedPlayer;
    int selectedPieceIndex;

    std::vector<sf::RectangleShape> moveHighlights;
    std::vector<int>                moveHighlightIndices;

    int  containerCols;
    bool exitGame = false;

    bool vsAI = true;  // AI = player 2
    int  aiPlayer;
    bool gameOver = false;
};

#endif // GAME_HPP
