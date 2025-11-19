#include "Game.h"
#include <iostream>

Game::Game() :
    window{ sf::VideoMode{ sf::Vector2u{2250U, 1250U}, 32U }, "The 4th Protocol" }, cellSize{ 250 }, gridCols{ 5 }, scale{ 4.f }, containerCols{ 2 }
{
    grid.resize(25);
    characterContainers.resize(10);

    player1Textures.resize(3);
    player2Textures.resize(3);

    player1Sprites.clear();
    player2Sprites.clear();

    initGraphics();

    for (int i = 0; i < 5; i++)
    {
        int texIndex = (i < 3) ? i : 2;

        player1Sprites.emplace_back(player1Textures[texIndex]);
        player2Sprites.emplace_back(player2Textures[texIndex]);

        player1Sprites[i].setScale({ scale ,scale });
        player2Sprites[i].setScale({ scale ,scale });

        player1Sprites[i].setOrigin({ player1Sprites[i].getTexture().getSize().x / 2.f, player1Sprites[i].getTexture().getSize().y / 2.f });
        player2Sprites[i].setOrigin({ player2Sprites[i].getTexture().getSize().x / 2.f, player2Sprites[i].getTexture().getSize().y / 2.f });
    }

    for (int i = 0; i < 5; i++)
    {
        int c = containerIndexMap[i];
        player1Sprites[i].setPosition({ characterContainers[c].getPosition().x + 125.f, characterContainers[c].getPosition().y + 125.f });
        player2Sprites[i].setPosition({ player1Sprites[i].getPosition().x + 250.f, characterContainers[i].getPosition().y + 125 });
    }

    placed1.resize(player1Sprites.size(), false);
    placed2.resize(player2Sprites.size(), false);

    currentTurn = 1;
}

Game::~Game()
{
}

void Game::run()
{
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    const float fps{ 60.0f };
    sf::Time timePerFrame = sf::seconds(1.0f / fps);
    while (window.isOpen())
    {
        processEvents();
        timeSinceLastUpdate += clock.restart();
        while (timeSinceLastUpdate > timePerFrame)
        {
            timeSinceLastUpdate -= timePerFrame;
            processEvents();
            update(timePerFrame);
        }
        render();
    }
}

void Game::processEvents()
{
    while (const std::optional newEvent = window.pollEvent())
    {
        if (newEvent->is<sf::Event::Closed>())
        {
            exitGame = true;
        }
        if (newEvent->is<sf::Event::KeyPressed>())
        {
            processKeys(newEvent);
        }
        if (newEvent->is<sf::Event::MouseButtonPressed>())
        {
            processMouseClicks();
        }
    }
}

void Game::processKeys(const std::optional<sf::Event> t_event)
{
    const sf::Event::KeyPressed* newKeypress = t_event->getIf<sf::Event::KeyPressed>();
    if (sf::Keyboard::Key::Escape == newKeypress->code)
    {
        exitGame = true;
    }
}

void Game::checkKeyboardState()
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
    {
        exitGame = true;
    }
}

void Game::processMouseClicks()
{
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f p(mousePos.x, mousePos.y);

    if (selectedSprite == -1)
    {
        if (currentTurn == 1)
        {
            for (int i = 0; i < player1Sprites.size(); i++)
            {
                if (!placed1[i] && player1Sprites[i].getGlobalBounds().contains(p))
                {
                    selectedSprite = i;
                    return;
                }
            }
        }
        else
        {
            for (int i = 0; i < player2Sprites.size(); i++)
            {
                if (!placed2[i] && player2Sprites[i].getGlobalBounds().contains(p))
                {
                    selectedSprite = i + 1000;
                    return;
                }
            }
        }
        return;
    }

    for (int i = 0; i < grid.size(); i++)
    {
        if (grid[i].getGlobalBounds().contains(p))
        {
            if (selectedSprite < 1000)
            {
                player1Sprites[selectedSprite].setPosition({ grid[i].getPosition().x + 125.f,grid[i].getPosition().y + 125.f });
                placed1[selectedSprite] = true;
                currentTurn = 2;
                displayedMessage.setString(player2Turn);
                displayedMessage.setOutlineColor(sf::Color::Red);
            }
            else
            {
                int r = selectedSprite - 1000;
                player2Sprites[r].setPosition({ grid[i].getPosition().x + 125.f, grid[i].getPosition().y + 125.f });
                placed2[r] = true;
                currentTurn = 1;
                displayedMessage.setString(player1Turn);
                displayedMessage.setOutlineColor(sf::Color::Yellow);
            }

            selectedSprite = -1;
            return;
        }
    }
}

void Game::update(sf::Time t_deltaTime)
{
    checkKeyboardState();
    if (exitGame)
    {
        window.close();
    }
}

void Game::render()
{
    window.clear(sf::Color::White);

    for (int i = 0; i < grid.size(); i++)
    {
        window.draw(grid[i]);
    }

    for (int i = 0; i < characterContainers.size(); i++)
    {
        window.draw(characterContainers[i]);
    }

    for (int i = 0; i < player1Sprites.size(); i++)
    {
        window.draw(player1Sprites[i]);
    }

    for (int i = 0; i < player2Sprites.size(); i++)
    {
        window.draw(player2Sprites[i]);
    }

    window.draw(messageBoard);
    window.draw(displayedMessage);
    window.display();
}

void Game::initGraphics()
{
    font.openFromFile("ASSETS\\FONTS\\Jersey20-Regular.ttf");

    for (int i = 0; i < grid.size(); i++)
    {
        int col = i % gridCols;
        int row = i / gridCols;

        grid[i].setSize({ cellSize,cellSize });
        grid[i].setFillColor(sf::Color::Transparent);
        grid[i].setOutlineColor(sf::Color::Black);
        grid[i].setOutlineThickness(-2.5f);
        grid[i].setPosition({ col * cellSize + 500.f, row * cellSize });
    }

    for (int i = 0; i < characterContainers.size(); i++)
    {
        int col = i % containerCols;
        int row = i % 5;

        characterContainers[i].setSize({ cellSize,cellSize });
        characterContainers[i].setFillColor(sf::Color::Transparent);
        characterContainers[i].setOutlineColor(sf::Color{ 0, 255, 255 });
        characterContainers[i].setOutlineThickness(-2.5f);
        characterContainers[i].setPosition({ col * cellSize, row * cellSize });
    }

    for (int i = 0; i < 3; i++)
    {
        player1Textures[i].loadFromFile(player1Files[i]);
        player2Textures[i].loadFromFile(player2Files[i]);
    }

    messageBoard.setSize({ 500, 1250 });
    messageBoard.setFillColor(sf::Color{ 0,0,0,175 });
    messageBoard.setPosition({ 1751, 0 });

    displayedMessage.setFont(font);
    displayedMessage.setCharacterSize(45.f);
    displayedMessage.setFillColor(sf::Color::White);
    displayedMessage.setOutlineColor(sf::Color::Red);
    displayedMessage.setOutlineThickness(-1.f);
    displayedMessage.setPosition({ 1760, 10 });
    displayedMessage.setString(player1Turn);
}
