#include "Game.h"

#include "GameInput.hpp"
#include "GameMovement.hpp"
#include "GameRules.hpp"
#include "GameAI.hpp"

#include <cmath>
#include <iostream>

// ------------------------------------------------------
// Core Game implementation
// ------------------------------------------------------

Game::Game()
    : window(sf::VideoMode({ 2250u, 1250u }), "The 4th Protocol")
    , font()
    , displayedMessage(font, "", 40)      // SFML 3: Text must have a font
    , cellSize(250.f)
    , scale(4.f)
    , gridCols(5)
    , currentTurn(1)
    , placementPhase(true)
    , selectedPlayer(0)
    , selectedPieceIndex(-1)
    , containerCols(2)
    , aiPlayer(2)
{
    grid.resize(25);
    characterContainers.resize(10);

    player1Textures.resize(3);
    player2Textures.resize(3);

    board.assign(25, nullptr);

    initGraphics();
    initPieces();
}

Game::~Game() = default;

void Game::run()
{
    sf::Clock clock;
    sf::Time accumulator;
    sf::Time step = sf::seconds(1.f / 60.f);

    while (window.isOpen())
    {
        accumulator += clock.restart();
        processEvents();

        while (accumulator > step)
        {
            accumulator -= step;
            processEvents();
            update(step);
        }

        render();
    }
}

void Game::initGraphics()
{
    window.setFramerateLimit(60);

    if (!font.openFromFile("ASSETS/FONTS/Jersey20-Regular.ttf"))
    {
        std::cout << "Failed to open font\n";
    }

    // Grid 5x5
    for (int i = 0; i < 25; ++i)
    {
        sf::RectangleShape cell;
        cell.setSize({ cellSize - 4.f, cellSize - 4.f });
        cell.setFillColor(sf::Color(200, 200, 200));
        cell.setOutlineThickness(2.f);
        cell.setOutlineColor(sf::Color::Black);

        int r = i / gridCols;
        int c = i % gridCols;

        cell.setPosition({ c * cellSize + 2.f,
                          r * cellSize + 2.f });

        grid[i] = cell;
    }

    // Character containers (5 for P1, 5 for P2) on the right side
    float boardWidth = gridCols * cellSize;
    float panelX = boardWidth + 50.f;
    float containerSize = 200.f;

    for (int i = 0; i < 10; ++i)
    {
        sf::RectangleShape box;
        box.setSize({ containerSize, containerSize });
        box.setFillColor(sf::Color(80, 80, 80));
        box.setOutlineThickness(2.f);
        box.setOutlineColor(sf::Color::White);

        int col = (i < 5) ? 0 : 1;
        int row = (i < 5) ? i : i - 5;

        box.setPosition({ panelX + col * (containerSize + 40.f),
                         150.f + row * (containerSize + 20.f) });

        characterContainers[i] = box;
    }

    // Message board
    float boardWidthRight = gridCols * cellSize;
    float msgPanelX = boardWidthRight + 50.f;

    messageBoard.setSize({ 800.f, 120.f });
    messageBoard.setFillColor(sf::Color(50, 50, 50));
    messageBoard.setOutlineThickness(3.f);
    messageBoard.setOutlineColor(sf::Color::White);
    messageBoard.setPosition({ msgPanelX, 20.f });

    displayedMessage.setFont(font);
    displayedMessage.setCharacterSize(40);
    displayedMessage.setFillColor(sf::Color::White);
    displayedMessage.setString(player1Turn);
    displayedMessage.setPosition(
        messageBoard.getPosition() + sf::Vector2f{ 20.f, 35.f });
}

