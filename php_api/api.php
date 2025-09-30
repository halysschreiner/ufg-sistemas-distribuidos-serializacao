<?php
// Permitir requisições de qualquer origem (CORS) - Apenas para desenvolvimento
header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Headers: *");
header("Access-Control-Allow-Methods: GET, POST, OPTIONS");
header("Content-Type: application/json; charset=UTF-8");

// Responde a requisições OPTIONS (pre-flight)
if ($_SERVER['REQUEST_METHOD'] == 'OPTIONS') {
    exit(0);
}

require_once 'ClienteDSV.php';

// --- Ponto de Entrada da API ---

$metodo = $_SERVER['REQUEST_METHOD'];
$clienteDSV = new ClienteDSV('127.0.0.1', 8080);

try {
    if ($metodo === 'GET') {
        // Rota para LISTAR pessoas
        $resultado = $clienteDSV->listar();
        http_response_code(200);
        echo json_encode($resultado['dados']);

    } elseif ($metodo === 'POST') {
        // Rota para INSERIR pessoa
        $corpoRequisicao = json_decode(file_get_contents('php://input'), true);
        
        if (json_last_error() !== JSON_ERROR_NONE) {
            throw new Exception("JSON invalido");
        }
        
        $resultado = $clienteDSV->inserir($corpoRequisicao);

        if ($resultado['comando'] === 'SUCESSO') {
            http_response_code(201); // 201 Created
        } else {
            http_response_code(400); // 400 Bad Request
        }
        echo json_encode($resultado);

    } else {
        http_response_code(405); // Method Not Allowed
        echo json_encode(['erro' => 'Metodo nao permitido']);
    }

} catch (Exception $e) {
    http_response_code(500); // Internal Server Error
    echo json_encode(['erro' => $e->getMessage()]);
}
