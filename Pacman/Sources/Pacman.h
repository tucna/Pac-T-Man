#pragma once

#include "Character.h"

class Pacman : public Character
{
public:
  Pacman();
  ~Pacman();

private:
  uint8_t m_lives;
};

