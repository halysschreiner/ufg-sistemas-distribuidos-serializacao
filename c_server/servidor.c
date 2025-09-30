#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sqlite3.h>
#include <ctype.h>

#define PORTA 8080
#define BUFFER_SIZE 4096
#define DB_FILE "/app/data/pessoas.db"

// Estrutura para os dados da pessoa
typedef struct {
    char nome[256];
    char cpf[256];
    char email[256];
    int idade;
} Pessoa;

// Protótipos
void *handle_client(void *socket_desc);
void inicializar_banco();
void processar_requisicao(char *requisicao, char *resposta, int client_sock);
int inserir_pessoa_db(Pessoa p);
void listar_pessoas_db(char *resposta);
void formatar_cpf(const char *cpf_original, char *cpf_formatado);

// Função principal
int main() {
    int server_fd, client_sock;
    struct sockaddr_in server, client;
    socklen_t client_len = sizeof(struct sockaddr_in);

    inicializar_banco();

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Erro ao criar socket");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORTA);

    if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Erro de bind");
        return 1;
    }

    listen(server_fd, 10);

    printf("Banco de dados inicializado\n");
    printf("Servidor rodando na porta %d\n", PORTA);
    printf("Protocolo: DSV (Delimited Structured Values)\n");
    printf("Aguardando conexões...\n\n");

    while ((client_sock = accept(server_fd, (struct sockaddr *)&client, &client_len))) {
        printf("Cliente conectado\n");
        
        pthread_t sniffer_thread;
        int *new_sock = malloc(sizeof(int));
        *new_sock = client_sock;

        if (pthread_create(&sniffer_thread, NULL, handle_client, (void *)new_sock) < 0) {
            perror("Erro ao criar thread");
            return 1;
        }
    }

    if (client_sock < 0) {
        perror("Erro ao aceitar conexão");
        return 1;
    }

    close(server_fd);
    return 0;
}

// Handler para cada cliente (executado em uma thread)
void *handle_client(void *socket_desc) {
    int sock = *(int *)socket_desc;
    free(socket_desc);
    
    char buffer_cliente[BUFFER_SIZE];
    char resposta[BUFFER_SIZE * 2];
    int read_size;

    if ((read_size = recv(sock, buffer_cliente, BUFFER_SIZE, 0)) > 0) {
        buffer_cliente[read_size] = '\0';
        printf("Requisição: %s\n", buffer_cliente);
        
        processar_requisicao(buffer_cliente, resposta, sock);

        write(sock, resposta, strlen(resposta));
        printf("Resposta: %s\n\n", resposta);
    }

    if (read_size == 0) {
        printf("Cliente desconectado\n");
        fflush(stdout);
    } else if (read_size == -1) {
        perror("Erro no recv");
    }

    close(sock);
    return 0;
}

/**
 * NOVA FUNÇÃO: Formata o CPF para o padrão XXX.XXX.XXX-XX
 */
void formatar_cpf(const char *cpf_original, char *cpf_formatado) {
    char cpf_limpo[12] = {0};
    int j = 0;
    // Limpa o CPF, mantendo apenas os dígitos
    for (int i = 0; cpf_original[i] != '\0' && j < 11; i++) {
        if (isdigit(cpf_original[i])) {
            cpf_limpo[j++] = cpf_original[i];
        }
    }
    cpf_limpo[j] = '\0';

    // Se o CPF limpo tiver 11 dígitos, formata. Caso contrário, usa o original.
    if (strlen(cpf_limpo) == 11) {
        sprintf(cpf_formatado, "%.3s.%.3s.%.3s-%.2s", 
                cpf_limpo, 
                cpf_limpo + 3, 
                cpf_limpo + 6, 
                cpf_limpo + 9);
    } else {
        strncpy(cpf_formatado, cpf_original, 255);
    }
}

/**
 * FUNÇÃO CORRIGIDA: Usa strtok_r para evitar o bug de aninhamento.
 */
