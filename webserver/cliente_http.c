#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 4096

int main(int argc,char *argv[]){
    if(argc != 2){
        printf("Uso:%s http://<host>:<porta>/<arquivo>\n",argv[0]);
        return 1;
    }

    char host[128],caminho[256];
    int porta;
    sscanf(argv[1],"http://%127[^:]:%d/%255[^\n]",host,&porta,caminho);

    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0){
        perror("Erro ao criar socket");
        return 1;
    }

    struct sockaddr_in servidor;
    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(porta);
    servidor.sin_addr.s_addr = inet_addr(host);

    if(connect(sock,(struct sockaddr*)&servidor,sizeof(servidor)) < 0){
        perror("Erro ao conectar");
        return 1;
    }

    char requisicao[512];
    sprintf(requisicao,"GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n",caminho,host);
    send(sock,requisicao,strlen(requisicao),0);

    FILE *saida = fopen("saida_recebida.txt","wb");
    char buffer[BUFFER_SIZE];
    int bytes;
    while((bytes = recv(sock,buffer,BUFFER_SIZE,0)) > 0){
        fwrite(buffer,1,bytes,saida);
    }

    printf("Arquivo salvo como saida_recebida.txt\n");

    fclose(saida);
    close(sock);
    return 0;
}
