#include "pch.h"

#include "Global.h"
#include "Ghost.h"

Ghost::Ghost():
  m_dotLimit(0),
  m_eatenDots(0),
  m_mode(Global::Mode::Scatter)
{
  SetMovement(Character::Movement::Stop);
  SetColumnsAndRowsOfAssociatedSpriteSheet(8, 7);
  SetSpriteScaleFactor(Global::ghostSize);
  SetFramesPerState(2);
}
