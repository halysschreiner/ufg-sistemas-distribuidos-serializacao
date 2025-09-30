<?php

/**
 * Classe para comunicação com o servidor C via protocolo DSV.
 */
class ClienteDSV
{
    private string $host;
    private int $port;
    private int $timeout;

    public function __construct(string $host = '127.0.0.1', int $port = 8080, int $timeout = 10)
    {
        $this->host = $host;
        $this->port = $port;
        $this->timeout = $timeout;
    }

    /**
     * Envia uma requisição para inserir uma nova pessoa.
     * @param array $dados Os dados da pessoa (nome, cpf, email, idade)
     * @return array Resposta parseada do servidor
     */
    public function inserir(array $dados): array
    {
        $mensagem = sprintf(
            "INSERIR|nome:%s|cpf:%s|email:%s|idade:%d",
            $dados['nome'] ?? '',
            $dados['cpf'] ?? '',
            $dados['email'] ?? '',
            $dados['idade'] ?? 0
        );
        $respostaDSV = $this->enviarRequisicao($mensagem);
        return $this->parsearResposta($respostaDSV);
    }

    /**
     * Envia uma requisição para listar todas as pessoas.
     * @return array Resposta parseada do servidor
     */
    public function listar(): array
    {
        $respostaDSV = $this->enviarRequisicao("LISTAR");
        return $this->parsearResposta($respostaDSV);
    }

    /**
     * Abre a conexão, envia a mensagem e retorna a resposta do servidor.
     * @param string $mensagem Mensagem no formato DSV
     * @return string Resposta crua do servidor
     */
    private function enviarRequisicao(string $mensagem): string
    {
        // Criação do socket
        $socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
        if ($socket === false) {
            return "ERRO|mensagem:Nao foi possivel criar o socket";
        }

        // Conexão com o servidor
        $conexao = socket_connect($socket, $this->host, $this->port);
        if ($conexao === false) {
            return "ERRO|mensagem:Nao foi possivel conectar ao servidor C";
        }

        // Envio da mensagem
        socket_write($socket, $mensagem, strlen($mensagem));

        // Leitura da resposta
        $resposta = socket_read($socket, 4096);

        // Fechamento do socket
        socket_close($socket);

        return $resposta ?: "ERRO|mensagem:Servidor nao respondeu";
    }
    
    /**
     * Parseia a resposta DSV do servidor para um array PHP.
     * @param string $respostaDSV
     * @return array
     */
    private function parsearResposta(string $respostaDSV): array
    {
        $partes = explode('|', $respostaDSV);
        $comando = array_shift($partes);
        $resultado = ['comando' => $comando, 'dados' => []];

        switch ($comando) {
            case 'SUCESSO':
            case 'ERRO':
                foreach ($partes as $parte) {
                    list($chave, $valor) = explode(':', $parte, 2);
                    $resultado['dados'][$chave] = $valor;
                }
                break;

            case 'LISTA':
                $pessoas = [];
                foreach ($partes as $blocoPessoa) {
                    if (empty($blocoPessoa)) continue;
                    
                    $camposPessoa = explode('#', $blocoPessoa);
                    array_shift($camposPessoa); // Remove o "PESSOA"
                    $pessoa = [];
                    foreach($camposPessoa as $campo) {
                       list($chave, $valor) = explode(':', $campo, 2);
                       $pessoa[$chave] = $valor;
                    }
                    $pessoas[] = $pessoa;
                }
                $resultado['dados'] = $pessoas;
                break;
        }

        return $resultado;
    }
}
