#ifndef GAME_HPP
#define GAME_HPP
#pragma warning( push )
#pragma warning( disable : 4275 )
// ignore this warning
// C:\SFML - 3.0.0\include\SFML\System\Exception.hpp(41, 47) : 
// warning C4275 : non dll - interface class 'std::runtime_error' used as base for dll - interface class 'sf::Exception'

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>

class Game
{
public:
	Game();
	~Game();
	void run();

private:
	void processEvents();
	void processKeys(const std::optional<sf::Event> t_event);
	void checkKeyboardState();
	void processMouseClicks();
	void update(sf::Time t_deltaTime);
	void render();
	void initGraphics();
	void generateMoveHighlights(int index, bool p1);
	void clearMoveHighlights();
	bool isAdjacent(const sf::Vector2f& a, const sf::Vector2f& b);
	bool isDiagonal(const sf::Vector2f& a, const sf::Vector2f& b);
	bool isOrthogonal(const sf::Vector2f& a, const sf::Vector2f& b);
	bool frogJump(int index, bool p1, const sf::Vector2f& dest);
	int getGridIndexAtPosition(const sf::Vector2f& p);

	sf::RenderWindow window; 
	sf::Font font;	
	sf::Text displayedMessage{ font };

	std::string player1Turn = "It's Player 1's Turn! (Yellow)";
	std::string player2Turn = "It's Player 2's Turn! (Red)";
	std::string player1Win = "Player 1, (Yellow), won!";
	std::string player2Win = "Player 2, (Red), won!";
	std::string draw = "It's a Draw!";

	std::vector<sf::RectangleShape> grid;
	std::vector<sf::RectangleShape> characterContainers;

	std::vector<sf::Texture> player1Textures;
	std::vector<sf::Sprite> player1Sprites;
	std::vector<sf::Texture> player2Textures;
	std::vector<sf::Sprite> player2Sprites;
	std::string player1Files[3] = { "ASSETS\\IMAGES\\Yellow\\Frog.png", "ASSETS\\IMAGES\\Yellow\\Snake.png", "ASSETS\\IMAGES\\Yellow\\Donkey.png"};
	std::string player2Files[3] = { "ASSETS\\IMAGES\\Red\\Frog.png", "ASSETS\\IMAGES\\Red\\Snake.png", "ASSETS\\IMAGES\\Red\\Donkey.png" };

	sf::RectangleShape messageBoard;
	
	int containerIndexMap[5] = { 0, 6, 4, 2, 8 };
	float cellSize;
	float scale;
	int gridCols;
	int selectedSprite = -1;
	std::vector<bool> placed1;
	std::vector<bool> placed2;
	int currentTurn;
	bool placementPhase = true;
	int selectedMoveSprite = -1;
	std::vector<sf::RectangleShape> moveHighlights;
	int containerCols;

	bool exitGame = false; 
};

#pragma warning( pop ) 
#endif 

