#include "application.h"

#pragma once

#define PIXEL_COUNT_STRIP1 300
#define PIXEL_COUNT_STRIP2 0

class Config
{
  public:
    static const uint8_t FPS = 60;
    static const IPAddress PublishToIp;
    static const int PublishToPort = 4040;
  //private:
    static constexpr uint8_t _serverIp[4] = {192, 168, 1, 5};
};
