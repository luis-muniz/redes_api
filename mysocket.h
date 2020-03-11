#ifndef _UTIL_H_
#define _UTIL_H_

/* Bibliotecas */
#include <netinet/in.h> // socket() - AF_INET - SOCK_STREAM - IPPROTO_TCP
#include <string.h>
#include <arpa/inet.h> //inet_pton()

/* Auxiliares*/
#define TRUE 1
#define FALSE 0
#define LIMITE_REQUESTS 10  // será usado no listen do REP
#define LIMITE_SUBSCRIBES 2 // será usado no tamanho do vetor de subscribes do PUB

/*Types*/
#define REP 1 
#define REQ 2
#define PUB 3
#define SUB 4

/* Opções de estados do REPLY e REQUEST */
enum Estados
{
    RECV = 0,
    SEND,
};

/* Struct que manterá informações dos tipos: REP, REQ, PUB e SUB */
struct Estrutura
{
    /* Usado por todos */
    int protocolo;
    int type;
    int sock;

    /*  Usadao apenas para REP e REQ */
    int estadoAtual; // manteŕa o estado (RECV ou SEND)

    /* Usado Apenas para REP */
    int sockRequestComunicacao; // será usado para guardar o socket retorno da função accept() para comunicação com o request

    /* Usado apenas para PUB */
    int listaSockSubscribe[LIMITE_SUBSCRIBES]; // guardará todos os socks (subscribes) que de se conectarem com o Publish
    unsigned int totalSubscribes;   // variavel para saber quantos estão conectados
};

/* ------ Funções da API ------- */

/*
    # Retorna um endereço de um struct Estrutura, e dependendo do type passado, apenas os atributos necessários serão preenchidos.
    # encerra a aplicação se não for passado um type esperado
*/
void *my_socket(int type);

/*
    As demais funcões da API, sempre terão o seguinte retorno:
    # 0  - em caso de sucesso
    # -1 - em caso de falha
*/
int my_bind(void *estrutura, const char *protocolo, const char *ip, const char *porta);

int my_recv(void *estrutura, void *msg, int msgTam, int flags);

int my_send(void *estrutura, void *msg, int msgTam, int flags);

int my_connect(void *estrutura, const char *protocolo, const char *ip, const char *porta);

int my_close(void *estrutura);

/* -------- Funcoes Auxiliares ------- */

// recebe duas strings para identifcar o erro e depois encerra a aplicação
void error(char *erro, char *mensagem);

// thread background que será usado pelo Publisher para ficar aceitando Subscribes
void *aceitarSubscribes(void *estrutura);

// inicializa a lista de subscribers como -1 (vaga disponível)
void inicializarListaDeSubscribers(void * estrutura);

void imprimirInformacoes(void *estrutura);

#endif