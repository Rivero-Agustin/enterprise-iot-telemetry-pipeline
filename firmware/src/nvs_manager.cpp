#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <cstdlib>

static const char *LOGTAG = "NVS_MANAGER";

// Función auxiliar genérica para extraer strings de la NVS reservando memoria
char* read_string_from_nvs(const char* key) {
    nvs_handle_t my_handle;
    
    // 1. Abrir el namespace "storage" en modo lectura
    esp_err_t err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(LOGTAG, "Error al abrir la NVS (%s)", esp_err_to_name(err));
        return NULL;
    }

    // 2. Consultar el tamaño exacto necesario en bytes (incluyendo '\0')
    size_t required_size = 0;
    err = nvs_get_str(my_handle, key, NULL, &required_size);
    if (err != ESP_OK || required_size == 0) {
        ESP_LOGE(LOGTAG, "La clave '%s' no existe en NVS o está vacía", key);
        nvs_close(my_handle);
        return NULL;
    }

    // 3. Asignar memoria dinámica en Heap para almacenar el texto largo del certificado
    char* buffer = (char*) malloc(required_size);
    if (buffer == NULL) {
        ESP_LOGE(LOGTAG, "Memoria RAM insuficiente para la clave '%s'", key);
        nvs_close(my_handle);
        return NULL;
    }

    // 4. Copiar el certificado desde la NVS al buffer
    err = nvs_get_str(my_handle, key, buffer, &required_size);
    nvs_close(my_handle);

    if (err != ESP_OK) {
        ESP_LOGE(LOGTAG, "Error leyendo la clave '%s'", key);
        free(buffer);
        return NULL;
    }

    return buffer;
}

// Wrapper para el certificado del dispositivo
char* read_cert_from_nvs(void) {
    return read_string_from_nvs("device_cert");
}

// Wrapper para la clave privada del dispositivo
char* read_private_key_from_nvs(void) {
    return read_string_from_nvs("priv_key");
}

// 1. Inicializar la partición NVS
void init_nvs() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

bool is_device_provisioned() {
    nvs_handle_t my_handle;
    if (nvs_open("storage", NVS_READONLY, &my_handle) != ESP_OK) return false;

    size_t cert_size = 0;
    size_t key_size = 0;
    
    // Verificamos que existan ambos
    esp_err_t err_cert = nvs_get_str(my_handle, "device_cert", NULL, &cert_size);
    esp_err_t err_key = nvs_get_str(my_handle, "priv_key", NULL, &key_size); // Nombre corto
    
    nvs_close(my_handle);

    return (err_cert == ESP_OK && cert_size > 0 && err_key == ESP_OK && key_size > 0);
}

// 3. Función para guardar los certificados nuevos que nos mande AWS
void save_device_credentials(const char* cert, const char* private_key) {
    nvs_handle_t my_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &my_handle));

    ESP_ERROR_CHECK(nvs_set_str(my_handle, "device_cert", cert));
    ESP_ERROR_CHECK(nvs_set_str(my_handle, "priv_key", private_key));
    
    ESP_ERROR_CHECK(nvs_commit(my_handle)); // Obligatorio para escribir físicamente
    nvs_close(my_handle);
    
    ESP_LOGI(LOGTAG, "Credenciales definitivas guardadas en NVS.");
}