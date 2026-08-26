<div align="right">
  🌎 <a href="README.md">English</a> | 🇪🇸 <a href="README-es.md">Español</a>
</div>

# 🌐 Enterprise IoT Provisioning & Telemetry Pipeline

Pipeline de telemetría IoT Cloud Native de extremo a extremo: ESP32 a AWS (JITP), orquestado con Node.js, MongoDB y Grafana mediante Docker.
El firmware está diseñado teniendo en cuenta la modularidad, presentando tareas concurrentes para la medición de distancia UWB, aprovisionamiento/diagnóstico BLE y comunicación MQTT con AWS IoT, gestionado a través de RTOS.

![ESP32](https://img.shields.io/badge/ESP32-000000?style=for-the-badge&logo=espressif&logoColor=white)
![AWS](https://img.shields.io/badge/AWS-%23FF9900.svg?style=for-the-badge&logo=amazon-aws&logoColor=white)
![NodeJS](https://img.shields.io/badge/node.js-6DA55F?style=for-the-badge&logo=node.js&logoColor=white)
![MongoDB](https://img.shields.io/badge/MongoDB-%234ea94b.svg?style=for-the-badge&logo=mongodb&logoColor=white)
![Docker](https://img.shields.io/badge/docker-%230db7ed.svg?style=for-the-badge&logo=docker&logoColor=white)
![Grafana](https://img.shields.io/badge/grafana-%23F46800.svg?style=for-the-badge&logo=grafana&logoColor=white)

> **🔒 Nota de Seguridad:** Los datos sensibles como credenciales de AWS IAM, contraseñas de Wi-Fi y certificados criptográficos X.509 han sido eliminados de este repositorio. Por favor, consulte los archivos `.env.example` (backend) y `config.example.h` (firmware) para configurar su propio entorno.

## 📋 Descripción General

Una arquitectura Cloud Native de extremo a extremo diseñada para la telemetría de sensores Ultra-Wideband (UWB). Este proyecto conecta el hardware físico en el Edge con infraestructura cloud Serverless, demostrando un ciclo de vida completo de datos desde el aprovisionamiento seguro del dispositivo hasta la observabilidad en tiempo real.

### 🎥 Demostración en Vivo

![Demo del Dashboard de Grafana](./docs/demo.dashboard.grafana.gif)

**El Ciclo de Vida de los Datos en Acción:**

- 📍 **Edge (Inferior Izquierda):** Hardware ESP32 UWB Pro mostrando mediciones de distancia sin procesar localmente.
- ⚙️ **Backend (Inferior Derecha):** Microservicio Node.js contenedorizado consumiendo mensajes desde AWS SQS, persistiendo los datos en MongoDB y confirmando/eliminando los mensajes de la cola.
- 📊 **Observabilidad (Superior):** Dashboard de Grafana reflejando instantáneamente las variaciones de distancia física.

_(Flujo de telemetría en tiempo real gestionado mediante una API REST personalizada con cache-busting)_

---

## 🏗️ Arquitectura y Flujo de Datos

![Diagrama de Arquitectura](./docs/architecture.diagram.png)

El pipeline está estructurado en cuatro capas diferenciadas:

1. **Edge y Seguridad (Hardware):**
   - **ESP32** capturando datos de sensores UWB.
   - Registro seguro de dispositivos mediante **Zero-Touch Provisioning (JITP)** en AWS IoT Core.
   - Seguridad a nivel de hardware: Almacenamiento persistente de certificados criptográficos X.509 y claves privadas en particiones de memoria segura (**NVS**).
2. **Ingesta Cloud (AWS Serverless):**
   - Enrutamiento asíncrono de mensajes MQTT utilizando **AWS IoT Rules**.
   - Desacoplamiento y encolamiento de mensajes mediante **AWS SQS** para un procesamiento fiable en el backend.
3. **Backend y Persistencia:**
   - Microservicio **Node.js** contenedorizado actuando como consumidor de SQS.
   - Formateo de datos de series temporales (ordenados por límites `-1` y marcas de tiempo) almacenados en **MongoDB**.
4. **Observabilidad (Frontend):**
   - Dashboard de **Grafana** contenedorizado junto con el backend.
   - Consume datos mediante una API REST JSON personalizada con parámetros adaptados de `cache-busting` (`?cb=${__to}`) para garantizar la transmisión de datos en vivo sin latencia.

---

## 🗂️ Estructura del Monorepo

Este proyecto utiliza un enfoque monorepo para separar responsabilidades manteniendo todo el pipeline en un solo lugar:

- `/firmware`: Proyecto de PlatformIO que contiene el código C++ para el ESP32.
- `/backend`: Microservicio Node.js, aprovisionamiento de Grafana y configuraciones de Docker Compose.

---

## 🚀 Despliegue Local (Backend y Observabilidad)

Las capas de backend y observabilidad están completamente contenedorizadas. Puede iniciar el entorno local (API Node.js, MongoDB y Grafana) utilizando Docker.

### Requisitos Previos

- [Docker](https://docs.docker.com/get-docker/) y Docker Compose instalados.

### Instrucciones de Configuración

1. **Clonar este repositorio:**

   ```bash
   git clone https://github.com/Rivero-Agustin/esp32-iot-telemetry-pipeline.git
   cd esp32-iot-telemetry-pipeline
   ```

2. **Configurar las Variables de Entorno:**
   Navegue al directorio de backend y configure sus credenciales de AWS.

   ```bash
   cd backend
   cp .env.example .env
   # Edite el archivo .env con sus claves de AWS IAM y la URL de SQS.
   ```

3. **Iniciar los Microservicios:**

   ```bash
   docker-compose up -d --build
   ```

4. **Acceder a los Servicios:**

- Dashboard de Grafana: http://localhost:3000 (Predeterminado: admin / admin)
- API REST de Node.js: http://localhost:3001
- Instancia de MongoDB: mongodb://localhost:27017

_La persistencia de datos está configurada mediante volúmenes de Docker (/var/lib/grafana y /data/db) para garantizar que las configuraciones de los dashboards y los datos de telemetría persistan tras el reinicio de los contenedores._

## 🛠️ Aspectos Técnicos Destacados

- **Seguridad Criptográfica:** Implementación del Principio de Menor Privilegio a lo largo de todo el ciclo de vida del dispositivo.
- **Orquestación de Microservicios:** Componentes de backend completamente aislados utilizando redes y volúmenes de Docker.
- **Observabilidad en Tiempo Real:** Resolución de la latencia nativa del dashboard mediante la ingeniería de un endpoint API personalizado con cache-busting para Grafana.
