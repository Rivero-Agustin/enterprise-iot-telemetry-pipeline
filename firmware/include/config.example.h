#ifndef CONFIG_H
#define CONFIG_H

// --- CONFIGURACIÓN DE ROL (Ancla o Etiqueta) ---
#define IS_ANCHOR true

/* =======================================================
 * CREDENCIALES DE RED Y NUBE (Reemplazar con datos reales)
 * ======================================================= */
#define WIFI_SSID "TU_RED_WIFI"
#define WIFI_PASS "TU_CONTRASEÑA"

// AWS IoT Core Endpoint (Broker MQTT)
#define ENDPOINT "mqtts://xxxxxxxxxxxxxx-ats.iot.us-east-1.amazonaws.com:8883"

/* =======================================================
 * CONFIGURACIÓN DE PROVISIONAMIENTO (JITP)
 * ======================================================= */
#define TOPIC_CREATE_CERT_REQ "$aws/certificates/create/json"
#define TOPIC_CREATE_CERT_ACC "$aws/certificates/create/json/accepted"
#define TOPIC_REGISTER_REQ    "$aws/provisioning-templates/PlantillaESP32UWB/provision/json"
#define TOPIC_REGISTER_ACC    "$aws/provisioning-templates/PlantillaESP32UWB/provision/json/accepted"

/* =======================================================
 * BLUETOOTH LOW ENERGY (BLE) UUIDS
 * ======================================================= */
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8" // Envía distancias
#define COMMAND_UUID        "c0de0001-feed-4688-b7f5-ea07361b26a8" // Recibe comandos

#endif // CONFIG_H