#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <dirent.h>

#define PORTA 8080
#define BUFFER_SIZE 4096

void enviar_arquivo(int cliente, const char *caminho){
    FILE *arquivo = fopen(caminho,"rb");
    if(!arquivo){
        const char *erro = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nArquivo não encontrado.\n";
        send(cliente,erro,strlen(erro),0);
        return;
    }

    fseek(arquivo,0,SEEK_END);
    long tamanho = ftell(arquivo);
    rewind(arquivo);

    char cabecalho[256];
    snprintf(cabecalho,sizeof(cabecalho),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: application/octet-stream\r\n"
             "Content-Length: %ld\r\n\r\n",tamanho);
    send(cliente,cabecalho,strlen(cabecalho),0);

    char buffer[BUFFER_SIZE];
    size_t bytes;
    while((bytes = fread(buffer,1,BUFFER_SIZE,arquivo)) > 0)
        send(cliente,buffer,bytes,0);

    fclose(arquivo);
}

void listar_diretorio(int cliente,const char *diretorio,const char *url){
    DIR *dir = opendir(diretorio);
    if(!dir){
        const char *erro = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nDiretório não encontrado.\n";
        send(cliente,erro,strlen(erro),0);
        return;
    }

    char resposta[BUFFER_SIZE*8];
    int usado = snprintf(resposta,sizeof(resposta),
        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
        "<html><body><h2>Arquivos disponiveis:</h2><ul>");

    struct dirent *entrada;
    while((entrada = readdir(dir)) != NULL){
        if(entrada->d_name[0] == '.')continue;

        usado += snprintf(resposta + usado, sizeof(resposta) - usado,
                          "<li><a href=\"%s/%s\">%s</a></li>",
                          url,entrada->d_name,entrada->d_name);

        if(usado >= (int)sizeof(resposta) - 512)
            break;
    }
    closedir(dir);

    usado += snprintf(resposta + usado,sizeof(resposta) - usado,
                      "</ul></body></html>");

    send(cliente,resposta,usado,0);
}

void tratar_requisicao(int cliente,const char *raiz,const char *recurso){
    char caminho_completo[512];
    snprintf(caminho_completo,sizeof(caminho_completo),"%s%s",raiz,recurso);

    struct stat st;
    if(stat(caminho_completo, &st) < 0){
        const char *erro = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nRecurso não encontrado.\n";
        send(cliente,erro,strlen(erro),0);
        return;
    }

    if(S_ISDIR(st.st_mode)){
        char index_path[1024];
        snprintf(index_path,sizeof(index_path),"%s/index.html",caminho_completo);
        if(stat(index_path, &st) == 0)
            enviar_arquivo(cliente,index_path);
        else
            listar_diretorio(cliente,caminho_completo,recurso);
    } else {
        enviar_arquivo(cliente,caminho_completo);
    }
}

int main(int argc,char *argv[]) {
    if(argc != 2){
        printf("Uso: %s <diretorio_base>\n",argv[0]);
        return 1;
    }

    const char *diretorio_base = argv[1];

    int servidor = socket(AF_INET,SOCK_STREAM,0);
    if(servidor < 0){
        perror("Erro ao criar socket");
        return 1;
    }

    // ⚙️ Permite reutilizar a porta (evita “Address already in use”)
    int opt = 1;
    setsockopt(servidor,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    struct sockaddr_in endereco;
    endereco.sin_family = AF_INET;
    endereco.sin_addr.s_addr = INADDR_ANY;
    endereco.sin_port = htons(PORTA);

    if(bind(servidor,(struct sockaddr *)&endereco,sizeof(endereco)) < 0){
        perror("Erro no bind");
        close(servidor);
        return 1;
    }

    if (listen(servidor, 10) < 0){
        perror("Erro no listen");
        close(servidor);
        return 1;
    }

    printf("Servidor rodando em http://localhost:%d servindo %s\n",PORTA,diretorio_base);

    while(1){
        struct sockaddr_in cliente_addr;
        socklen_t cliente_len = sizeof(cliente_addr);
        int cliente = accept(servidor,(struct sockaddr*)&cliente_addr,&cliente_len);
        if(cliente < 0)continue;

        char buffer[BUFFER_SIZE];
        memset(buffer,0,sizeof(buffer));
        recv(cliente,buffer,sizeof(buffer) - 1,0);

        char metodo[8],recurso[256];
        sscanf(buffer,"%s %s",metodo,recurso);

        if(strcmp(metodo,"GET") == 0){
            if(strcmp(recurso,"/") == 0)
                tratar_requisicao(cliente,diretorio_base,"/");
            else
                tratar_requisicao(cliente,diretorio_base,recurso);
        }else{
            const char *erro = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
            send(cliente,erro,strlen(erro),0);
        }

        close(cliente);
    }

    close(servidor);
    return 0;
}
