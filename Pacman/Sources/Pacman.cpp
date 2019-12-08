#include "pch.h"

#include "Pacman.h"

Pacman::Pacman()
{
  SetPosition(10.5f, 0.25f, 9.5f);
  SetColumnsAndRowsOfAssociatedSpriteSheet(12, 2);
  SetSpriteScaleFactor(Global::pacManSize);
  SetMovement(Character::Movement::Left);
  SetSpriteY(0);
  SetFramesPerState(2);
}
