# Regla de AWS IoT Core para enrutar mensajes MQTT a AWS SQS
resource "aws_iot_topic_rule" "esp32_telemetry_rule" {
  name        = "esp32_telemetry_to_sqs"
  description = "Enruta telemetria de sensores UWB desde MQTT hacia SQS inyectando timestamp de procesamiento"
  enabled     = true
  sql_version = "2016-03-23"
  sql         = "SELECT *, timestamp() AS timestamp_procesamiento FROM '${var.mqtt_topic_filter}'"

  sqs {
    queue_url  = aws_sqs_queue.telemetry_queue.id
    role_arn   = aws_iam_role.iot_to_sqs_role.arn
    use_base64 = false
  }

  error_action {
    sqs {
      queue_url  = aws_sqs_queue.telemetry_dlq.id
      role_arn   = aws_iam_role.iot_to_sqs_role.arn
      use_base64 = false
    }
  }
}

