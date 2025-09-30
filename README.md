# Sistema de Cadastro de Pessoas (Cliente/Servidor com DSV)

Este projeto é uma implementação completa de um sistema de cadastro de pessoas, demonstrando a comunicação entre um frontend moderno em React, um backend em PHP e um servidor de alta performance em C. A comunicação entre o PHP e o C é realizada através de um protocolo personalizado chamado **DSV (Delimited Structured Values)**.

## Arquitetura do Sistema

O sistema é dividido em três componentes principais que operam de forma desacoplada:

1. **Frontend (React):** Uma interface de usuário reativa e moderna construída com React. O usuário interage com esta interface para adicionar e visualizar pessoas. Ela se comunica com o Backend PHP via requisições HTTP (JSON).
    
2. **Backend (PHP API):** Um servidor PHP que atua como uma ponte (API). Ele recebe as requisições HTTP do frontend, traduz os dados para o protocolo DSV e os envia para o Servidor C via Sockets TCP.
    
3. **Servidor (C):** O núcleo do sistema, responsável pela lógica de negócio e persistência dos dados. Escrito em C para máxima performance, ele utiliza multi-threading para lidar com múltiplas conexões simultâneas e armazena os dados em um banco de dados SQLite.
    

```
┌───────────────┐      HTTP (JSON)      ┌──────────────┐      TCP (DSV)       ┌──────────────┐
│ Frontend      │◄─────────────────────►│ Backend      │◄────────────────────►│ Servidor     │
│ (React)       │                       │ (PHP API)    │                      │ (C)          │
└───────────────┘                       └──────────────┘                      └──────────────┘
                                                                                      │
                                                                                      ▼
                                                                                ┌──────────┐
                                                                                │ SQLite   │
                                                                                │ (Banco)  │
                                                                                └──────────┘
```

---

## Como Executar

Existem duas maneiras de executar este projeto: utilizando Docker (recomendado para simplicidade) ou configurando cada serviço manualmente.

### Método 1: Execução com Docker (Recomendado)

Este método utiliza Docker e Docker Compose para construir uma imagem e executar um contêiner com todos os serviços e dependências já configurados.

#### Pré-requisitos

- [Docker](https://www.docker.com/get-started)
    
- [Docker Compose](https://docs.docker.com/compose/install/)
    

#### Passo 1: Estrutura de Arquivos

Organize seu projeto na seguinte estrutura. Isso é crucial para que o Docker consiga encontrar e copiar os arquivos corretamente.

```
/projeto-cadastro/
├── c_server/
│   └── servidor.c
├── php_api/
│   ├── api.php
│   └── ClienteDSV.php
├── react_frontend/
│   ├── public/
│   │   └── index.html
│   └── src/
│       ├── App.css
│       └── App.js
├── docker-compose.yaml
├── Dockerfile
├── package.json
└── supervisord.conf
```

> **Nota:** Certifique-se de que todos os arquivos de configuração (`Dockerfile`, `docker-compose.yaml`, `supervisord.conf`, `package.json`, etc.) estejam preenchidos conforme as instruções anteriores.

#### Passo 2: Subir o Contêiner

Abra um terminal na raiz do projeto (`/projeto-cadastro/`) e execute o seguinte comando:

Bash

```
docker-compose up --build
```

O processo pode demorar alguns minutos na primeira vez, pois o Docker irá baixar as imagens base e instalar todas as dependências. Ao final, você verá os logs dos três serviços (C, PHP e React) sendo exibidos no seu terminal.

#### Passo 3: Acessar a Aplicação

Com o contêiner em execução, abra seu navegador e acesse: **`http://localhost:3000`**

#### Passo 4: Encerrar a Aplicação

Para parar o contêiner, volte ao terminal e pressione `Ctrl + C`. Para remover o contêiner e a rede criada, execute:

Bash

```
docker-compose down
```

---

### Método 2: Execução Local (Manual)

Este método requer a instalação manual de todas as dependências e a execução de cada serviço em um terminal separado.

#### Pré-requisitos

Bash

```
# Para o Servidor C (Debian/Ubuntu)
sudo apt-get update
sudo apt-get install gcc sqlite3 libsqlite3-dev

# Para o Backend PHP (Debian/Ubuntu)
sudo apt-get install php-cli php-sockets

# Para o Frontend React
sudo apt-get install nodejs npm
```

#### Passo 1: Iniciar o Servidor C

1. Navegue até a pasta `c_server`.
    
2. Compile o código:
    
    Bash
    
    ```
    gcc -o servidor servidor.c -lsqlite3 -lpthread -Wall
    ```
    
3. Execute o servidor (mantenha este terminal aberto):
    
    Bash
    
    ```
    ./servidor
    ```
    

#### Passo 2: Iniciar o Backend PHP

1. Abra um **novo terminal** e navegue até a pasta `php_api`.
    
2. Inicie o servidor web do PHP (mantenha este terminal aberto):
    
    Bash
    
    ```
    php -S localhost:8000
    ```
    

#### Passo 3: Iniciar o Frontend React

1. Abra um **terceiro terminal** e navegue até a pasta `react_frontend`.
    
2. Instale as dependências (apenas na primeira vez):
    
    Bash
    
    ```
    npm install
    ```
    
3. Inicie a aplicação React (mantenha este terminal aberto):
    
    Bash
    
    ```
    npm start
    ```
    

#### Passo 4: Acessar a Aplicação

Seu navegador deverá abrir automaticamente em `http://localhost:3000`. Caso contrário, acesse o endereço manualmente.
