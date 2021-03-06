#include <iostream>
#include <string>
#include "SFML/Graphics.hpp"
#include "board.cpp"
#include "server.cpp"
#include "client.cpp"
// #include "grid.cpp"
#define WINDOWW 800
#define WINDOWH 600
#define OFFLINE 0
#define SERVER 1
#define CLIENT 2
using namespace sf;
using namespace std;

// This disaster is because I didn't use header files
void Server::broadcast(Packet packet) {
	for (int n=0; n<2; n++) {
		if (isOnlineClient[n]) clientSocket[n].send(packet);
		else clients[n]->receiveOffline(packet);
	}
}

int gamemode;
int playerTurn = 0;
Server* server;
Client* client1;
Client* client2;
int playerWin;
Text playerNText;
Text playerTurnText;

int main() {
	
	// Create the game board
	int (*line)[4][3];
	
	cout << "Enter gamemode (0=OFFLINE, 1=SERVER, 2=CLIENT): " << endl;
	cin >> gamemode;
	
	// Initialize text
	Font font;
	font.loadFromFile("C:/Windows/Fonts/arial.ttf");
	
	playerNText.setFont(font);
	playerNText.setFillColor(Color::Black);
	playerNText.setCharacterSize(20);
	playerNText.setPosition(Vector2f(50, 50));
	
	playerTurnText.setFont(font);
	playerTurnText.setFillColor(Color::Black);
	playerTurnText.setCharacterSize(20);
	playerTurnText.setPosition(Vector2f(WINDOWW/2, 2*WINDOWH/3));
	
	// Initialize server and client
	if (gamemode == OFFLINE) {
		server = new Server();
		client1 = new Client(0, server);
		client2 = new Client(1, server);
		server->addClient(0, client1);
		server->addClient(1, client2);
		client1->myTurn = true;
		client2->myTurn = false;
	}
	else if (gamemode == SERVER) {
		server = new Server();
		client1 = new Client(0, server);
		server->addClient(0, client1);
		server->addClient(1);
		client1->myTurn = true;
		playerNText.setString("Player 1");
	}
	else if (gamemode == CLIENT) {
		client2 = new Client(1);
		client2->myTurn = false;
		playerNText.setString("Player 2");
	}
	
	// Create the window
	RenderWindow window(VideoMode(WINDOWW, WINDOWH), "cubetac");
	
	// Set the window's framerate to the monitor's refresh rate
	window.setVerticalSyncEnabled(true);
	
	// Main loop
	while (window.isOpen()) {
		
		// Handle network events
		if (gamemode == SERVER && !client1->myTurn) {
			server->receiveOnline(1);
		}
		else if (gamemode == CLIENT && !client2->myTurn) {
			client2->receiveOnline();
		}
		
		// Clear the render window
		window.clear(Color::White);
		
		if (gamemode == OFFLINE) {
			Board* board = client1->getBoard();
			window.draw(*board);
			if (client1->myTurn) playerTurn = 1;
			else playerTurn = 2;
			playerWin = board->playerWin();
		}
		if (gamemode == SERVER || gamemode == OFFLINE) {
			window.draw(playerNText);
			Board* board = client1->getBoard();
			window.draw(*board);
			if (client1->myTurn) playerTurn = 1;
			else playerTurn = 2;
			playerWin = board->playerWin();
		}
		else if (gamemode == CLIENT) {
			window.draw(playerNText);
			Board* board = client2->getBoard();
			window.draw(*board);
			if (client2->myTurn) playerTurn = 2;
			else playerTurn = 1;
			playerWin = board->playerWin();
		}
		
		if (playerWin == 0) playerTurnText.setString("Player "+to_string(playerTurn)+"'s turn");
		else playerTurnText.setString("Player "+to_string(playerWin)+" wins!");
		// Center the text
		{
			int w = playerTurnText.getCharacterSize();
			int l = playerTurnText.getString().getSize();
			playerTurnText.setPosition((WINDOWW/2) - (w*l/4), 2*WINDOWH/3);
		}
		window.draw(playerTurnText);

		// End the frame
		window.display();
		
		// Handle the window's events
		Event event;
		while (window.pollEvent(event)) {
			
			// If a mouse button is pressed
			if (event.type == Event::MouseButtonPressed) {
				// If the left mouse button is pressed
				if (event.mouseButton.button == Mouse::Left) {
					// If no player has won yet
					if (playerWin == 0) {
						if (gamemode == SERVER && client1->myTurn) {
							client1->onMouseClick(Vector2f(event.mouseButton.x, event.mouseButton.y));
						}
						else if (gamemode == CLIENT && client2->myTurn) {
							client2->onMouseClick(Vector2f(event.mouseButton.x, event.mouseButton.y));
						}
						else if (gamemode == OFFLINE) {
							if (client1->myTurn) client1->onMouseClick(Vector2f(event.mouseButton.x, event.mouseButton.y));
							else if (client2->myTurn) client2->onMouseClick(Vector2f(event.mouseButton.x, event.mouseButton.y));
						}
					}
				}
			}
			
			// If the window is closed
			if (event.type == Event::Closed) {
				// Free memory
				if (gamemode == OFFLINE) {
					delete server;
					delete client1;
					delete client2;
				} else if (gamemode == SERVER) {
					delete server;
					delete client1;
				} else if (gamemode == CLIENT) {
					delete client2;
				}
				window.close();
			}
			
		}
		
	}
	
	return 0;
	
}