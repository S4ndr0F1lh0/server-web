#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#define PORTA 5050
#define BUFFER_SIZE 4096

void enviar_arquivo(int cliente,const char *caminho){
    FILE *arquivo = fopen(caminho,"rb");
    if (!arquivo){
        char *erro = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nArquivo não encontrado.\n";
        send(cliente,erro,strlen(erro),0);
        return;
    }

    char cabecalho[256];
    fseek(arquivo,0,SEEK_END);
    long tamanho = ftell(arquivo);
    rewind(arquivo);

    sprintf(cabecalho,"HTTP/1.1 200 OK\r\nContent-Length:%ld\r\n\r\n",tamanho);
    send(cliente,cabecalho,strlen(cabecalho),0);

    char buffer[BUFFER_SIZE];
    size_t bytes;
    while((bytes = fread(buffer,1,BUFFER_SIZE,arquivo)) > 0){
        send(cliente,buffer,bytes,0);
    }

    fclose(arquivo);
}

void listar_diretorio(int cliente,const char *diretorio){
    DIR *dir = opendir(diretorio);
    if(!dir){
        char *erro = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nDiretório não encontrado.\n";
        send(cliente,erro,strlen(erro),0);
        return;
    }

    char resposta[BUFFER_SIZE*2] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h2>Arquivos disponíveis:</h2><ul>";
    struct dirent *entrada;
    while((entrada = readdir(dir)) != NULL){
        if(entrada->d_name[0] != '.'){
            strcat(resposta,"<li><a href=\"");
            strcat(resposta,entrada->d_name);
            strcat(resposta,"\">");
            strcat(resposta,entrada->d_name);
            strcat(resposta,"</a></li>");
        }
    }
    closedir(dir);
    strcat(resposta,"</ul></body></html>");
    send(cliente,resposta,strlen(resposta),0);
}

int main(int argc,char *argv[]){
    if(argc < 2){
        printf("Uso: %s <diretorio>\n",argv[0]);
        return 1;
    }

    const char *diretorio_base = argv[1];
    int servidor = socket(AF_INET, SOCK_STREAM, 0);
    if(servidor < 0){
        perror("Erro ao criar socket");
        return 1;
    }

    struct sockaddr_in endereco;
    endereco.sin_family = AF_INET;
    endereco.sin_addr.s_addr = INADDR_ANY;
    endereco.sin_port = htons(PORTA);

    if(bind(servidor,(struct sockaddr*)&endereco,sizeof(endereco)) < 0){
        perror("Erro no bind");
        return 1;
    }

    listen(servidor,5);
    printf("Servidor rodando na porta %d, servindo %s\n",PORTA,diretorio_base);

    while (1){
        struct sockaddr_in cliente_addr;
        socklen_t cliente_len = sizeof(cliente_addr);
        int cliente = accept(servidor,(struct sockaddr*)&cliente_addr,&cliente_len);
        if(cliente < 0)continue;

        char buffer[BUFFER_SIZE];
        recv(cliente,buffer,BUFFER_SIZE-1,0);

        char metodo[8],caminho[256];
        sscanf(buffer,"%s %s",metodo,caminho);

        char caminho_completo[512];
        snprintf(caminho_completo,sizeof(caminho_completo),"%s%s",diretorio_base,
                 strcmp(caminho,"/") == 0 ? "/index.html" : caminho);

        struct stat st;
        if(stat(caminho_completo, &st) == 0){
            if(S_ISDIR(st.st_mode))listar_diretorio(cliente, caminho_completo);
            else enviar_arquivo(cliente, caminho_completo);
        }else{
            char *erro = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nRecurso não encontrado.\n";
            send(cliente,erro,strlen(erro),0);
        }

        close(cliente);
    }

    close(servidor);
    return 0;
}
