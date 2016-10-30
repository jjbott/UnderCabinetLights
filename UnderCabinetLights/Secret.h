#include "Animation.h"
#include <deque>
#pragma once

class Secret
{
  public:
    Secret();
    Animation* Knock(int test, int test2);
  private:
    std::deque<uint32_t> _knocks;
    uint32_t _lastKnock;
};
