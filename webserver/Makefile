all: meu_servidor meu_navegador

meu_servidor: servidor_http.c
	gcc servidor_http.c -o meu_servidor 

meu_navegador: cliente_http.c
	gcc cliente_http.c -o meu_navegador 

clean:
	rm -f meu_servidor meu_navegador
