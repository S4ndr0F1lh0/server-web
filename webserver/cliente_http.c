#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 4096

int main(int argc,char *argv[]){
    if(argc != 2){
        printf("Uso: %s http://<host>:<porta>/<arquivo>\n",argv[0]);
        return 1;
    }

    char host[128],caminho[256];
    int porta;

    if(sscanf(argv[1],"http://%127[^:]:%d/%255[^\n]",host,&porta,caminho) != 3){
        fprintf(stderr,"URL invalida. Exemplo: http://localhost:8080/arquivo.txt\n");
        return 1;
    }

    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0){perror("Erro ao criar socket");
        return 1;
    }

    struct sockaddr_in servidor;
    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(porta);
    servidor.sin_addr.s_addr = inet_addr(host);

    if(connect(sock,(struct sockaddr*)&servidor,sizeof(servidor)) < 0){
        perror("Erro ao conectar");
        close(sock);
        return 1;
    }

    char requisicao[512];
    sprintf(requisicao,"GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",caminho,host);
    send(sock,requisicao,strlen(requisicao),0);

    char buffer[BUFFER_SIZE];
    char resposta[BUFFER_SIZE*8] = "";
    int recebido,total = 0;
    int header_terminado = 0;
    FILE *saida = NULL;

    while((recebido = recv(sock,buffer,sizeof(buffer),0)) > 0){
        if(!header_terminado){
            memcpy(resposta + total,buffer,recebido);
            total += recebido;
            resposta[total] = '\0';

            char *corpo = strstr(resposta,"\r\n\r\n");
            if(corpo){
                corpo += 4;// pula o cabeçalho
                header_terminado = 1;

                // Verifica status HTTP
                if(strncmp(resposta, "HTTP/1.1 200", 12) != 0){
                    fprintf(stderr,"Erro do servidor:\n%.*s\n",recebido,resposta);
                    close(sock);
                    return 1;
                }

                // Cria arquivo de saída
                char *nome_arquivo = strrchr(caminho, '/');
                nome_arquivo = nome_arquivo ? nome_arquivo + 1 : caminho;
                if(strlen(nome_arquivo) == 0) nome_arquivo = "index.html";

                saida = fopen(nome_arquivo,"wb");
                if(!saida){
                    perror("Erro ao criar arquivo"); 
                    close(sock); 
                    return 1; 
                }

                // Escreve corpo já recebido
                int corpo_len = total - (corpo - resposta);
                fwrite(corpo,1,corpo_len,saida);
            }
        }else{
            fwrite(buffer,1,recebido,saida);
        }
    }

    if(saida){
        fclose(saida);
        printf("Arquivo salvo com sucesso!\n");
    }else{
        printf("Nenhum arquivo salvo(erro ou resposta vazia).\n");
    }
    close(sock);
    return 0;
}
