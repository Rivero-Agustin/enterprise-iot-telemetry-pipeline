<div align="right">
  🌎 <a href="README-en.md">English</a> | 🇪🇸 <a href="README.md">Español</a>
</div>

# 🌐 Enterprise IoT Provisioning & Telemetry Pipeline

Pipeline de telemetría IoT Cloud Native de extremo a extremo: ESP32 a AWS (JITP), orquestado tanto en Docker como en Kubernetes nativo con ArgoCD (GitOps), con infraestructura en la nube automatizada mediante Terraform (IaC) y CI/CD en GitHub Actions.
El firmware está diseñado teniendo en cuenta la modularidad, presentando tareas concurrentes para la medición de distancia UWB, aprovisionamiento/diagnóstico BLE y comunicación MQTT con AWS IoT, gestionado a través de RTOS.

![ESP32](https://img.shields.io/badge/ESP32-000000?style=for-the-badge&logo=espressif&logoColor=white)
![AWS](https://img.shields.io/badge/AWS-%23FF9900.svg?style=for-the-badge&logo=amazon-aws&logoColor=white)
![NodeJS](https://img.shields.io/badge/node.js-6DA55F?style=for-the-badge&logo=node.js&logoColor=white)
![MongoDB](https://img.shields.io/badge/MongoDB-%234ea94b.svg?style=for-the-badge&logo=mongodb&logoColor=white)
![Docker](https://img.shields.io/badge/docker-%230db7ed.svg?style=for-the-badge&logo=docker&logoColor=white)
![Grafana](https://img.shields.io/badge/grafana-%23F46800.svg?style=for-the-badge&logo=grafana&logoColor=white)
![Terraform](https://img.shields.io/badge/terraform-%235835CC.svg?style=for-the-badge&logo=terraform&logoColor=white)
![GitHub Actions](https://img.shields.io/badge/github%20actions-%232671E8.svg?style=for-the-badge&logo=githubactions&logoColor=white)
![Kubernetes](https://img.shields.io/badge/kubernetes-%23326ce5.svg?style=for-the-badge&logo=kubernetes&logoColor=white)
![ArgoCD](https://img.shields.io/badge/argo%20cd-%23EF7B4D.svg?style=for-the-badge&logo=argo&logoColor=white)

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

El pipeline está estructurado en cinco capas diferenciadas:

1. **Edge y Seguridad (Hardware):**
   - **ESP32** capturando datos de sensores UWB.
   - Registro seguro de dispositivos mediante **Zero-Touch Provisioning (JITP)** en AWS IoT Core.
   - Seguridad a nivel de hardware: Almacenamiento persistente de certificados criptográficos X.509 y claves privadas en particiones de memoria segura (**NVS**).
2. **Ingesta Cloud (AWS Serverless):**
   - Enrutamiento asíncrono de mensajes MQTT utilizando **AWS IoT Rules**.
   - Desacoplamiento y encolamiento de mensajes mediante **AWS SQS** para un procesamiento fiable en el backend.
   - Resiliencia y tolerancia a fallos: **Dead Letter Queue (DLQ)** con política de reenvío automático.
3. **Backend y Persistencia:**
   - Despliegue dual: soporte para desarrollo local con **Docker Compose** o arquitectura Cloud-Native en **Kubernetes**.
   - Microservicio **Node.js** actuando como consumidor de SQS mediante long-polling con sondas _Liveness_ y _Readiness_.
   - Persistencia de series temporales en **MongoDB** estructurado como `StatefulSet` en Kubernetes con volúmenes persistentes (`PVC`).
4. **Observabilidad (Frontend):**
   - Dashboard de **Grafana** contenedorizado junto con el backend (accesible vía `NodePort` en Kubernetes o puerto 3000 en Docker).
   - Consume datos mediante una API REST JSON personalizada con parámetros adaptados de `cache-busting` (`?cb=${__to}`) para garantizar la transmisión de datos en vivo sin latencia.
5. **Infraestructura como Código (IaC) & GitOps:**
   - Despliegue declarativo de infraestructura en AWS con **Terraform** y backend remoto seguro (**AWS S3** + **DynamoDB State Locking**).
   - Pipeline CI/CD en **GitHub Actions**: validación y `terraform plan` predictivo en Pull Requests, y `terraform apply` automático al fusionar a `main`.
   - Continuous Delivery / GitOps con **ArgoCD**: orquestación declarativa de microservicios con **Kustomize** sincronizados en tiempo real desde GitHub con _Self-Healing_.

---

## 🗂️ Estructura del Monorepo

Este proyecto utiliza un enfoque monorepo para separar responsabilidades manteniendo todo el pipeline en un solo lugar:

- `/.github`: Automatización CI/CD con GitHub Actions para validación y despliegue GitOps de Terraform.
- `/firmware`: Proyecto de PlatformIO que contiene el código C++ para el ESP32.
- `/backend`: Microservicio Node.js, aprovisionamiento de Grafana y configuraciones de Docker Compose.
- `/k8s`: Manifiestos de Kubernetes nativo estructurados con Kustomize (Deployments, StatefulSets, Probes) listos para orquestación GitOps con ArgoCD.
- `/terraform`: Infraestructura como Código (IaC) para aprovisionar colas SQS, Dead Letter Queues (DLQ), reglas de AWS IoT Core y políticas IAM con Principio de Menor Privilegio.

---

## ☁️ Despliegue de Infraestructura Cloud (Terraform)

Toda la infraestructura requerida en AWS se despliega automáticamente en segundos utilizando Terraform:

1. **Requisitos Previos:**
   - [Terraform](https://developer.hashicorp.com/terraform/install) (>= 1.5.0).
   - Credenciales de AWS configuradas en el entorno (`AWS_ACCESS_KEY_ID`, `AWS_SECRET_ACCESS_KEY`, `AWS_REGION`).

2. **Inicializar y Desplegar:**

   ```bash
   cd terraform
   terraform init
   terraform plan
   terraform apply
   ```

3. **Conectar con Backend y Firmware:**
   Terraform generará automáticamente los endpoints y URLs necesarios:
   - `sqs_queue_url` ➔ Copiar en `QUEUE_URL` dentro de `backend/.env`.
   - `iot_endpoint` ➔ Copiar en `ENDPOINT` dentro de `firmware/include/config.h`.

4. **Destruir Recursos (Ahorro de Costos):**
   Para eliminar todos los recursos de AWS y evitar costos cuando no esté en uso:

   ```bash
   terraform destroy
   ```

---

## ☸️ Opción 1: Despliegue Cloud-Native & GitOps (Kubernetes + ArgoCD) [Recomendado]

Para entornos escalables y de producción, la arquitectura se orquesta de forma declarativa con **Kubernetes** y **Kustomize**, y se sincroniza automáticamente mediante **ArgoCD**:

### 1. Despliegue Automático con ArgoCD (GitOps)

Si ya tiene ArgoCD instalado en su clúster de Kubernetes, simplemente aplique el manifiesto de la aplicación:

```bash
kubectl apply -f k8s/argocd/application.yaml
```

ArgoCD leerá continuamente este repositorio, desplegará los microservicios en el namespace `iot-pipeline` y mantendrá el clúster sincronizado de forma autónoma.

- **Acceso a la UI de ArgoCD:** `https://localhost:8085` (o vía `kubectl port-forward svc/argocd-server -n argocd 8085:443`).

### 2. Despliegue Manual con Kustomize (Sin ArgoCD)

```bash
# 1. Aplicar la configuración base en el clúster
kubectl apply -k ./k8s/base

# 2. Verificar el estado de pods y servicios
kubectl get pods,svc,pvc -n iot-pipeline

# 3. Acceder a Grafana en el clúster
kubectl port-forward svc/grafana-service -n iot-pipeline 3000:3000
```

---

## 🚀 Opción 2: Despliegue Rápido Local (Docker Compose) [Ligero]

Para pruebas de desarrollo local en un solo comando sin requerir un clúster de Kubernetes activo:

### Requisitos Previos

- [Docker](https://docs.docker.com/get-docker/) y Docker Compose instalados.

### Instrucciones de Configuración

1. **Configurar las Variables de Entorno:**

   ```bash
   cd backend
   cp .env.example .env
   # Edite el archivo .env con sus claves de AWS IAM y la URL de SQS.
   ```

2. **Iniciar los Microservicios:**

   ```bash
   docker-compose up -d --build
   ```

3. **Acceder a los Servicios:**
   - Dashboard de Grafana: http://localhost:3000 (Predeterminado: admin / admin)
   - API REST de Node.js: http://localhost:8080
   - Instancia de MongoDB: mongodb://localhost:27017

_La persistencia de datos está configurada mediante volúmenes de Docker (/var/lib/grafana y /data/db) para garantizar que las configuraciones de los dashboards y los datos de telemetría persistan tras el reinicio de los contenedores._

---

## 🛠️ Aspectos Técnicos Destacados

- **Orquestación Cloud-Native & GitOps (Kubernetes & ArgoCD):** Arquitectura declarativa con Kustomize, base de datos persistente mediante `StatefulSet`, sondas de resiliencia (`liveness/readiness probes`) y reconciliación continua automatizada con ArgoCD (_Self-Healing_).
- **Infraestructura como Código (IaC) & GitOps:** Despliegue cloud 100% automatizado mediante Terraform en HCL, backend remoto protegido en AWS S3 con DynamoDB state locking, y pipeline CI/CD en GitHub Actions con planes predictivos en PRs.
- **Resiliencia y Confiabilidad:** Desacoplamiento asíncrono con AWS SQS y Dead Letter Queue (DLQ) con política de reintentos para aislar errores sin interrupciones.
- **Seguridad Criptográfica:** Implementación del Principio de Menor Privilegio a lo largo de todo el ciclo de vida del dispositivo y roles IAM acotados.
- **Orquestación de Microservicios:** Componentes de backend completamente aislados utilizando redes y volúmenes de Docker o Kubernetes.
- **Observabilidad en Tiempo Real:** Resolución de la latencia nativa del dashboard mediante la ingeniería de un endpoint API personalizado con cache-busting para Grafana.
