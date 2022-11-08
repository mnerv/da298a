/**
 * @file   main.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @author Linnéa Mörk
 * @brief  sunlight embedded program
 * @date   2022-10-23
 *
 * @copyright Copyright (c) 2022
 */
#include "Arduino.h"

#include "sky.hpp"

void setup() {
    Serial.begin(115200);
}

void loop() {
    Serial.println(sky::hello());
    delay(250);
}

