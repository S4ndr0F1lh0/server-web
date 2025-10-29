all: meu_servidor meu_navegador

meu_servidor: servidor_http.c
	gcc servidor_http.c -o meu_servidor 

meu_navegador: cliente_http.c
	gcc cliente_http.c -o meu_navegador 

rodar_servidor: servidor_http.c
	./meu_servidor site

buscar_arquivo: cliente_http.c
	./meu_navegador http://127.0.0.1:8080/{nome_arquivo}

clean:
	rm -f meu_servidor meu_navegador
