# ==============================================================================
# RECURSOS PARA EL ESTADO REMOTO (S3 + DYNAMODB STATE LOCKING)
# ==============================================================================

# Bucket S3 para almacenar el archivo de estado de Terraform
resource "aws_s3_bucket" "terraform_state" {
  bucket        = "esp32-iot-pipeline-tfstate-395103361070"
  force_destroy = false

  tags = {
    Name        = "esp32-iot-pipeline-tfstate-395103361070"
    Description = "Almacenamiento seguro del estado de Terraform"
  }
}

# Versionado habilitado para recuperar estados anteriores ante incidentes
resource "aws_s3_bucket_versioning" "terraform_state_versioning" {
  bucket = aws_s3_bucket.terraform_state.id

  versioning_configuration {
    status = "Enabled"
  }
}

# Cifrado en reposo del estado con AES-256
resource "aws_s3_bucket_server_side_encryption_configuration" "terraform_state_crypto" {
  bucket = aws_s3_bucket.terraform_state.id

  rule {
    apply_server_side_encryption_by_default {
      sse_algorithm = "AES256"
    }
  }
}

# Bloqueo total de acceso público al bucket de estado
resource "aws_s3_bucket_public_access_block" "terraform_state_block_public" {
  bucket = aws_s3_bucket.terraform_state.id

  block_public_acls       = true
  block_public_policy     = true
  ignore_public_acls      = true
  restrict_public_buckets = true
}

# Tabla DynamoDB para el bloqueo de estado (evita ejecuciones simultáneas de terraform apply)
resource "aws_dynamodb_table" "terraform_locks" {
  name         = "esp32-iot-pipeline-tfstate-locks"
  billing_mode = "PAY_PER_REQUEST"
  hash_key     = "LockID"

  attribute {
    name = "LockID"
    type = "S"
  }

  tags = {
    Name        = "esp32-iot-pipeline-tfstate-locks"
    Description = "Control de concurrencia y bloqueo de estado para Terraform"
  }
}

