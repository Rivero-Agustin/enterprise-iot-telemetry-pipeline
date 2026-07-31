#include "mqtt_manager.h"
#include "certs.h"
#include "uwb_engine.h"
#include "nvs_manager.h"
#include "config.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "cJSON.h"
#include <string.h>

static const char* LOGTAG = "MQTT";

// Variables globales para la tarea
TaskHandle_t telemetry_task_handle = NULL;
esp_mqtt_client_handle_t global_mqtt_client = NULL;

bool provisioning_completed = false;

char ownership_token[1024] = {0};

bool parse_aws_certificates_response(const char* json_data) {
    cJSON *root = cJSON_Parse(json_data);
    if (root == NULL) {
        ESP_LOGE("PROVISIONING", "Error parseando el JSON de AWS");
        return false;
    }

    cJSON *certPem = cJSON_GetObjectItem(root, "certificatePem");
    cJSON *privateKey = cJSON_GetObjectItem(root, "privateKey");
    cJSON *token = cJSON_GetObjectItem(root, "certificateOwnershipToken");

    if (cJSON_IsString(certPem) && cJSON_IsString(privateKey) && cJSON_IsString(token)) {
        // 1. Guardar en memoria NVS (Física)
        save_device_credentials(certPem->valuestring, privateKey->valuestring);
        
        // 2. Guardar token en memoria RAM para el siguiente paso MQTT
        strncpy(ownership_token, token->valuestring, sizeof(ownership_token) - 1);
        
        ESP_LOGI("PROVISIONING", "Certificados guardados exitosamente.");
        cJSON_Delete(root); // ¡CRÍTICO! Liberar la memoria RAM
        return true;
    }

    cJSON_Delete(root);
    return false;
}

// Handler que procesa todos los eventos de la conexión MQTT
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    // Cambiamos %ld por %d porque event_id es int32_t
    ESP_LOGD(LOGTAG, "Evento despachado desde el bucle base=%s, event_id=%d", base, (int)event_id);
    
    // C++ requiere casting explícito desde void*
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    switch ((esp_mqtt_event_id_t)event_id) {
        
        case MQTT_EVENT_CONNECTED: {

            ESP_LOGI(LOGTAG, "Conectado a AWS IoT");
            // Si estamos en modo aprovisionamiento (NO tenemos certs en NVS)
            if (!is_device_provisioned()) {
                // Paso 1: Suscribirse a la respuesta de los certificados
                esp_mqtt_client_subscribe(client, TOPIC_CREATE_CERT_ACC, 1);
                // Paso 2: Pedir los certificados mandando un JSON vacío
                esp_mqtt_client_publish(client, TOPIC_CREATE_CERT_REQ, "{}", 2, 1, 0);
            } else {
                // Modo Normal: Suscribirte a tus tópicos de trabajo habituales
                ESP_LOGI("MQTT", "Modo Operativo Normal. Dispositivo ya registrado.");
                esp_mqtt_client_subscribe(client, "esp32/comandos", 1);
        
                // Lanzamos la tarea de FreeRTOS si no estaba corriendo ya
                if (telemetry_task_handle == NULL) {
                    xTaskCreate(
                        uwb_telemetry_task,      // Puntero a la función de la tarea
                        "uwb_telemetry_task",    // Nombre para debug
                        4096,                    // Tamaño del Stack (En bytes para ESP-IDF)
                        NULL,                    // Parámetros
                        5,                       // Prioridad (5 es estándar/alta)
                        &telemetry_task_handle   // Manejador
                    );
                }

            }
            break;
        } 
            
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(LOGTAG, "MQTT_EVENT_DISCONNECTED: Se perdió la conexión con AWS");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(LOGTAG, "MQTT_EVENT_SUBSCRIBED: AWS confirmó la suscripción, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(LOGTAG, "MQTT_EVENT_PUBLISHED: AWS confirmó la recepción del dato, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(LOGTAG, "MQTT_EVENT_DATA: ¡Mensaje recibido desde la nube!");
            printf("Tópico: %.*s\r\n", event->topic_len, event->topic);
            printf("Datos: %.*s\r\n", event->data_len, event->data);

            // Comprobamos de qué tópico viene el mensaje
            if (strncmp(event->topic, TOPIC_CREATE_CERT_ACC, event->topic_len) == 0) {
                ESP_LOGI("MQTT", "AWS nos envió los certificados nuevos!");
                
                // Parseamos y guardamos el JSON (llamamos a nuestra función)
                if (parse_aws_certificates_response(event->data)) {
                    // Paso 3: Suscribirse al registro de plantilla
                    esp_mqtt_client_subscribe(client, TOPIC_REGISTER_ACC, 1);
                    
                    // Paso 4: Armar el JSON de registro con la MAC como SerialNumber
                    uint8_t mac[6];
                    esp_efuse_mac_get_default(mac);
                    char payload[2048];
                    snprintf(payload, sizeof(payload), 
                        "{\"certificateOwnershipToken\": \"%s\", \"parameters\": {\"SerialNumber\": \"%02x%02x%02x%02x%02x%02x\"}}", 
                        ownership_token, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

                    // Publicar la petición de registro
                    esp_mqtt_client_publish(client, TOPIC_REGISTER_REQ, payload, 0, 1, 0);
                }
            } 
            else if (strncmp(event->topic, TOPIC_REGISTER_ACC, event->topic_len) == 0) {
                // Ejecutamos el reinicio
                ESP_LOGW("MQTT", "¡Aprovisionamiento JITP completado! Reiniciando ESP32 en 2 seg...");
                vTaskDelay(2000 / portTICK_PERIOD_MS);
                esp_restart();
            }
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGE(LOGTAG, "MQTT_EVENT_ERROR: Error en la conexión");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                ESP_LOGE(LOGTAG, "Error del socket TCP/TLS: %s", strerror(event->error_handle->esp_transport_sock_errno));
            }
            break;

        default:
            ESP_LOGI(LOGTAG, "Evento MQTT no manejado, id: %d", event->event_id);
            break;
    }
}

