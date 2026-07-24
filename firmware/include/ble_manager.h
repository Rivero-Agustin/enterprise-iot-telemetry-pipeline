#pragma once
#include <Arduino.h>

// Funciones públicas que el main podrá usar
void initBLE(const char* deviceName);
void updateBLEDistance(float distance);