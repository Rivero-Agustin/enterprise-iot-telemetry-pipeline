#pragma once // Evita inclusiones duplicadas
#include <Arduino.h>

char* read_cert_from_nvs(void);
char* read_private_key_from_nvs(void);
void init_nvs(void);
bool is_device_provisioned(void);
void save_device_credentials(const char* cert, const char* private_key);