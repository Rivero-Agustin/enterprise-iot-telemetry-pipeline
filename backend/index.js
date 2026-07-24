import {
  SQSClient,
  ReceiveMessageCommand,
  DeleteMessageCommand,
} from "@aws-sdk/client-sqs";
import { MongoClient } from "mongodb";
import dotenv from "dotenv";
import express from "express";

// Variables de entorno (para pruebas locales sin Docker)
dotenv.config();

// CONFIGURACIÓN DEL SERVIDOR HTTP (GRAFANA)
const app = express();
app.use(express.json());

// CONFIGURACIÓN AWS SQS
const sqsClient = new SQSClient({ region: process.env.AWS_REGION });
const queueUrl = process.env.QUEUE_URL;

// CONEXIÓN A MONGODB
const mongoUrl =
  process.env.MONGO_URI || "mongodb://host.docker.internal:27017";
const dbClient = new MongoClient(mongoUrl);

// ENDPOINT PARA GRAFANA
app.get("/api/telemetry", async (req, res) => {
  try {
    const db = dbClient.db("iot_dashboard");
    const collection = db.collection("telemetria");

    // Traemos los últimos 50 registros ordenados del más nuevo al más viejo
    const data = await collection
      .find({})
      .sort({ timestamp_procesamiento: -1 })
      .limit(50)
      .toArray();

    // Grafana JSON API espera un arreglo de objetos invertido para leerlos en orden cronologico
    res.json(data.reverse());
  } catch (error) {
    console.error("Error consultando MongoDB:", error);
    res.status(500).json({ error: "Error interno del servidor" });
  }
});

// Arrancamos el servidor HTTP en el puerto 8080
app.listen(8080, () => {
  console.log(
    "🚀 Servidor HTTP escuchando en el puerto 8080 (Listo para Grafana)",
  );
});

// BUCLE DE SQS
async function pollMessages() {
  await dbClient.connect();
  const db = dbClient.db("iot_dashboard");
  const collection = db.collection("telemetria");
  console.log("Conectado a MongoDB local. Iniciando sondeo en SQS...");
  while (true) {
    try {
      const receiveParams = {
        QueueUrl: queueUrl,
        MaxNumberOfMessages: 10,
        WaitTimeSeconds: 20,
      };
      const data = await sqsClient.send(
        new ReceiveMessageCommand(receiveParams),
      );

      if (data.Messages) {
        for (const message of data.Messages) {
          const payload = JSON.parse(message.Body);
          console.log(`\n📦 Recibido: ${payload.timestamp_procesamiento}`);

          // 1. GUARDAR EN LA BASE DE DATOS LOCAL
          await collection.insertOne(payload);
          console.log("💾 Guardado en MongoDB");

          // 2. BORRAR DE SQS AWS
          const deleteParams = {
            QueueUrl: queueUrl,
            ReceiptHandle: message.ReceiptHandle,
          };
          await sqsClient.send(new DeleteMessageCommand(deleteParams));
          console.log("✅ Eliminado de AWS SQS");
        }
      }
    } catch (error) {
      console.error("❌ Error consumiendo SQS:", error);
      await new Promise((resolve) => setTimeout(resolve, 5000));
    }
  }
}
// Arrancamos el bucle infinito
pollMessages();
