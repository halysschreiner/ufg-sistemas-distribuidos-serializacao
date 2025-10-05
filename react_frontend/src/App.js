import React, { useState, useEffect, useCallback } from 'react';
import axios from 'axios';
import './App.css';

// URL da sua API PHP
const API_URL = 'http://localhost:8000/api.php';

function App() {
  const [pessoas, setPessoas] = useState([]);
  const [feedback, setFeedback] = useState({ mensagem: '', tipo: '' });
  
  // Função para buscar a lista de pessoas da API
  const fetchPessoas = useCallback(async () => {
    try {
      const response = await axios.get(API_URL);
      setPessoas(response.data);
    } catch (error) {
      console.error("Erro ao buscar pessoas:", error);
      setFeedback({ mensagem: 'Erro ao conectar com o servidor.', tipo: 'erro' });
    }
  }, []);

  // useEffect para buscar os dados iniciais quando o componente montar
  useEffect(() => {
    fetchPessoas();
  }, [fetchPessoas]);

  // Função para lidar com o cadastro de uma nova pessoa
  const handleCadastro = async (novaPessoa) => {
    try {
      const response = await axios.post(API_URL, novaPessoa);
      setFeedback({ mensagem: response.data.dados.mensagem, tipo: 'sucesso' });
      fetchPessoas(); // Atualiza a lista após o cadastro
    } catch (error) {
      const erroMsg = error.response?.data?.dados?.mensagem || 'Erro desconhecido ao cadastrar.';
      setFeedback({ mensagem: erroMsg, tipo: 'erro' });
    }
  };

  return (
    <div className="container">
      <header>
        <h1>Sistema de Cadastro de Pessoas</h1>
      </header>
      <main>
        <div className="form-container">
          <h2>Cadastrar Nova Pessoa</h2>
          <CadastroForm onCadastro={handleCadastro} />
          {feedback.mensagem && (
            <div className={`feedback ${feedback.tipo}`}>
              {feedback.mensagem}
            </div>
          )}
        </div>
        <div className="list-container">
          <h2>Lista de Pessoas Cadastradas</h2>
          <PessoasList pessoas={pessoas} />
        </div>
      </main>
    </div>
  );
}

// Componente do Formulário de Cadastro
function CadastroForm({ onCadastro }) {
  const [nome, setNome] = useState('');
  const [cpf, setCpf] = useState('');
  const [email, setEmail] = useState('');
  const [idade, setIdade] = useState('');

  const handleSubmit = (e) => {
    e.preventDefault();
    if (!nome || !cpf || !email || !idade) {
        alert('Por favor, preencha todos os campos.');
        return;
    }
    onCadastro({ nome, cpf, email, idade: parseInt(idade) });
    // Limpar formulário
    setNome('');
    setCpf('');
    setEmail('');
    setIdade('');
  };

  return (
    <form onSubmit={handleSubmit}>
      <label>Nome Completo:</label>
      <input type="text" value={nome} onChange={(e) => setNome(e.target.value)} required />
      
      <label>CPF:</label>
      <input type="text" value={cpf} onChange={(e) => setCpf(e.target.value)} required />
      
      <label>E-mail:</label>
      <input type="email" value={email} onChange={(e) => setEmail(e.target.value)} required />
      
      <label>Idade:</label>
      <input type="number" value={idade} onChange={(e) => setIdade(e.target.value)} min="1" max="150" required />
      
      <button type="submit">Cadastrar Pessoa</button>
    </form>
  );
}

// Componente da Tabela de Listagem
function PessoasList({ pessoas }) {
  if (!pessoas || pessoas.length === 0) {
    return <p>Nenhuma pessoa cadastrada.</p>;
  }

  return (
    <table>
      <thead>
        <tr>
          <th>ID</th>
          <th>Nome</th>
          <th>CPF</th>
          <th>Email</th>
          <th>Idade</th>
        </tr>
      </thead>
      <tbody>
        {pessoas.map((pessoa) => (
          <tr key={pessoa.id}>
            <td>{pessoa.id}</td>
            <td>{pessoa.nome}</td>
            <td>{pessoa.cpf}</td>
            <td>{pessoa.email}</td>
            <td>{pessoa.idade}</td>
          </tr>
        ))}
      </tbody>
    </table>
  );
}

export default App;
