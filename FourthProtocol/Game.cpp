#include "Game.h"

#include "GameInput.hpp"
#include "GameMovement.hpp"
#include "GameRules.hpp"
#include "GameAI.hpp"
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <ctime>

Game::Game()
// Original: window(sf::VideoMode({ 2250u, 1250u }), "The 4th Protocol")
    : window(sf::VideoMode({ 1250u, 695u }), "The 4th Protocol")
    , font()
    // Original: displayedMessage(font, "", 40)
    , displayedMessage(font, "", 23)
    , difficultyTexts{
        // Original: sf::Text(font, "", 30) -> Scaled to 17
        sf::Text(font, "", 17),
        // Original: sf::Text(font, "", 30) -> Scaled to 17
        sf::Text(font, "", 17),
        // Original: sf::Text(font, "", 30) -> Scaled to 17
        sf::Text(font, "", 17)
    }
    // Original: difficultyCurrentText(font, "", 32) -> Scaled to 19
    , difficultyCurrentText(font, "", 19)
    // Original: cellSize(250.f) -> Scaled to 138.9f
    , cellSize(138.9f)
    // Original: scale(4.f) -> Scaled to 2.2224f
    , scale(2.2224f)
    , gridCols(5)
    , currentTurn(1)
    , placementPhase(true)
    , selectedPlayer(0)
    , selectedPieceIndex(-1)
    , containerCols(2)
    , aiPlayer(2)
    , aiMaxDepth(3)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

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

void Game::resetGame()
{
    currentTurn = 1;
    placementPhase = true;
    gameOver = false;
    selectedPlayer = 0;
    selectedPieceIndex = -1;
    selectedPieceContainerIndex = -1;
    selectedGridIndex = -1;

    board.assign(25, nullptr);

    for (auto& p : player1Pieces)
    {
        p->setPlaced(false);
    }
    for (auto& p : player2Pieces)
    {
        p->setPlaced(false);
    }

    clearMoveHighlights();
    displayedMessage.setString(player1Turn);

    initPieces();
}

