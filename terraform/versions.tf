terraform {
  required_version = ">= 1.5.0"

  # Backend Remoto: Guarda el tfstate en S3 y usa DynamoDB para State Locking
  backend "s3" {
    bucket         = "esp32-iot-pipeline-tfstate-395103361070"
    key            = "iot-pipeline/terraform.tfstate"
    region         = "us-east-1"
    dynamodb_table = "esp32-iot-pipeline-tfstate-locks"
    encrypt        = true
  }

  required_providers {
    aws = {
      source  = "hashicorp/aws"
      version = "~> 5.0"
    }
  }
}

provider "aws" {
  region = var.aws_region

  default_tags {
    tags = {
      Project     = var.project_name
      Environment = var.environment
      ManagedBy   = "Terraform"
    }
  }
}
