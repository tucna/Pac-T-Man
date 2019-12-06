#include "pch.h"

#include "Pacman.h"

Pacman::Pacman():
  m_lives(3)
{
  SetPosition(10.5f, 0.25f, 9.5f);
  SetColumnsAndRowsOfAssociatedSpriteSheet(12, 2);
  SetSpriteScaleFactor(Global::pacManSize);
  SetMovement(Character::Movement::Stop);
  SetSpriteY(0);
  SetFramesPerState(2);
}

Pacman::~Pacman()
{
}
