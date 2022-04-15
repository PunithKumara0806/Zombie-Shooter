#include<sstream>
#include<fstream>
#include<SFML/Graphics.hpp>
#include<SFML/Audio.hpp>
#include "Player.h"
#include "ZombieArena.h"
#include "TextureHolder.h"
#include "Bullet.h"
#include "Pickup.h"
#include<iostream>//for debugin

using namespace sf;

int main() {
	//Here is the instance of TextureHolder
	TextureHolder holder;

	// The game will always be in one of four states
	enum class State { PAUSED, LEVELING_UP, GAME_OVER, PLAYING };
	State state = State::GAME_OVER;
	Vector2f resolution;
	resolution.x = VideoMode::getDesktopMode().width;
	resolution.y = VideoMode::getDesktopMode().height;
	float scaleX = resolution.x / 1980;
	float scaleY = resolution.y / 1080;

	RenderWindow window(VideoMode(resolution.x,resolution.y),"Zombie Arena",Style::Fullscreen);

	View mainView(FloatRect(0, 0, resolution.x, resolution.y));
	//minimap
	View minimap;
	minimap.setViewport(FloatRect(0.8f, 0.1f, 0.2f, 0.2f*resolution.x/resolution.y));
	//playerblip
	RectangleShape playerblip;
	playerblip.setSize(Vector2f(10, 10));
	playerblip.setFillColor(Color::Red);
	//Pickup blips
	RectangleShape Pickupblip;
	Pickupblip.setSize(Vector2f(20, 20));
	Pickupblip.setFillColor(Color::Blue);
	//Health blilp
	RectangleShape Healthblip;
	Healthblip.setSize(Vector2f(20, 20));
	Healthblip.setFillColor(Color::Green);

	Vector2f minimapsize;
	minimapsize.x = resolution.x * 0.2f;
	minimapsize.y = minimapsize.x;
	//minimap background
	RectangleShape minimapBackground;
	minimapBackground.setSize(minimapsize);
	minimapBackground.setFillColor(Color::White);
	minimapBackground.setPosition(0, 0);

	Clock clock;
	Time gameTimeTotal;
	
	Vector2f mouseWorldPosition;
	Vector2i mouseScreenPosition;

	Player player;
	player.getSprite().setPosition(resolution.x,resolution.y);
	IntRect arena;
	//create the background
	VertexArray background;
	Texture textureBackground = TextureHolder::GetTexture("graphics/background_sheet.png");

	//Prepare for  a horde of zombies
	int numZombies = 0;
	int numZombiesAlive;
	Zombie* zombies = nullptr;

	//bullet constraints
	Bullet bullets[100];
	int currentBullet = 0;
	int bulletsSpare = 24;
	int bulletsInClip = 8;
	int clipSize = 8;
	float fireRate = 2;
	//last pressed time to shoot
	Time lastPressed;
	// replacing mouse cursor with cross hair
	window.setMouseCursorVisible(false);
	Sprite spriteCrosshair;
	Texture textureCrosshair = TextureHolder::GetTexture("graphics/crosshair.png");
	spriteCrosshair.setTexture(textureCrosshair);
	spriteCrosshair.setOrigin(25, 25);

	//pickups
	Pickup healthPickup(1);
	Pickup ammoPickup(2);

	//Score
	int score = 0;
	int hiScore = 0;

	//For the home/game over screen
	Sprite spriteGameOver;
	Texture textureGameOver = TextureHolder::GetTexture("graphics/background.png");
	spriteGameOver.setTexture(textureGameOver);
	spriteGameOver.setPosition(0, 0);

	//Create a view for the HUD
	View hudView(FloatRect(0, 0, resolution.x, resolution.y));
	//Create a sprite for the ammo icon
	Sprite spriteAmmoIcon;
	Texture textureAmmoIcon = TextureHolder::GetTexture("graphics/ammo_icon.png");
	spriteAmmoIcon.setTexture(textureAmmoIcon);
	spriteAmmoIcon.setPosition(0, 0);

	//Load the font
	Font font;
	font.loadFromFile("fonts/KOMIKAP_.ttf");

	//Paused text
	Text pausedText;
	pausedText.setFont(font);
	pausedText.setCharacterSize(155*scaleX);
	pausedText.setFillColor(Color::White);
	pausedText.setPosition(400*scaleX, 400*scaleY);
	pausedText.setString("Press Enter \n to Continue");

	//Game Over
	Text gameOverText;
	gameOverText.setFont(font);
	gameOverText.setCharacterSize(125*scaleX);
	gameOverText.setFillColor(Color::White);
	gameOverText.setPosition(250*scaleX, 850*scaleY);
	gameOverText.setString("Press Enter to play");

	//Leveling up
	Text levelUpText;
	levelUpText.setFont(font);
	levelUpText.setCharacterSize(80*scaleX);
	levelUpText.setFillColor(Color::White);
	levelUpText.setPosition(150*scaleX, 250*scaleY);
	std::stringstream levelUpStream;
	levelUpStream<< "1- Increased rate of fire" <<
		"\n2- Increased clip size(next reload)" <<
		"\n3- Increased max health" <<
		"\n4- Increased run speed" <<
		"\n5- More and better health pickups" <<
		"\n6- More and better ammo pickups";
	levelUpText.setString(levelUpStream.str());

	//Ammo
	Text ammoText;
	ammoText.setFont(font);
	ammoText.setCharacterSize(55*scaleX);
	ammoText.setFillColor(Color::White);
	ammoText.setPosition(200*scaleX, 980*scaleY);

	//Score
	Text scoreText;
	scoreText.setFont(font);
	scoreText.setCharacterSize(55*scaleX);
	scoreText.setFillColor(Color::White);
	scoreText.setPosition(spriteAmmoIcon.getGlobalBounds().width + 50, 0);

	//Load the high score from a text file
	std::ifstream inputFile("gamedata/scores.txt");
	if (inputFile.is_open()) {
		inputFile >> hiScore;
		inputFile.close();
	}
	// Hi Score
	Text hiScoreText;
	hiScoreText.setFont(font);
	hiScoreText.setCharacterSize(55);
	hiScoreText.setFillColor(Color::White);
	hiScoreText.setPosition(1350*scaleX, 0);
	std::stringstream s;
	s << "Hi Score:" << hiScore;
	hiScoreText.setString(s.str());

	// Zombies remaining
	Text zombiesRemainingText;
	zombiesRemainingText.setFont(font);
	zombiesRemainingText.setCharacterSize(55*scaleX);
	zombiesRemainingText.setFillColor(Color::White);
	zombiesRemainingText.setPosition(1500*scaleX, 980*scaleY);
	zombiesRemainingText.setString("Zombies: 100");

	// Wave number
	int wave = 0;
	Text waveNumberText;
	waveNumberText.setFont(font);
	waveNumberText.setCharacterSize(55*scaleX);
	waveNumberText.setFillColor(Color::White);
	waveNumberText.setPosition(1250*scaleX, 980*scaleY);
	waveNumberText.setString("Wave: 0");

	// Health bar
	RectangleShape healthBar;
	healthBar.setFillColor(Color::Red);
	healthBar.setPosition(450*scaleX, 980*scaleY);
	//Default size of Health bar
	RectangleShape DefaulthealthBar;
	DefaulthealthBar.setFillColor(Color::White);
	DefaulthealthBar.setPosition(450 * scaleX, 980 * scaleY);
	DefaulthealthBar.setSize(Vector2f(player.getHealth() * 3, 50));


	//When did we last updte the hud
	int framesSinceLastHUDUpdate = 0;
	//how often we update
	int fpsMeasurementFrameInterval = 1000;

	//Prepare the hit sound
	SoundBuffer hitBuffer;
	hitBuffer.loadFromFile("sound/hit.wav");
	Sound hit;
	hit.setBuffer(hitBuffer);

	//Prepare the splat sound 
	SoundBuffer splatBuffer;
	splatBuffer.loadFromFile("sound/splat.wav");
	Sound splat;
	splat.setBuffer(splatBuffer);

	//Prepare the shoot sound
	SoundBuffer shootBuffer;
	shootBuffer.loadFromFile("sound/shoot.wav");
	Sound shoot;
	shoot.setBuffer(shootBuffer);

	//Prepare the reload sound
	SoundBuffer reloadBuffer;
	reloadBuffer.loadFromFile("sound/reload.wav");
	Sound reload;
	reload.setBuffer(reloadBuffer);

	//Prepare the reload failed sound
	SoundBuffer reloadFailedBuffer;
	reloadFailedBuffer.loadFromFile("sound/reload_failed.wav");
	Sound reloadFailed;
	reloadFailed.setBuffer(reloadFailedBuffer);

	//Prepare the powerup sound
	SoundBuffer powerupBuffer;
	powerupBuffer.loadFromFile("sound/powerup.wav");
	Sound powerup;
	powerup.setBuffer(powerupBuffer);

	//Prepare the pickup sound
	SoundBuffer pickupBuffer;
	pickupBuffer.loadFromFile("sound/pickup.wav");
	Sound pickup;
	pickup.setBuffer(pickupBuffer);

	//main game loop
	while (window.isOpen())
	{




		Event event;
		while (window.pollEvent(event)) {
			if (event.type == Event::KeyPressed) {
				if (event.key.code == Keyboard::Return && state == State::PLAYING) {
					state = State::PAUSED;
				}
				else if (event.key.code == Keyboard::Return && state == State::PAUSED) {
					state = State::PLAYING;
					clock.restart();
				}
				else if (event.key.code == Keyboard::Return && state == State::GAME_OVER) {
					state = State::LEVELING_UP;
					wave = 0;
					score = 0;

					//Prepare the gun and ammo for next game
					currentBullet = 0;
					bulletsSpare = 24;
					bulletsInClip = 8;
					clipSize = 8;
					fireRate = 2;

					//Reset the player's state
					player.resetPlayerStats();
				}
				if (state == State::PLAYING) {
					//Bullet Reloading
					if (event.key.code == Keyboard::R){
						if (bulletsSpare >= clipSize && bulletsInClip < clipSize) {
							//bullets are available for reload;
							bulletsInClip = clipSize;
							bulletsSpare -= clipSize;
							reload.play();
						}
						else if (bulletsSpare > 0) {
							//few bullets left
							bulletsInClip = bulletsSpare;
							bulletsSpare = 0;
							reload.play();
						}
						else {
							reloadFailed.play();
						}
					}
				}
				
			}
		}
		// Handle the player quitting
		if (Keyboard::isKeyPressed(Keyboard::Escape))
		{
			window.close();
		}
		if (state == State::PLAYING) {
			// Handle the pressing and releasing of the WASD keys
			if (Keyboard::isKeyPressed(Keyboard::W))
			{
				player.moveUp();
			
			}
			else
			{
				player.stopUp();
			}
			if (Keyboard::isKeyPressed(Keyboard::S))
			{
				player.moveDown();
			
			}
			else
			{
				player.stopDown();
			}
			if (Keyboard::isKeyPressed(Keyboard::A))
			{
				player.moveLeft();
				
			}
			else
			{
				player.stopLeft();
			}
			if (Keyboard::isKeyPressed(Keyboard::D))
			{
				player.moveRight();
				
			}
			else
			{
				player.stopRight();
			}
			//Fire a bullet
			if (Mouse::isButtonPressed(sf::Mouse::Left)) {
				if (gameTimeTotal.asMilliseconds() - lastPressed.asMilliseconds() > 1000 / fireRate && bulletsInClip > 0) {
					//passing the players location to shoot function
					bullets[currentBullet].shoot(player.getCenter().x, player.getCenter().y, mouseWorldPosition.x, mouseWorldPosition.y);
					currentBullet++;
					if (currentBullet > 99) {
						currentBullet = 0;
					}
					lastPressed = gameTimeTotal;
					shoot.play();
					bulletsInClip--;
				}
			}//End fire a bullet
		}//End WASD while playing

		// Handle the LEVELING up state
		if (state == State::LEVELING_UP)
		{
			// Handle the player LEVELING up
			if (event.key.code == Keyboard::Num1)
			{
				//Increase Fire rate
				fireRate++;
				state = State::PLAYING;
			}
			else if (event.key.code == Keyboard::Num2)
			{
				//Increase clip size
				clipSize += clipSize;
				state = State::PLAYING;
			}
			else if (event.key.code == Keyboard::Num3)
			{
				//Increase health
				player.upgradeHealth();
				state = State::PLAYING;
			}
			else if (event.key.code == Keyboard::Num4)
			{
				//Increase speed
				player.upgradeSpeed();
				state = State::PLAYING;
			}
			else if (event.key.code == Keyboard::Num5)
			{
				//Upgrade pickup
				healthPickup.upgrade();
				state = State::PLAYING;
			}
			else if (event.key.code == Keyboard::Num6)
			{
				//Upgrade pickup
				ammoPickup.upgrade();
				state = State::PLAYING;
			}
			if (state == State::PLAYING)
			{
				//Increase the wave number
				wave++;
				// Prepare the level
				// We will modify the next two lines later
				arena.width = 500*wave;
				arena.height = 500*wave;
				arena.left = 0;
				arena.top = 0;

				//pass the vertex array by reference
				int tileSize = CreateBackground(background, arena);
				
				player.spawn(arena, resolution, tileSize);
				//Configue the pickups
				healthPickup.setArena(arena);
				ammoPickup.setArena(arena);
				//Create a horde of zombies
				numZombies = 5*wave;

				//Delete the previously allocated memory (if it exists)
				delete[] zombies;
				zombies = createHorde(numZombies, arena);
				numZombiesAlive = numZombies;
				
				//play the powerup sound 
				powerup.play();

				// Reset the clock so there isn't a frame jump
				clock.restart();
			}
		}// End LEVELING up
		/*
		****************
		UPDATE THE FRAME
		****************
		*/
		if (state == State::PLAYING)
		{
			// Update the delta time
			Time dt = clock.restart();
			// Update the total game time
			gameTimeTotal += dt;
			// Make a decimal fraction of 1 from the delta time
			float dtAsSeconds = dt.asSeconds();
			// Where is the mouse pointer
			if (Mouse::getPosition(window).x >= 0 && Mouse::getPosition(window).y >= 0 && Mouse::getPosition(window).x <= resolution.x && Mouse::getPosition(window).y <= resolution.y)
				mouseScreenPosition = Mouse::getPosition(window);
			// Convert mouse position to world coordinates of mainView
			mouseWorldPosition = window.mapPixelToCoords(Mouse::getPosition(window), mainView);
			//Set the crosshari to the mouse world location
			spriteCrosshair.setPosition(mouseWorldPosition);

			// Update the player
			player.update(dtAsSeconds, mouseScreenPosition);//changed from mouse::getPosition to mouseScreenPosition
			// Make a note of the players new position
			Vector2f playerPosition(player.getCenter());
			// Make the view centre around the player
			mainView.setCenter(player.getCenter());

			//Loop through eac Zombie and update them
			for (int i = 0; i < numZombies; i++) {
				if (zombies[i].isAlive()) {
					zombies[i].update(dt.asSeconds(), playerPosition);
				}
			}

			//update any bullets that are in-flight
			for (int i = 0; i < 100; i++) {
				if (bullets[i].isInFlight()) {
					bullets[i].update(dtAsSeconds);
				}
			}
			//update pickups for spawn and respawn
			healthPickup.update(dtAsSeconds);
			ammoPickup.update(dtAsSeconds);
			Pickupblip.setPosition(Vector2f(minimapsize.x*ammoPickup.getPosition().left/arena.width, minimapsize.y*ammoPickup.getPosition().top/arena.height));
			Healthblip.setPosition(Vector2f(minimapsize.x* healthPickup.getPosition().left / arena.width, minimapsize.y* healthPickup.getPosition().top / arena.height));
			//Collision detection

			//Zombie being shot
			for (int i = 0; i < 100; i++) {
				for (int j = 0; j < numZombies; j++) {
					if (bullets[i].isInFlight() && zombies[j].isAlive()) {
						if (bullets[i].getPosition().intersects(zombies[j].getPosition())) {
							//Stop the bullet
							bullets[i].stop();

							//Register the hit and see if it was a kill
							if (zombies[j].hit()) {
								score += 10;
								if (score >= hiScore) {
									hiScore = score;
								}
								numZombiesAlive--;
								if (numZombiesAlive == 0) {
									state = State::LEVELING_UP;
								}
							}
							splat.play();
						}
					}
				}
			}//End zombie being shot

			//zombie touched the player
			for (int i = 0; i < numZombies; i++) {
				if (player.getPosition().intersects(zombies[i].getPosition()) && zombies[i].isAlive()) {
					if (player.hit(gameTimeTotal)) {
						hit.play();
					}
					if (player.getHealth() <= 0) {
						state = State::GAME_OVER;

						std::ofstream outputFile("gamedata/scores.txt");
						outputFile << hiScore;
						outputFile.close();
					}
				}
			}//End player touch
			
			 //touchin a health pickup
			if (player.getPosition().intersects(healthPickup.getPosition()) && healthPickup.isSpawned()) {
				player.increasedHealthLevel(healthPickup.gotIt());
				pickup.play();
			}
			//touchin a ammo pickup
			if (player.getPosition().intersects(ammoPickup.getPosition()) && ammoPickup.isSpawned()) {
				bulletsSpare += ammoPickup.gotIt();
				reload.play();
			}

			// size up the health bar
			healthBar.setSize(Vector2f(player.getHealth() * 3, 50));
			// Increment the number of frames since the previous update
			framesSinceLastHUDUpdate++;
			// re-calculate every fpsMeasurementFrameInterval frames
			if (framesSinceLastHUDUpdate > fpsMeasurementFrameInterval)
			{
				// Update game HUD text
				std::stringstream ssAmmo;
				std::stringstream ssScore;
				std::stringstream ssHiScore;
				std::stringstream ssWave;
				std::stringstream ssZombiesAlive;

				// Update the ammo text
				ssAmmo << bulletsInClip << "/" << bulletsSpare;
				ammoText.setString(ssAmmo.str());// Update the score text
				ssScore << "Score:" << score;
				scoreText.setString(ssScore.str());

				// Update the high score text
				ssHiScore << "Hi Score:" << hiScore;
				hiScoreText.setString(ssHiScore.str());

				// Update the wave
				ssWave << "Wave:" << wave;
				waveNumberText.setString(ssWave.str());

				// Update the high score text
				ssZombiesAlive << "Zombies:" << numZombiesAlive;
				zombiesRemainingText.setString(ssZombiesAlive.str());
				framesSinceLastHUDUpdate = 0;
			}// End HUD update

		}// End updating the scene
		/*
		**************
		Draw the scene
		**************
		*/
		if (state == State::PLAYING)
		{
			
			
			window.clear();
			// set the mainView to be displayed in the window
		// And draw everything related to it
			window.setView(mainView);
			//Draw the background
			window.draw(background, &textureBackground);
			//Draw the Zombies
			for (int i = 0; i < numZombies; i++) {
				window.draw(zombies[i].getSprite());
			}
			//Draw the bullet
			for (int i = 0; i < 100; i++) {
				if (bullets[i].isInFlight()) {
					window.draw(bullets[i].getShape());
				}
			}
			//Draw the pickups , if spawned
			if (ammoPickup.isSpawned()){
				window.draw(ammoPickup.getSprite());
			}
			if (healthPickup.isSpawned()) {
				window.draw(healthPickup.getSprite());
			}
			
			// Draw the player
			window.draw(player.getSprite());

			//Draw the crosshair
			window.draw(spriteCrosshair);
			// Switch to the HUD view
			window.setView(hudView);

			// Draw all the HUD elements
			window.draw(spriteAmmoIcon);
			window.draw(ammoText);
			window.draw(scoreText);
			window.draw(hiScoreText);
			window.draw(DefaulthealthBar);
			window.draw(healthBar);
			window.draw(waveNumberText);
			window.draw(zombiesRemainingText);
			window.setView(minimap);
			
			playerblip.setPosition(Vector2f(minimapsize.x*player.getCenter().x/arena.width, minimapsize.y * player.getCenter().y/arena.height));
			window.draw(minimapBackground);
			if (ammoPickup.isSpawned()) {
				window.draw(Pickupblip);
			}
			if (healthPickup.isSpawned()) {
				window.draw(Healthblip);
			}
			window.draw(playerblip);
			window.setView(hudView);
		}
		if (state == State::LEVELING_UP)
		{
			window.draw(spriteGameOver);
			window.draw(levelUpText);
		}
		if (state == State::PAUSED)
		{
			window.draw(pausedText);
		}
		if (state == State::GAME_OVER)
		{
			window.draw(spriteGameOver);
			window.draw(gameOverText);
			window.draw(scoreText);
			window.draw(hiScoreText);
		}
		window.display();
	}
	delete[] zombies;
	return 0;
}