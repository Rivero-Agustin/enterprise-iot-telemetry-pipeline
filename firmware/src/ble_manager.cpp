#include "ble_manager.h"
#include "config.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h> // Necesario para enviar notificaciones al celular
#include "esp_log.h"

extern QueueHandle_t bleCommandQueue;

// Punteros globales para manejar la sesión
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;

static const char* LOGTAG = "BLE";

// Clase para detectar cuándo un celular se conecta o desconecta
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        ESP_LOGI(LOGTAG, "Celular conectado por BLE");
    };
    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        ESP_LOGI(LOGTAG, "Celular desconectado");
        BLEDevice::startAdvertising();
    }
};

class CommandCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pChar) {
        // Obtenemos el valor crudo que envió el celular
        std::string rxValue = pChar->getValue();

        if (rxValue.length() > 0) {
            String comando = String(rxValue.c_str());
            comando.trim(); 
            ESP_LOGI(LOGTAG, "Comando recibido: %s", comando.c_str());

            // Convertimos el String de Arduino a un array de caracteres de C
            char cmdBuffer[50];
            strncpy(cmdBuffer, comando.c_str(), sizeof(cmdBuffer) - 1);
            cmdBuffer[sizeof(cmdBuffer) - 1] = '\0'; 

            // Empujamos el mensaje hacia el Núcleo 1
            xQueueSend(bleCommandQueue, &cmdBuffer, 0);

            if (comando == "DORMIR") {
                ESP_LOGI(LOGTAG, "Ejecutando rutina de ahorro de energia...");
            } 
            else if (comando == "CALIBRAR") {
                ESP_LOGI(LOGTAG, "Calibrando antena UWB...");
            }
            else if (comando == "REINICIAR") {
                ESP_LOGW(LOGTAG, "Reiniciando ESP32...");
                ESP.restart(); // Comando nativo para reiniciar la placa
            }
            else {
                ESP_LOGE(LOGTAG, "Comando desconocido, ignorando.");
            }
        }
    }
};

void initBLE(const char* deviceName) {
    // 1. Iniciamos el hardware BLE con el nombre de tu placa
    BLEDevice::init(deviceName);

    // 2. Creamos el Servidor y le asignamos nuestros callbacks (conexión/desconexión)
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // 3. Creamos el Servicio (El Menú)
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // 4. Creamos la Característica (El Plato principal: Lectura y Notificación)
    pCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID,
                        BLECharacteristic::PROPERTY_READ   |
                        BLECharacteristic::PROPERTY_NOTIFY
                      );

    // Descriptor estándar para que el celular sepa que puede suscribirse a datos en vivo
    pCharacteristic->addDescriptor(new BLE2902());

    // NUEVO: Creamos la Característica de Comandos (RX - Recibir)
    BLECharacteristic *pCommandChar = pService->createCharacteristic(
                                        COMMAND_UUID,
                                        BLECharacteristic::PROPERTY_WRITE
                                      );

    // Le conectamos las "orejas" (la clase que creamos arriba)
    pCommandChar->setCallbacks(new CommandCallbacks());

    // 5. Encendemos el servicio
    pService->start();

    // 6. ¡Comenzamos a gritar nuestra existencia al mundo (Advertising)!
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    // Funciones recomendadas para dispositivos Apple/Android
    pAdvertising->setMinPreferred(0x06); 
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    ESP_LOGI(LOGTAG, "Bluetooth iniciado. Dispositivo ahora es VISIBLE");
}

void updateBLEDistance(float distance) {
    // Solo enviamos datos si hay un celular físicamente conectado
    if (deviceConnected && pCharacteristic != NULL) {
        // Convertimos el float de distancia a un texto para enviarlo por radio
        char buffer[10];
        dtostrf(distance, 4, 2, buffer); // 4 caracteres totales, 2 decimales
        
        pCharacteristic->setValue(buffer);
        pCharacteristic->notify(); // ¡Empujamos el dato a la pantalla del celular!
    }
}