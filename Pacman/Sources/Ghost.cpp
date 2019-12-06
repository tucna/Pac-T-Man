#include "pch.h"

#include "Ghost.h"

Ghost::Ghost():
  m_dotLimit(0),
  m_eatenDots(0),
  m_mode(Global::Mode::Scatter)
{
}

Ghost::~Ghost()
{
}
