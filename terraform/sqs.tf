# Dead Letter Queue (DLQ) para almacenar mensajes que fallaron en procesamiento
resource "aws_sqs_queue" "telemetry_dlq" {
  name                      = "${var.sqs_queue_name}-dlq"
  message_retention_seconds = 1209600 # 14 días (máximo permitido)

  tags = {
    Name = "${var.sqs_queue_name}-dlq"
    Type = "DeadLetterQueue"
  }
}

# Cola principal de telemetría consumida por el backend Node.js
resource "aws_sqs_queue" "telemetry_queue" {
  name                       = var.sqs_queue_name
  delay_seconds              = 0
  max_message_size           = 262144 # 256 KB
  message_retention_seconds  = 86400  # 1 día de retención
  receive_wait_time_seconds  = 20     # Long Polling (optimiza costos y latencia del SDK de Node.js)
  visibility_timeout_seconds = 30     # Tiempo para procesar el mensaje antes de reintentar

  redrive_policy = jsonencode({
    deadLetterTargetArn = aws_sqs_queue.telemetry_dlq.arn
    maxReceiveCount     = 3
  })

  tags = {
    Name = var.sqs_queue_name
    Type = "PrimaryQueue"
  }
}