void Game::initGraphics()
{
    window.setFramerateLimit(60);

    if (!font.openFromFile("ASSETS/FONTS/Jersey20-Regular.ttf"))
    {
        std::cout << "Failed to open font\n";
    }

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

    float boardWidth = gridCols * cellSize;
    // Original: 300.f -> Scaled to 167.f
    float panelX = boardWidth + 167.f;
    // Original: 200.f -> Scaled to 111.f
    float containerSize = 111.f;

    for (int i = 0; i < 10; ++i)
    {
        sf::RectangleShape box;
        box.setSize({ containerSize, containerSize });
        box.setFillColor(sf::Color(80, 80, 80));
        box.setOutlineThickness(2.f);
        box.setOutlineColor(sf::Color::White);

        int col = (i < 5) ? 0 : 1;
        int row = (i < 5) ? i : i - 5;

        // Original: 40.f -> Scaled to 22.f
        // Original: 150.f -> Scaled to 83.f
        // Original: 20.f -> Scaled to 11.f
        box.setPosition({ panelX + col * (containerSize + 22.f),
                         83.f + row * (containerSize + 11.f) });

        characterContainers[i] = box;
    }

    float boardWidthRight = gridCols * cellSize;
    // Original: 50.f -> Scaled to 28.f
    float msgPanelX = boardWidthRight + 28.f;

    // Original: 800.f -> Scaled to 445.f
    // Original: 120.f -> Scaled to 67.f
    messageBoard.setSize({ 445.f, 67.f });
    messageBoard.setFillColor(sf::Color(50, 50, 50));
    messageBoard.setOutlineThickness(3.f);
    messageBoard.setOutlineColor(sf::Color::White);
    // Original: 20.f -> Scaled to 11.f
    messageBoard.setPosition({ msgPanelX, 11.f });

    displayedMessage.setFont(font);
    // Character size is now 23, set in constructor
    displayedMessage.setCharacterSize(23);
    displayedMessage.setFillColor(sf::Color::White);
    displayedMessage.setString(player1Turn);
    // Original: 20.f -> Scaled to 11.f
    // Original: 35.f -> Scaled to 19.f
    displayedMessage.setPosition(
        messageBoard.getPosition() + sf::Vector2f{ 11.f, 19.f });

    // Original: 50.f -> Scaled to 28.f
    float difficultyX = boardWidth + 28.f;
    // Original: 150.f -> Scaled to 83.f
    float difficultyTopY = 83.f;

    difficultyCurrentText.setFont(font);
    // Character size is now 19, set in constructor
    difficultyCurrentText.setCharacterSize(19);
    difficultyCurrentText.setFillColor(sf::Color::White);
    difficultyCurrentText.setPosition({ difficultyX, difficultyTopY });

    const char* levelNames[3] = { "Easy", "Medium", "Hard" };

    // Original: 50.f -> Scaled to 28.f
    float boxYStart = difficultyTopY + 28.f;

    for (int i = 0; i < 3; ++i)
    {
        // Original: 220.f -> Scaled to 122.f
        // Original: 60.f -> Scaled to 33.f
        difficultyBoxes[i].setSize({ 122.f, 33.f });
        difficultyBoxes[i].setOutlineThickness(2.f);
        difficultyBoxes[i].setOutlineColor(sf::Color::White);
        // Original: 80.f -> Scaled to 44.f
        difficultyBoxes[i].setPosition({ difficultyX, boxYStart + i * 44.f });

        difficultyTexts[i].setFont(font);
        // Character size is now 17, set in constructor
        difficultyTexts[i].setCharacterSize(17);
        difficultyTexts[i].setFillColor(sf::Color::White);
        difficultyTexts[i].setString(levelNames[i]);

        sf::Vector2f pos = difficultyBoxes[i].getPosition();
        // Original: 20.f -> Scaled to 11.f
        // Original: 15.f -> Scaled to 8.f
        difficultyTexts[i].setPosition({ pos.x + 11.f, pos.y + 8.f });
    }

    updateDifficultyUI();

}

void Game::updateDifficultyUI()
{
    int index = 1;
    if (aiMaxDepth <= 1)
        index = 0;
    else if (aiMaxDepth >= 3)
        index = 2;

    const char* levelNames[3] = { "Easy", "Medium", "Hard" };

    for (int i = 0; i < 3; ++i)
    {
        if (i == index)
            difficultyBoxes[i].setFillColor(sf::Color(90, 140, 220));
        else
            difficultyBoxes[i].setFillColor(sf::Color(80, 80, 80));
    }

    difficultyCurrentText.setString(std::string("Difficulty: ") + levelNames[index]);
}

void Game::updateContainerUI()
{
    // Reset all containers to default color
    for (int i = 0; i < 10; ++i)
    {
        characterContainers[i].setFillColor(sf::Color(80, 80, 80));
    }

    // Highlight the selected container/piece
    if (selectedPieceContainerIndex != -1)
    {
        // Use a distinct highlight color, e.g., a bright blue
        characterContainers[selectedPieceContainerIndex].setFillColor(sf::Color(90, 140, 220));
    }
}

