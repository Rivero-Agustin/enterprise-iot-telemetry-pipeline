# 🌐 Enterprise IoT Provisioning & Telemetry Pipeline
End-to-end Cloud Native IoT telemetry pipeline: ESP32 to AWS (JITP), orchestrated with Node.js, MongoDB, and Grafana via Docker.

![ESP32](https://img.shields.io/badge/ESP32-000000?style=for-the-badge&logo=espressif&logoColor=white)
![AWS](https://img.shields.io/badge/AWS-%23FF9900.svg?style=for-the-badge&logo=amazon-aws&logoColor=white)
![NodeJS](https://img.shields.io/badge/node.js-6DA55F?style=for-the-badge&logo=node.js&logoColor=white)
![MongoDB](https://img.shields.io/badge/MongoDB-%234ea94b.svg?style=for-the-badge&logo=mongodb&logoColor=white)
![Docker](https://img.shields.io/badge/docker-%230db7ed.svg?style=for-the-badge&logo=docker&logoColor=white)
![Grafana](https://img.shields.io/badge/grafana-%23F46800.svg?style=for-the-badge&logo=grafana&logoColor=white)

> **Note:** Sensitive data such as AWS endpoints, Wi-Fi credentials, and X.509 cryptographic certificates have been removed from this repository for security purposes. Please refer to the .env.example and config_example.h files to configure your own environment.

## 📋 Overview
An end-to-end Cloud Native architecture designed for Ultra-Wideband (UWB) sensor telemetry. This project bridges physical hardware at the edge with Serverless cloud infrastructure, demonstrating a complete data lifecycle from secure device provisioning to real-time observability.

### 🎥 Live Demonstration
![Demo of Grafana Dashboard]([LINK_AL_GIF_O_VIDEO_AQUI])
*(Real-time telemetry stream handled via custom cache-busting REST API)*

---

## 🏗️ Architecture & Data Flow

![Architecture Diagram]([LINK_A_TU_DIAGRAMA_AQUI_EJ_DRAWIO])

The pipeline is structured into four distinct layers:

1. **Edge & Security (Hardware):** 
   - **ESP32** capturing UWB sensor data.
   - Secure device registration via **Zero-Touch Provisioning (JITP)** on AWS IoT Core.
   - Hardware-level security: Persistent storage of X.509 cryptographic certificates and private keys in secure memory partitions (**NVS**).
2. **Cloud Ingestion (AWS Serverless):**
   - Asynchronous MQTT message routing using **AWS IoT Rules**.
   - Decoupling and message queuing via **AWS SQS** for reliable backend processing.
3. **Backend & Persistence:**
   - Containerized **Node.js** microservice acting as an SQS consumer.
   - Time-series data formatting (ordered by `-1` limits and timestamps) stored in **MongoDB**.
4. **Observability (Frontend):**
   - **Grafana** dashboard containerized alongside the backend.
   - Consumes data via a Custom JSON REST API with tailored `cache-busting` parameters (`?cb=${__to}`) to ensure zero-latency live data streaming.

---

## 🚀 Local Infrastructure (Docker Compose)

The backend and observability layers are fully containerized. You can spin up the local environment (Node.js API, MongoDB, and Grafana) using the provided `docker-compose.yml`.

### Prerequisites
* [Docker](https://docs.docker.com/get-docker/) & Docker Compose installed.

### Run the Stack

1. Clone this repository:
   ```bash
   git clone [https://github.com/](https://github.com/)[TU_USUARIO]/[TU_REPOSITORIO].git
   cd [TU_REPOSITORIO]

2. Start the services using Docker Compose:
   ```bash
   docker-compose up -d

3. Access the services:
   ```bash
   Grafana Dashboard: http://localhost:3000 (Default credentials: admin / admin)
   Node.js REST API: http://localhost:3001 (Or the port you mapped)
   MongoDB Instance: mongodb://localhost:27017

*Data persistence is configured via Docker volumes (/var/lib/grafana and /data/db) to ensure dashboard layouts and telemetry data survive container restarts.*

## 🛠️ Key Technical Highlights

- **Cryptographic Security:** Implemented the Principle of Least Privilege across the device lifecycle.
- **Microservices Orchestration:** Fully isolated backend components using Docker networks and volumes.
- **Real-time Observability:** Solved native dashboard latency by engineering a custom cache-busting API endpoint for Grafana.
