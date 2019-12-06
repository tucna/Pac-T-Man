#pragma once

#include "Global.h"
#include "Character.h"

class Ghost : public Character
{
public:
  Ghost();
  ~Ghost();

  void SetMode(Global::Mode mode) { m_mode = mode; }
  void SetDotLimit(uint8_t dotLimit) { m_dotLimit = dotLimit; }

  void IncrementEatenDots() { m_eatenDots++; }

  Global::Mode GetMode() { return m_mode; }

private:
  uint8_t m_dotLimit;
  uint8_t m_eatenDots;

  Global::Mode m_mode;
};

