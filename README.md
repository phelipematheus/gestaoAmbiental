# Projeto de Gerenciamento de Resíduos Industriais

## Visão Geral
Este projeto é uma aplicação gráfica desenvolvida em C com GTK+ para gerenciamento de resíduos industriais, cadastro de empresas, geração de relatórios, e manipulação de dados relacionados ao processamento de resíduos. Além disso, possui funcionalidades de segurança com criptografia usando OpenSSL, embora seu uso não seja detalhado no trecho fornecido.

---

## Funcionalidades principais

### Cadastro de Usuários
- **Validação de login**: valida usuários através de um arquivo `usuarios.txt`.
- **Cadastro de novos usuários**: permite cadastrar novos usuários com armazenamento no mesmo arquivo.

### Gerenciamento de Empresas
- Dispara operações de leitura e escrita em arquivo `dados.txt`.
- Possui funções para inserir, deletar e carregar dados de empresas, incluindo informações como CNPJ, nome, endereço e estado.

### Resíduos
- Manipulação de resíduos industriais em arquivo `dados_residuos.txt`.
- Inserção, deleção e visualização de resíduos relacionados às empresas.
- Agrupamento de resíduos por região do Brasil.

### Relatórios
- Geração de relatórios globais em formatos CSV ou TXT.
- Relatórios detalhados por anos, incluindo despesas, quantidade de resíduos, divisão por semestre, e por mês.
- Geração de relatórios por CNPJ de empresas, com integração de dados de resíduos.

### Interface Gráfica (GTK+)
- Utiliza Glade para criar a interface (`login.glade`).
- Navegação entre diferentes "views" usando uma `GtkStack`.
- Botões para ações como login, cadastro, geração de relatórios, e manipulação de registros.

### Manipulação de Arquivos
- Abre, lê, escreve, deleta e renomeia arquivos essenciais ao funcionamento da aplicação.
- Implementa funcionalidade de geração de nomes de arquivo incrementais para evitar sobreescrita.

### Segurança e Criptografia
- Cabeçalhos do OpenSSL indicam uso potencial de AES ou operações de geração de chaves, mas detalhes não estão no trecho fornecido.

---

## Estruturas de Dados
- **Empresa**: armazena CNPJ, nome, custo total, quantidade de resíduos.
- **AnoRelatorio**: dados anuais, totais semestrais, custos mensais.
- **Constantes e Limites**: limites máximos definidos para empresas, anos, linhas, etc.

---

## Arquivos utilizados
- `usuarios.txt` - grava e valida usuários.
- `dados.txt` - dados de empresas/pessoas.
- `dados_residuos.txt` - detalhes de resíduos processados.
- Arquivos gerados dinamicamente na pasta "Downloads" no formato CSV ou TXT.

---

## Como usar a aplicação

### Início
- Execute o programa. A janela inicial de login será exibida.
- Faça login ou cadastre-se.

### Navegação
- Após login, navegue pelo menu principal:
  - Cadastro de empresas/dados.
  - Registro de resíduos industriais.
  - Geração de relatórios (CSV ou TXT).
  - Manutenção de registros (adicionar, deletar).

### Geração de Relatórios
- Clique nos botões destinados à geração do relatório global, seja em CSV ou TXT.
- Os relatórios são salvos na pasta de Downloads com nomes incrementais automática.

### Manutenção de Registros
- Selecione uma empresa na lista, delete ou edite.
- Insira novos registros através dos formulários específicos.

---

## Como compilar
- Garanta que o GTK+ e OpenSSL estejam instalados no seu sistema.
- Compile este projeto com um comando semelhante ao:
```bash
gcc -o meu_app main.c `pkg-config --cflags --libs gtk4 openssl` -lssl -lcrypto