void Game::initPieces()
{
    for (int i = 0; i < 3; ++i)
    {
        player1Textures[i].loadFromFile(player1Files[i]);
        player2Textures[i].loadFromFile(player2Files[i]);
    }

    // Revised to ensure correct piece counts and remove duplicates
    player1Pieces.clear();
    player2Pieces.clear();

    player1Pieces.push_back(std::make_unique<FrogPiece>(player1Textures[0], 1));
    player1Pieces.push_back(std::make_unique<SnakePiece>(player1Textures[1], 1));
    player1Pieces.push_back(std::make_unique<DonkeyPiece>(player1Textures[2], 1));
    player1Pieces.push_back(std::make_unique<DonkeyPiece>(player1Textures[2], 1));
    player1Pieces.push_back(std::make_unique<DonkeyPiece>(player1Textures[2], 1));

    player2Pieces.push_back(std::make_unique<FrogPiece>(player2Textures[0], 2));
    player2Pieces.push_back(std::make_unique<SnakePiece>(player2Textures[1], 2));
    player2Pieces.push_back(std::make_unique<DonkeyPiece>(player2Textures[2], 2));
    player2Pieces.push_back(std::make_unique<DonkeyPiece>(player2Textures[2], 2));
    player2Pieces.push_back(std::make_unique<DonkeyPiece>(player2Textures[2], 2));

    const float scale = 2.2224f;

    for (int i = 0; i < 5; ++i)
    {
        sf::Sprite& s = player1Pieces[i]->getSprite();
        s.setOrigin({ s.getTexture().getSize().x / 2.f, s.getTexture().getSize().y / 2.f });
        s.setScale({ scale, scale });

        sf::Vector2f boxPos = characterContainers[i].getPosition();
        sf::Vector2f boxSize = characterContainers[i].getSize();
        s.setPosition({ boxPos.x + boxSize.x * 0.5f,
                        boxPos.y + boxSize.y * 0.5f });
    }

    for (int i = 0; i < 5; ++i)
    {
        sf::Sprite& s = player2Pieces[i]->getSprite();
        s.setOrigin({ s.getTexture().getSize().x / 2.f, s.getTexture().getSize().y / 2.f });
        s.setScale({ scale, scale });

        sf::Vector2f boxPos = characterContainers[5 + i].getPosition();
        sf::Vector2f boxSize = characterContainers[5 + i].getSize();
        s.setPosition({ boxPos.x + boxSize.x * 0.5f,
                        boxPos.y + boxSize.y * 0.5f });
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

void Game::processKeys(const std::optional<sf::Event>& ev)
{
    if (!ev) return;

    if (const auto* key = ev->getIf<sf::Event::KeyPressed>())
    {
        if (key->code == sf::Keyboard::Key::Escape)
            exitGame = true;
        else if (key->code == sf::Keyboard::Key::R)
            resetGame();
        else if (key->code == sf::Keyboard::Key::Num1)
        {
            aiMaxDepth = 1;
            updateDifficultyUI();
        }
        else if (key->code == sf::Keyboard::Key::Num2)
        {
            aiMaxDepth = 2;
            updateDifficultyUI();
        }
        else if (key->code == sf::Keyboard::Key::Num3)
        {
            aiMaxDepth = 3;
            updateDifficultyUI();
        }
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

    if (placementPhase)
        updateContainerUI();

    for (auto& cell : grid)
        window.draw(cell);

    // Draw selection highlight for the piece being moved
    if (!placementPhase && selectedGridIndex != -1)
    {
        sf::RectangleShape selectedHighlight;
        selectedHighlight.setSize({ cellSize - 10.f, cellSize - 10.f });
        selectedHighlight.setFillColor(sf::Color(255, 215, 0, 150)); // Gold/Yellow
        sf::Vector2f pos = grid[selectedGridIndex].getPosition();
        selectedHighlight.setPosition(pos + sf::Vector2f{ 5.f, 5.f });
        window.draw(selectedHighlight);
    }

    for (auto& h : moveHighlights)
        window.draw(h);

    for (auto& box : characterContainers)
        window.draw(box);

    for (int i = 0; i < 3; ++i)
    {
        window.draw(difficultyBoxes[i]);
        window.draw(difficultyTexts[i]);
    }

    window.draw(difficultyCurrentText);

    for (auto& p : player1Pieces)
        window.draw(p->getSprite());
    for (auto& p : player2Pieces)
        window.draw(p->getSprite());

    window.draw(messageBoard);
    window.draw(displayedMessage);

    window.display();
}