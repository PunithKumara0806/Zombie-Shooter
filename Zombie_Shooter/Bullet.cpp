#include "Bullet.h"

Bullet::Bullet() {
	m_BulletShape.setSize(sf::Vector2f(6, 6));
}

void Bullet::shoot(float startX, float startY, float targetX, float targetY) {
	m_InFlight = true;
	m_Position.x = startX;
	m_Position.y = startY;

	// calculate the gradient of the flight path
	float gradient = (startX - targetX) / (startY - targetY);

	// taking absolute value of gradient
	if (gradient < 0)
		gradient *= -1;

	// calculating the slope of x,y
	float ratioXY = m_BulletSpeed / (1 + gradient);
	// 1 added to the denom to get
	//  speedX + speedY = speed
	// x and y step distance for each second, speed
	m_BulletDistanceX = ratioXY * gradient;
	m_BulletDistanceY = ratioXY;
	
	//Direction of the bullet
	if (targetX < startX)
		m_BulletDistanceX *= -1;
	if (targetY < startY)
		m_BulletDistanceY *= -1;
	// max range of the bullet from the start
	float range = 1000;
	m_MinX = startX - range;
	m_MaxX = startX + range;
	m_MinY = startY - range;
	m_MaxY = startY + range;

	m_BulletShape.setPosition(m_Position);
}

void Bullet::stop() {
	m_InFlight = false;
}

bool Bullet::isInFlight() {
	return m_InFlight;
}

FloatRect Bullet::getPosition() {
	return m_BulletShape.getGlobalBounds();
}

RectangleShape Bullet::getShape() {
	return m_BulletShape;
}

void Bullet::update(float elapsedTime) {
	m_Position.x += m_BulletDistanceX * elapsedTime;
	m_Position.y += m_BulletDistanceY * elapsedTime;

	m_BulletShape.setPosition(m_Position);
	// range checking
	if (m_Position.x < m_MinX || m_Position.x > m_MaxX || m_Position.y < m_MinY || m_Position.y > m_MaxY)
		m_InFlight = false;
}