void processar_requisicao(char *requisicao, char *resposta, int client_sock) {
    char *saveptr1, *saveptr2;
    char *comando = strtok_r(requisicao, "|", &saveptr1);
    
    if (comando == NULL) {
        sprintf(resposta, "ERRO|mensagem:Comando invalido");
        return;
    }
    printf("Comando recebido: %s\n", comando);

    if (strcmp(comando, "INSERIR") == 0) {
        Pessoa p = {0};
        char *campo;
        while ((campo = strtok_r(NULL, "|", &saveptr1)) != NULL) {
            char *chave = strtok_r(campo, ":", &saveptr2);
            char *valor = strtok_r(NULL, "", &saveptr2); // Pega todo o resto como valor

            if (chave && valor) {
                if (strcmp(chave, "nome") == 0) strncpy(p.nome, valor, sizeof(p.nome) - 1);
                else if (strcmp(chave, "cpf") == 0) {
                    // Chama a nova função de formatação aqui
                    formatar_cpf(valor, p.cpf);
                }
                else if (strcmp(chave, "email") == 0) strncpy(p.email, valor, sizeof(p.email) - 1);
                else if (strcmp(chave, "idade") == 0) p.idade = atoi(valor);
            }
        }

        int novo_id = inserir_pessoa_db(p);
        if (novo_id > 0) {
            sprintf(resposta, "SUCESSO|mensagem:Pessoa inserida com sucesso|id:%d", novo_id);
        } else {
            sprintf(resposta, "ERRO|mensagem:CPF duplicado ou dados invalidos");
        }
    }
    else if (strcmp(comando, "LISTAR") == 0) {
        listar_pessoas_db(resposta);
    }
    else {
        sprintf(resposta, "ERRO|mensagem:Comando desconhecido");
    }
}

void inicializar_banco() {
    sqlite3 *db;
    char *err_msg = 0;
    int rc = sqlite3_open(DB_FILE, &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Nao foi possivel abrir o banco: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    const char *sql = "CREATE TABLE IF NOT EXISTS pessoas ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "nome TEXT NOT NULL, "
                      "cpf TEXT UNIQUE NOT NULL, "
                      "email TEXT, "
                      "idade INTEGER, "
                      "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        exit(1);
    }
    
    sqlite3_close(db);
}

int inserir_pessoa_db(Pessoa p) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc = sqlite3_open(DB_FILE, &db);
    if (rc != SQLITE_OK) return -1;

    const char *sql = "INSERT INTO pessoas (nome, cpf, email, idade) VALUES (?, ?, ?, ?);";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, p.nome, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, p.cpf, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, p.email, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, p.idade);
    } else {
        fprintf(stderr, "Falha ao preparar statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    int novo_id = -1;
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        novo_id = sqlite3_last_insert_rowid(db);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return novo_id;
}

void listar_pessoas_db(char *resposta) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc = sqlite3_open(DB_FILE, &db);

    if (rc != SQLITE_OK) {
        sprintf(resposta, "ERRO|mensagem:Nao foi possivel conectar ao banco");
        return;
    }

    const char *sql = "SELECT id, nome, cpf, email, idade FROM pessoas ORDER BY id;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        sprintf(resposta, "ERRO|mensagem:Erro na consulta ao banco");
        sqlite3_close(db);
        return;
    }

    strcpy(resposta, "LISTA");
    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
        char buffer_pessoa[512];
        sprintf(buffer_pessoa, "|PESSOA#id:%d#nome:%s#cpf:%s#email:%s#idade:%d",
                sqlite3_column_int(stmt, 0),
                sqlite3_column_text(stmt, 1),
                sqlite3_column_text(stmt, 2),
                sqlite3_column_text(stmt, 3),
                sqlite3_column_int(stmt, 4));
        strcat(resposta, buffer_pessoa);
    }
    
    if (count == 0) {
        strcpy(resposta, "LISTA");
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}
