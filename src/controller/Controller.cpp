

#include "Controller.hpp"

Controller controller;

void setup() {
    controller.begin();
}

void loop() {
    controller.update();
}