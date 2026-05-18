#!/bin/bash
set -e

echo "=== Actualizando sistema ==="
apt-get update -qq

echo "=== Instalando Docker ==="
apt-get install -y ca-certificates curl gnupg
install -m 0755 -d /etc/apt/keyrings
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | gpg --dearmor -o /etc/apt/keyrings/docker.gpg
chmod a+r /etc/apt/keyrings/docker.gpg
echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable" > /etc/apt/sources.list.d/docker.list
apt-get update -qq
apt-get install -y docker-ce docker-ce-cli containerd.io docker-compose-plugin

echo "=== Verificando Docker ==="
docker --version
docker compose version

echo "=== Clonando repositorio ==="
cd /opt
if [ -d "poo-final" ]; then
    echo "Repo ya existe, haciendo pull..."
    cd poo-final && git pull
else
    git clone https://github.com/felipetosolini/poo-final.git
    cd poo-final
fi

echo "=== Creando .env de produccion ==="
cat > backend/.env << 'EOF'
DB_URL=mysql+pymysql://chessuser:chesspassword@db:3306/chessdb
JWT_SECRET=CAMBIAR_POR_UN_SECRET_SEGURO_EN_PRODUCCION
OPENAI_API_KEY=PONER_API_KEY_REAL_AQUI
EOF
echo "IMPORTANTE: edita /opt/poo-final/backend/.env con los valores reales"

echo "=== Levantando contenedores ==="
docker compose up -d --build

echo "=== Esperando que la API levante ==="
sleep 10
curl -f http://localhost:8000/health && echo "" && echo "API OK"

echo ""
echo "Setup completo. API disponible en http://89.116.29.15:8000/health"
