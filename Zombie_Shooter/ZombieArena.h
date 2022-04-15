#pragma once
#include<SFML/Graphics.hpp>
#include"Zombie.h"

using namespace sf;

int CreateBackground(VertexArray& rVA, IntRect arena);
Zombie* createHorde(int numZombie, IntRect arena);