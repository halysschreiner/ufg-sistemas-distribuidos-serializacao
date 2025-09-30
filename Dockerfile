# Estágio 1: Base com todas as dependências
FROM ubuntu:22.04

# Evita prompts interativos durante a instalação
ENV DEBIAN_FRONTEND=noninteractive

# Instala dependências
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    gcc make libsqlite3-dev sqlite3 \
    php-cli php-sockets \
    curl ca-certificates \
    supervisor \
    && rm -rf /var/lib/apt/lists/*

# Instala o Node.js v18 usando nvm
ENV NVM_DIR /root/.nvm
ENV NODE_VERSION 18.17.1
RUN mkdir -p $NVM_DIR && \
    curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.1/install.sh | bash && \
    . $NVM_DIR/nvm.sh && nvm install $NODE_VERSION && nvm alias default $NODE_VERSION && nvm use default

# Adiciona o Node.js ao PATH
ENV NODE_PATH $NVM_DIR/v$NODE_VERSION/lib/node_modules
ENV PATH $NVM_DIR/versions/node/v$NODE_VERSION/bin:$PATH

# Define o diretório de trabalho
WORKDIR /app

# Copia os códigos-fonte para o contêiner
COPY c_server/ ./c_server/
COPY php_api/ ./php_api/
COPY react_frontend/ ./react_frontend/

# Instala as dependências do React
WORKDIR /app/react_frontend
RUN npm install

# Retorna ao diretório principal e copia a configuração do Supervisor
WORKDIR /app
COPY supervisord.conf /etc/supervisor/conf.d/supervisord.conf

# Expõe as portas
EXPOSE 3000 8000
