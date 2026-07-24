# Usamos la versión LTS ligera de Node
FROM node:20-alpine

# Directorio de trabajo dentro del contenedor
WORKDIR /app

# Copiamos solo los archivos de dependencias primero (optimiza la caché de Docker)
COPY package*.json ./

# Instalamos solo dependencias de producción
RUN npm install --omit=dev

# Copiamos el resto del código
COPY . .

# Exponemos el puerto que va a usar el endpoint de Grafana (ej: 3000)
EXPOSE 8080

# Comando para arrancar el servicio
CMD ["node", "index.js"]