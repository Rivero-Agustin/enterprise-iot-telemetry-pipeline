# Rol IAM asumido por el motor de reglas de AWS IoT Core
resource "aws_iam_role" "iot_to_sqs_role" {
  name = "${var.project_name}-iot-to-sqs-${var.environment}"

  assume_role_policy = jsonencode({
    Version = "2012-10-17"
    Statement = [
      {
        Effect = "Allow"
        Principal = {
          Service = "iot.amazonaws.com"
        }
        Action = "sts:AssumeRole"
      }
    ]
  })

  tags = {
    Name = "${var.project_name}-iot-to-sqs-role"
  }
}

# Política IAM estricta: Permite a AWS IoT enviar mensajes únicamente a nuestras colas SQS
resource "aws_iam_policy" "iot_to_sqs_policy" {
  name        = "${var.project_name}-iot-to-sqs-policy-${var.environment}"
  description = "Permite al motor de reglas de AWS IoT Core encolar telemetria en SQS"

  policy = jsonencode({
    Version = "2012-10-17"
    Statement = [
      {
        Effect = "Allow"
        Action = "sqs:SendMessage"
        Resource = [
          aws_sqs_queue.telemetry_queue.arn,
          aws_sqs_queue.telemetry_dlq.arn
        ]
      }
    ]
  })
}

resource "aws_iam_role_policy_attachment" "iot_to_sqs_attach" {
  role       = aws_iam_role.iot_to_sqs_role.name
  policy_arn = aws_iam_policy.iot_to_sqs_policy.arn
}

# Política IoT Core para dispositivos ESP32 (Principio de Menor Privilegio)
resource "aws_iot_policy" "esp32_device_policy" {
  name = "ESP32_Dev_Policy"

  policy = jsonencode({
    Version = "2012-10-17"
    Statement = [
      {
        Effect   = "Allow"
        Action   = "iot:Connect"
        Resource = "arn:aws:iot:${var.aws_region}:*:client/*"
      },
      {
        Effect = "Allow"
        Action = "iot:Publish"
        Resource = [
          "arn:aws:iot:${var.aws_region}:*:topic/esp32/datos/*",
          "arn:aws:iot:${var.aws_region}:*:topic/$aws/certificates/create/*",
          "arn:aws:iot:${var.aws_region}:*:topic/$aws/provisioning-templates/*"
        ]
      },
      {
        Effect = "Allow"
        Action = [
          "iot:Subscribe",
          "iot:Receive"
        ]
        Resource = [
          "arn:aws:iot:${var.aws_region}:*:topicfilter/$aws/certificates/create/*",
          "arn:aws:iot:${var.aws_region}:*:topicfilter/$aws/provisioning-templates/*",
          "arn:aws:iot:${var.aws_region}:*:topic/$aws/certificates/create/*",
          "arn:aws:iot:${var.aws_region}:*:topic/$aws/provisioning-templates/*"
        ]
      }
    ]
  })
}

