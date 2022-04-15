#include"Zombie.h"
#include"TextureHolder.h"
#include <cstdlib>
#include<ctime>
#include<iostream>// for debugging

using namespace std;

void Zombie::spawn(float startX, float startY, int type, int seed) {

	switch (type)
	{
	case 0:
		//Bloater type
		m_Sprite = Sprite(TextureHolder::GetTexture("graphics/bloater.png"));
		m_Speed = BLOATER_SPEED;
		m_Health = BLOATER_HEALTH;
		m_angular_speed += BLOATER_SPEED;
		break;
	case 1:
		//Chaser type
		m_Sprite = Sprite(TextureHolder::GetTexture("graphics/chaser.png"));
		m_Speed = CHASER_SPEED;
		m_Health = CHASER_HEALTH;
		m_angular_speed += CHASER_SPEED;
		break;
	case 2:
		//Crawler type
		m_Sprite = Sprite(TextureHolder::GetTexture("graphics/crawler.png"));
		m_Speed = CRAWLER_SPEED;
		m_Health = CRAWLER_HEALTH;
		m_angular_speed += CRAWLER_SPEED;
		break;
	default:
		break;
	}

	//Modifying the Speed of every zombie to make it unique
	srand((int)time(0) * seed);
	// modifier range - 50 - 100
	float modifier = (rand() % MAX_VARRIANCE) + OFFSET;


	modifier /= 100;
	m_Speed *= modifier;
	m_angular_speed *= modifier;

	m_Position.x = startX;
	m_Position.y = startY;

	m_Sprite.setOrigin(25, 25);
	m_Sprite.setPosition(m_Position);
}

bool Zombie::hit() {
	m_Health--;
	if (m_Health < 0) {
		//dead
		m_Alive = false;
		m_Sprite.setTexture(TextureHolder::GetTexture("graphics/blood.png"));
		return true;
	}
	return false;
}

bool Zombie::isAlive() {
	return m_Alive;
}

FloatRect Zombie::getPosition() {
	return m_Sprite.getGlobalBounds();
}

Sprite Zombie::getSprite() {
	return m_Sprite;
}

void Zombie::update(float elapsedTime, Vector2f playerLocation) {
	float playerX = playerLocation.x;
	float playerY = playerLocation.y;

	if (playerX > m_Position.x) {
		m_Position.x = m_Position.x + m_Speed * elapsedTime;
	}
	if (playerY > m_Position.y) {
		m_Position.y = m_Position.y + m_Speed * elapsedTime;
	}

	if (playerX < m_Position.x) {
		m_Position.x = m_Position.x - m_Speed * elapsedTime;
	}
	if (playerY < m_Position.y) {
		m_Position.y = m_Position.y - m_Speed * elapsedTime;
	}

	m_Sprite.setPosition(m_Position);
	//face the sprite in the correct direction
	float angular_speed = m_angular_speed;
	float target_angle = (atan2(playerY - m_Position.y, playerX - m_Position.x) * 180 / 3.141);
	if (target_angle < 0)
		target_angle += 360;
	if (abs(target_angle - m_Sprite.getRotation()) > 5.0f)// step of 5.0f to avoid toggling
	{
		if (m_Sprite.getRotation() - target_angle < 180.0f && m_Sprite.getRotation() > target_angle) {
			angular_speed = -m_angular_speed;
		}
		else if (m_Sprite.getRotation() < target_angle && target_angle - m_Sprite.getRotation() > 180.0f) {
			angular_speed = -m_angular_speed;
		}

		m_Sprite.rotate(elapsedTime * angular_speed);
	}
	

	
}