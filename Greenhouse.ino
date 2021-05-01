/***************************************************
  Greenhouse monitor

 ****************************************************/

#include "SystemManager.h"

SystemManager systemManager;

void setup() {
    systemManager.init();
}

void loop() {
    systemManager.run();
}