void mqtt_app_start(void)
{
    // BARRERA DE SEGURIDAD: Si es un Tag, abortamos la inicialización de MQTT para ahorrar batería
    if (!IS_ANCHOR) {
        ESP_LOGI(LOGTAG, "Rol: TAG. Se deshabilita el cliente MQTT para ahorrar energía.");
        return;
    }

    // En C++ inicializamos el struct en 0 de esta manera:
    esp_mqtt_client_config_t mqtt_cfg = {};
    
    // Y asignamos los valores línea por línea para evitar errores del compilador
    mqtt_cfg.uri = ENDPOINT; // <-- PONE TU ENDPOINT ACÁ
    mqtt_cfg.cert_pem = amazon_root_ca;

    mqtt_cfg.buffer_size = 4096;
    mqtt_cfg.out_buffer_size = 4096;

    // 3. Lógica de Ruteo (La Máquina de Estados)
    if (is_device_provisioned()) {
        ESP_LOGI(LOGTAG, "Modo Operativo: Leyendo credenciales únicas desde NVS...");

        char* cert = read_cert_from_nvs();
        char* key = read_private_key_from_nvs();

        if (cert != NULL && key != NULL) {
            mqtt_cfg.client_cert_pem = cert;
            mqtt_cfg.client_key_pem = key;
        } else {
            ESP_LOGE(LOGTAG, "Error critico al leer certs de NVS. ¿Corrupción de memoria?");
            // Tratar error o forzar re-aprovisionamiento
        }
    } else {
        ESP_LOGI(LOGTAG, "Arrancando con Claim Certificate (Modo Aprovisionamiento)");
        mqtt_cfg.client_cert_pem = claim_cert_pem;
        mqtt_cfg.client_key_pem = claim_private_key;
    }

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    // Usamos MQTT_EVENT_ANY que es del tipo correcto, en lugar de ESP_EVENT_ANY_ID
    esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);

    global_mqtt_client = client;
}