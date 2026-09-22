variable "aws_region" {
  description = "AWS region for deploying resources"
  type        = string
  default     = "us-east-1"
}

variable "project_name" {
  description = "Base project name used in tags and resource naming"
  type        = string
  default     = "enterprise-iot-telemetry-pipeline"
}

variable "environment" {
  description = "Deployment environment (e.g. dev, staging, prod)"
  type        = string
  default     = "dev"
}

variable "sqs_queue_name" {
  description = "Name of the primary SQS telemetry queue"
  type        = string
  default     = "esp32-telemetria-queue"
}

variable "mqtt_topic_filter" {
  description = "MQTT topic pattern that AWS IoT Core rule listens to"
  type        = string
  default     = "esp32/datos/+"
}

variable "iot_endpoint_address" {
  description = "AWS IoT Core ATS endpoint address"
  type        = string
  default     = "a22o96apcs8yee-ats.iot.us-east-1.amazonaws.com"
}