void Game::initPieces()
{
    // Load textures (Texture::loadFromFile is still valid in SFML 3) :contentReference[oaicite:3]{index=3}
    for (int i = 0; i < 3; ++i)
    {
        if (!player1Textures[i].loadFromFile(player1Files[i]))
            std::cout << "Failed to load " << player1Files[i] << "\n";
        if (!player2Textures[i].loadFromFile(player2Files[i]))
            std::cout << "Failed to load " << player2Files[i] << "\n";
    }

    auto makeP1Frog = [&]() { return std::make_unique<FrogPiece>(player1Textures[0], 1); };
    auto makeP1Snake = [&]() { return std::make_unique<SnakePiece>(player1Textures[1], 1); };
    auto makeP1Donkey = [&]() { return std::make_unique<DonkeyPiece>(player1Textures[2], 1); };

    auto makeP2Frog = [&]() { return std::make_unique<FrogPiece>(player2Textures[0], 2); };
    auto makeP2Snake = [&]() { return std::make_unique<SnakePiece>(player2Textures[1], 2); };
    auto makeP2Donkey = [&]() { return std::make_unique<DonkeyPiece>(player2Textures[2], 2); };

    // Simple set: 5 pieces each
    player1Pieces.push_back(makeP1Frog());
    player1Pieces.push_back(makeP1Snake());
    player1Pieces.push_back(makeP1Donkey());
    player1Pieces.push_back(makeP1Frog());
    player1Pieces.push_back(makeP1Snake());

    player2Pieces.push_back(makeP2Frog());
    player2Pieces.push_back(makeP2Snake());
    player2Pieces.push_back(makeP2Donkey());
    player2Pieces.push_back(makeP2Frog());
    player2Pieces.push_back(makeP2Snake());

    // Place in containers
    for (int i = 0; i < static_cast<int>(player1Pieces.size()) && i < 5; ++i)
    {
        sf::Sprite& s = player1Pieces[i]->getSprite();
        auto bounds = s.getLocalBounds();

        s.setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });

        sf::Vector2f pos = characterContainers[i].getPosition();
        sf::Vector2f size = characterContainers[i].getSize();

        s.setPosition({ pos.x + size.x / 2.f,
                       pos.y + size.y / 2.f });
        s.setScale({ 0.7f, 0.7f });
    }

    for (int i = 0; i < static_cast<int>(player2Pieces.size()) && i < 5; ++i)
    {
        sf::Sprite& s = player2Pieces[i]->getSprite();
        auto bounds = s.getLocalBounds();

        s.setOrigin({ bounds.size.x / 2.f, bounds.size.y / 2.f });

        sf::Vector2f pos = characterContainers[5 + i].getPosition();
        sf::Vector2f size = characterContainers[5 + i].getSize();

        s.setPosition({ pos.x + size.x / 2.f,
                       pos.y + size.y / 2.f });
        s.setScale({ 0.7f, 0.7f });
    }
}

void Game::processEvents()
{
    while (const std::optional ev = window.pollEvent())
    {
        if (ev->is<sf::Event::Closed>())
            exitGame = true;

        if (ev->is<sf::Event::KeyPressed>())
            processKeys(ev);

        if (ev->is<sf::Event::MouseButtonPressed>())
            processMouseClicks();
    }
}

void Game::processKeys(const std::optional<sf::Event> ev)
{
    if (!ev) return;

    if (const auto* key = ev->getIf<sf::Event::KeyPressed>())
    {
        if (key->code == sf::Keyboard::Key::Escape)
            exitGame = true;
    }
}

void Game::update(sf::Time)
{
    if (exitGame)
        window.close();

    if (gameOver)
        return;

    if (vsAI)
        updateAI();
}

void Game::render()
{
    window.clear(sf::Color(30, 30, 30));

    // Board
    for (auto& cell : grid)
        window.draw(cell);

    // Move highlights
    for (auto& h : moveHighlights)
        window.draw(h);

    // Piece containers
    for (auto& box : characterContainers)
        window.draw(box);

    // Pieces
    for (auto& p : player1Pieces)
        window.draw(p->getSprite());
    for (auto& p : player2Pieces)
        window.draw(p->getSprite());

    // Message board
    window.draw(messageBoard);
    window.draw(displayedMessage);

    window.display();
}
