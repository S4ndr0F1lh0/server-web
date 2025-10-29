## Descrevendo o programa
O projeto se trata de um servidor HTTP e um cliente HTTP simples em C.

## Compilando

No terminal, dentro da pasta do projeto, digite:

gcc servidor_http.c -o meu_servidor
gcc cliente_http.c -o meu_navegador

### Iniciar o servidor:

./meu_servidor site

Ele rodará na porta 8080 e servirá os arquivos da pasta site.
Se houver um index.html, o servidor retorna essa página, senão retorna uma lista de arquivos pertencentes à pasta site.

### Testar o cliente:

./meu_navegador http://127.0.0.1:8080/index.html
./meu_navegador http://127.0.0.1:8080/teste.txt
./meu_navegador http://127.0.0.1:8080/bandeiras.png

O conteúdo recebido será salvo.
Se tiver um nome de arquivo ou formato diferente dos existentes, retorna erro no terminal.

