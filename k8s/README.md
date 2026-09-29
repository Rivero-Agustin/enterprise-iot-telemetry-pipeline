# ☸️ Kubernetes Native Architecture (Kustomize & GitOps)

Manifiestos declarativos de Kubernetes diseñados bajo estándares CNCF para orquestar la ingesta, persistencia y observabilidad del **Enterprise IoT Telemetry Pipeline**.

Esta arquitectura reemplaza el entorno local de Docker Compose por un despliegue nativo de microservicios en Kubernetes, completamente preparado para ser gestionado de forma continua por **ArgoCD**.

---

## 🏗️ Componentes del Clúster

| Componente           | Tipo de Recurso                             | Función y Justificación Técnica                                                                                                                                                     |
| :------------------- | :------------------------------------------ | :---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Namespace**        | `Namespace` (`iot-pipeline`)                | Aislamiento lógico de recursos y seguridad multi-tenant.                                                                                                                            |
| **MongoDB**          | `StatefulSet` + `Service` (Headless)        | Persistencia garantizada con `volumeClaimTemplates` (5Gi), identidad de red fija (`mongodb-service:27017`) y arranque secuencial.                                                   |
| **Backend Consumer** | `Deployment` + `Service` (ClusterIP)        | Microservicio Node.js consumidor de AWS SQS con **Liveness & Readiness Probes** (`/api/telemetry`), límites estrictos de CPU/RAM y desacoplamiento mediante `ConfigMap` y `Secret`. |
| **Grafana**          | `Deployment` + `Service` (NodePort `30080`) | Panel de observabilidad en tiempo real con plugin JSON preinstalado y volumen persistente (`grafana-storage`).                                                                      |

---

## 🗂️ Estructura de Kustomize

```text
k8s/
├── base/                              # Declaración común agnóstica de entorno
│   ├── kustomization.yaml
│   ├── namespace.yaml
│   ├── mongodb/
│   │   ├── statefulset.yaml
│   │   └── service.yaml
│   ├── backend/
│   │   ├── deployment.yaml
│   │   ├── service.yaml
│   │   ├── configmap.yaml
│   │   └── secret.example.yaml
│   └── grafana/
│       ├── deployment.yaml
│       ├── service.yaml
│       └── pvc.yaml
└── overlays/                          # Variaciones por entorno
    └── dev/                           # Entorno de desarrollo local (prefijo dev-)
        └── kustomization.yaml
```

---

## 🚀 Despliegue Manual (Sin ArgoCD)

### 1. Configurar Secretos Sensibles

Cree su archivo de secretos a partir de la plantilla:

```bash
cp k8s/base/backend/secret.example.yaml k8s/base/backend/secret.yaml
# Edite secret.yaml con sus credenciales reales de AWS IAM y la URL de SQS
```

### 2. Validar Manifiestos (Dry-Run con Kustomize)

```bash
kubectl kustomize ./k8s/base
```

### 3. Aplicar al Clúster

```bash
kubectl apply -k ./k8s/base
```

### 4. Verificar Estado de los Pods y Servicios

```bash
kubectl get pods,svc,pvc -n iot-pipeline
```

### 5. Acceder a Grafana

- **En Minikube / K3d / Docker Desktop**:
  Abra [http://localhost:30080](http://localhost:30080) (o use `kubectl port-forward svc/grafana-service 3000:3000 -n iot-pipeline`).
  Credenciales por defecto: `admin` / `admin`.
