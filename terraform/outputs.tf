output "sqs_queue_url" {
  description = "URL de la cola SQS principal (copiar a QUEUE_URL en backend/.env)"
  value       = aws_sqs_queue.telemetry_queue.id
}

output "sqs_queue_arn" {
  description = "ARN de la cola SQS principal"
  value       = aws_sqs_queue.telemetry_queue.arn
}

output "sqs_dlq_url" {
  description = "URL de la Dead Letter Queue (DLQ)"
  value       = aws_sqs_queue.telemetry_dlq.id
}

output "iot_endpoint" {
  description = "Endpoint ATS de AWS IoT Core (copiar a ENDPOINT en firmware/include/config.h)"
  value       = "mqtts://${var.iot_endpoint_address}:8883"
}

output "iot_topic_rule_name" {
  description = "Nombre de la regla creada en AWS IoT Core"
  value       = aws_iot_topic_rule.esp32_telemetry_rule.name
}

output "iot_policy_name" {
  description = "Nombre de la politica para certificados de dispositivos ESP32"
  value       = aws_iot_policy.esp32_device_policy.name
}

