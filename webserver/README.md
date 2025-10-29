# Servidor e Cliente HTTP em C

## Descrição
O projeto se trata de um **servidor HTTP** e um **cliente HTTP** simples em **C**.

---

## Compilação

No terminal, dentro da pasta do projeto, digite:

gcc servidor_http.c -o meu_servidor
gcc cliente_http.c -o meu_navegador

---

## Execução

### Iniciar o servidor:

./meu_servidor site

Ele rodará na porta 5050 e servirá os arquivos da pasta `site`.

### Testar com o cliente:

./meu_navegador http://127.0.0.1:5050/index.html

O conteúdo recebido será salvo em `saida_recebida.txt`.

---

## Observações
- Usa **porta 5050**
- Funciona em **qualquer distribuição Linux**
- O servidor lista arquivos se `index.html` não existir

