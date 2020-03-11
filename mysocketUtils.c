#include "mysocket.h" // API
#include <stdio.h>    // I/O
#include <stdlib.h>   // exit() - FAILURE
#include <pthread.h>

/* Lista de Estruturas */
unsigned int tamanho = 0;              // guardará o tamamho atual da lista
struct Estrutura listaDeEstrutura[50]; // Define uma lista de estrutras que poderá ser utilzada por 50 estruturas diferentes

/* Funções da API */
void *my_socket(int type)
{

    /* ------REPLY------ */
    if (type == REP)
    {
        listaDeEstrutura[tamanho].sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // crio o socket
        listaDeEstrutura[tamanho].type = REP;                                       // defino o tipo como REP
        listaDeEstrutura[tamanho].sockRequestComunicacao = -1;                      // defino o socket para comunicação com o request como -1
        listaDeEstrutura[tamanho].estadoAtual = RECV;                               // estado default do REP quando é criado
    }
    /* ------REQUEST------ */
    else if (type == REQ)
    {
        listaDeEstrutura[tamanho].sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // crio o socket
        listaDeEstrutura[tamanho].type = REQ;                                       // defino o tipo como REQ
        listaDeEstrutura[tamanho].estadoAtual = SEND;                               // estado default do REQ quando é criado
    }
    /* ------PUBLISH------ */
    else if (type == PUB)
    {
        listaDeEstrutura[tamanho].sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // crio o socket
        listaDeEstrutura[tamanho].type = PUB;                                       // defino o tipo como PUB
    }
    /* ------SUBSCRIBE------ */
    else if (type == SUB)
    {
        listaDeEstrutura[tamanho].sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // crio o socket
        listaDeEstrutura[tamanho].type = SUB;                                       // defino o tipo como SUB
    }
    /* ------NENHUM DOS TIPOS ANTERIORES------ */
    else
    {
        error("my_socket()", "Use um desses tipos: <REP> <REQ> <PUB> <SUB>"); // Encerra a aplicação se os tipos não forem os esperados
    }

    return &listaDeEstrutura[tamanho++]; // Retorna o endereço da estrutura corresponde com o tipo passado
                                         // Incrementa o tamanho total da lista de estruturas
}

/*OBS: # O accept() será realizado aqui através de uma thread se o type for PUBLISHER
       # Vários clientes simultaneamente através de thread
*/
int my_bind(void *estrutura, const char *protocolo, const char *ip, const char *porta)
{
    /* Verifica se todos os parametros foram passados*/
    if (estrutura == NULL)
    {
        error("<void *estrutura>", "O parametro está nulo"); // encerra a aplicação
        return -1;
    }

    int type = ((struct Estrutura *)estrutura)->type; // guarda o type da estrutura numa variável

    /* REPLY */
    if (type == REP)
    {
        uint16_t replyPort = atoi(porta); // converte a porta passado para inteiro
        uint32_t replyIP = atoi(ip);      // coverte o ip passado para inteiro

        struct sockaddr_in replyEndereco;                  // Endereço local
        memset(&replyEndereco, 0, sizeof(replyEndereco));  // Limpa a struct
        replyEndereco.sin_family = AF_INET;                // Família de endereço IPv4
        replyEndereco.sin_addr.s_addr = htonl(INADDR_ANY); // De qualquer interface
        replyEndereco.sin_port = htons(replyPort);         // Porta local

        // executa o bind
        int value = bind(((struct Estrutura *)estrutura)->sock, (struct sockaddr *)&replyEndereco, sizeof(replyEndereco));
        if (value < 0)
        {
            error("bind()", "A função retornou um erro ao ser executada"); // encerra a aplicação
            return -1;
        }

        // executa o listen
        value = listen(((struct Estrutura *)estrutura)->sock, LIMITE_REQUESTS);
        if (value < 0)
        {
            error("listen()", "A função retornou um erro ao ser executada"); // encerra a aplicação
            return -1;
        }

        //retorna 0 se tudo deu certo
        return 0;
    }
    /* PUBLISH */
    else if (type == PUB)
    {
        uint16_t publishPort = atoi(porta); // converte a porta passado para inteiro
        uint32_t publishIP = atoi(ip);      // coverte o ip passado para inteiro

        struct sockaddr_in publishEndereco;                   // Endereço local
        memset(&publishEndereco, 0, sizeof(publishEndereco)); // Limpa a struct
        publishEndereco.sin_family = AF_INET;                 // Família de endereço IPv4
        publishEndereco.sin_addr.s_addr = htonl(INADDR_ANY);  // De qualquer interface
        publishEndereco.sin_port = htons(publishPort);        // Porta local

        // executa o bind
        int retorno = bind(((struct Estrutura *)estrutura)->sock, (struct sockaddr *)&publishEndereco, sizeof(publishEndereco));
        if (retorno < 0)
        {
            error("bind()", "A função retornou um erro ao ser executada"); // encerra a aplicação
            return -1;
        }

        // executa o listen
        retorno = listen(((struct Estrutura *)estrutura)->sock, 10);
        if (retorno < 0)
        {
            error("bind()", "A função retornou um erro ao ser executada"); //encerra a aplicação
            return -1;
        }

        inicializarListaDeSubscribers(estrutura); // inicializo a lista de subscribers com -1 para informar que tem vaga

        /*----Começo do Accept() usando a thread----*/
        /* Crio uma thread para ficar aceitandos o subscribes em background */
        pthread_t backgorund;
        pthread_create(&backgorund, NULL, aceitarSubscribes, estrutura); // Passo a estrutura como parametro para a thread
        /*----O Accept() irá ficá rodadando em background aceitando Subscribers----*/

        //retorna 0 se tudo deu certo
        return 0;
    }
    /* ERROR */
    else
    {
        error("my_bind()", "O type da estrutura deve conter tipos <REP> ou <PUB> para usar a funcao"); // encerro a aplicaçao
        return -1;
    }
}

int my_connect(void *estrutura, const char *protocolo, const char *ip, const char *porta)
{
    // Faz algumas validações dos argumentos recebidos
    if (estrutura == NULL)
    {
        error("<void *estrutura>", "O parametro está nulo"); // encerra a aplicação
        return -1;
    }

    int type = ((struct Estrutura *)estrutura)->type; // Guarda o type da estrutura numa váriavel

    /* REQUEST */
    if (type == REQ)
    {

        const char *replyIP = ip;           // guarda o ip passado pelo parametro
        in_port_t replyPorta = atoi(porta); // guarda e converte a porta para inteiro passado como parametro

        // Constrói a struct de endereço do servidor
        struct sockaddr_in enderecoReply;                 // Endereço servidor
        memset(&enderecoReply, 0, sizeof(enderecoReply)); // Limpa a struct

        enderecoReply.sin_family = AF_INET;                                      // Família de endereços IPv4
        int value = inet_pton(AF_INET, replyIP, &enderecoReply.sin_addr.s_addr); // Converte o endereço passado pelo parametro
        if (value == 0)
        {
            error("inet_pton()", "A string de endereço está inválida"); // encerra a aplicação
            return -1;
        }
        else if (value < 0)
        {
            error("inet_pton()", "A função retornou um erro"); // encerra a aplicação
            return -1;
        }

        enderecoReply.sin_port = htons(replyPorta); // Porta

        value = connect(((struct Estrutura *)estrutura)->sock, (struct sockaddr *)&enderecoReply, sizeof(enderecoReply)); // conecta o request ao reply
        if (value < 0)
        {
            error("connect()", "A função retornou um erro"); // encerra a aplicação
            return -1;
        }

        //retorna 0 se tudo deu certo
        return 0;
    }
    /* SUBSCRIBE */
    else if (type == SUB)
    {
        // Guardando os parametros
        const char *publishIP = ip;           // guarda o ip passado pelo parametro
        in_port_t publishPorta = atoi(porta); // guarda e converte a porta para inteiro passado como parametro

        // Constrói a struct de endereço do servidor
        struct sockaddr_in enderecoPublish;                   // Endereço servidor
        memset(&enderecoPublish, 0, sizeof(enderecoPublish)); // Limpa a struct

        enderecoPublish.sin_family = AF_INET;                                        // Família de endereços IPv4
        int value = inet_pton(AF_INET, publishIP, &enderecoPublish.sin_addr.s_addr); // Converte o endereço
        if (value == 0)
        {
            error("inet_pton()", "A string de endereço está inválida"); // encerra a aplicação
            return -1;
        }
        else if (value < 0)
        {
            error("inet_pton()", "A função retornou um erro"); // encerra a aplicação
            return -1;
        }

        enderecoPublish.sin_port = htons(publishPorta); // Porta

        value = connect(((struct Estrutura *)estrutura)->sock, (struct sockaddr *)&enderecoPublish, sizeof(enderecoPublish)); // conecta o subscribe ao publish
        if (value < 0)
        {
            error("connect()", "A função retornou um erro"); // encerra a aplicação
            return -1;
        }

        //retorna 0 se tudo deu certo
        return 0;
    }
    /* ERROR */
    else
    {
        error("my_connect()", "O type da estrutura deve conter tipos <REQ> ou <SUB> para usar a funcao"); // encerra a aplicação
        return -1;
    }
}

/*OBS: # O accept() será realizado aqui se o type for REPLY
       # Um cliente por vez.
*/
int my_recv(void *estrutura, void *msg, int msgTam, int flags)
{
    // Faz algumas validações dos argumentos recebidos
    if (estrutura == NULL)
    {
        error("<void *estrutura>", "O parametro está nulo"); // encerra a aplicação
        return -1;
    }

    int type = ((struct Estrutura *)estrutura)->type; // Guarda o type da estrutura numa variável

    /* REPLY */
    if (type == REP)
    {
        /*----Começo do Accept()----*/

        /* Entra nesse condicional se não possui request conectado 
            # -1 - desconectado ou vago
        */
        if (((struct Estrutura *)estrutura)->sockRequestComunicacao == -1)
        {
            struct sockaddr_in requestEndereco;                     // Endereço cliente
            socklen_t requestEnderecoTam = sizeof(requestEndereco); // Define o tamanho da struct de endereço do cliente

            /* Espera pela conexão de um cliente */
            puts("Esperando Pedidos De Requests");
            // O Reply aceita uma conexão por vez
            int sockRequestComunicacao = accept(((struct Estrutura *)estrutura)->sock, (struct sockaddr *)&requestEndereco, &requestEnderecoTam);
            if (sockRequestComunicacao < 0)
            {
                error("accept()", "A função retornou um erro."); // encerra a aplicação
                return -1;
            }

            ((struct Estrutura *)estrutura)->sockRequestComunicacao = sockRequestComunicacao; // adiciona o sock recebido pelo accept ao um atributo do Reply para poder fazer os recv e send com o request futuramente
        }
        /*----Fim do Accept()----*/

        /* Faz o controle de quando o Reply pode receber dados ou não através do atributo estadoAtual*/
        if (((struct Estrutura *)estrutura)->estadoAtual != RECV) // se não for RECV, então não pode receber
        {
            error("Reply", "Não é permitido receber um pedido enquanto não enviar a resposta do pedido anterior"); // encerra a aplicação
            return -1;
        }
        else // senão (== RECV), pode receber e muda o estado
        {
            ((struct Estrutura *)estrutura)->estadoAtual = SEND; // muda o estado
        }

        // recebendo os dados do Request
        ssize_t numBytesRcvd = recv(((struct Estrutura *)estrutura)->sockRequestComunicacao, (char *)msg, msgTam, 0);

        if (numBytesRcvd < 0) // O recv deu erro
        {
            error("recv()", "A funcao retornou um erro"); // encerra a aplicação
            return -1;
        }
        else if (numBytesRcvd == 0) // O Request fechou a conexão
        {
            close(((struct Estrutura *)estrutura)->sockRequestComunicacao); // fecha a conexao com este request também
            ((struct Estrutura *)estrutura)->sockRequestComunicacao = -1;   // libera a vaga para o proximo cliente
            puts("A conexão foi fechada pelo request");                     // mostra um aviso na tela
            return -1;
        }

        //retorna 0 se tudo deu certo
        return 0;
    }
    /* REQUEST */
    else if (type == REQ)
    {
        /* Faz o controle de quando o Request pode receber dados ou não através do atributo estadoAtual*/
        if (((struct Estrutura *)estrutura)->estadoAtual != RECV) // se não for RECV, então não pode receber
        {
            error("Request", "Não é permitido receber uma resposta enquanto não enviar um pedido"); // encerra a aplicação
            return -1;
        }
        else // senão (== RECV), pode receber e muda o estado
        {
            ((struct Estrutura *)estrutura)->estadoAtual = SEND; // muda o estado
        }

        /* Recebendo os dados do Reply */
        ssize_t numBytesRcvd = recv(((struct Estrutura *)estrutura)->sock, (char *)msg, msgTam, flags);
        if (numBytesRcvd < 0) // o recv deu erro
        {
            error("recv()", "A funcao retornou um erro"); // encerra a aplicação
            return -1;
        }
        else if (numBytesRcvd == 0) // o Reply fechou a conexão
        {
            close(((struct Estrutura *)estrutura)->sock);        // fecha a conexao com o Reply também
            error("recv()", "A conexão foi fechada pelo Reply"); // encerra a aplicação
            return -1;
        }

        //retorna 0 se tudo deu certo
        return 0;
    }
    /* SUBSCRIBE */
    else if (type == SUB)
    {
        // Recebendo dados do Publisher
        ssize_t numBytesRcvd = recv(((struct Estrutura *)estrutura)->sock, (char *)msg, msgTam, flags);
        if (numBytesRcvd < 0) // erro no recv
        {
            error("recv()", "A funcao retornou um erro"); //encerra a aplicação
            return -1;
        }
        else if (numBytesRcvd == 0) // O Publish fechou a conexão
        {
            close(((struct Estrutura *)estrutura)->sock);          // fecho a conexão também
            error("recv()", "A conexão foi fechada pelo Publish"); // encerro a aplicação
            return -1;
        }

        //retorna 0 se tudo deu certo
        return 0;
    }
    /* ERROR */
    else
    {
        error("my_recv()", "O type da estrutura deve conter tipos <REP>, <REQ> ou <SUB> para usar a funcao"); //encerra a aplicação
        return -1;
    }
}

int my_send(void *estrutura, void *msg, int msgTam, int flags)
{
    // Faz algumas validações dos argumentos recebidos
    if (estrutura == NULL)
    {
        error("<void *estrutura>", "O parametro está nulo"); //encerra a aplicação
        return -1;
    }

    int type = ((struct Estrutura *)estrutura)->type; // Guardando o type da estrutura

    /* REPLY */
    if (type == REP)
    {
        /* Faz o controle de quando o Reply pode enviar dados ou não */
        if (((struct Estrutura *)estrutura)->estadoAtual != SEND) // se o estado não for SEND, então não pode enviar
        {
            error("Reply", "Não é permitido enviar uma resposta enquanto não receber um pedido"); // encerra a aplicação
            return -1;
        }
        else //senão (estadoAtual == SEND): então pode enviar e muda o estado
        {
            ((struct Estrutura *)estrutura)->estadoAtual = RECV; // muda o estado
        }

        /* Se a conexão existir ainda, ele pode enviar 
            # igual a -1 não há conexão
            # diferente de -1 há conexão
        */
        if (((struct Estrutura *)estrutura)->sockRequestComunicacao != -1) // entra na condição se há conexão
        {
            ssize_t numBytes = send(((struct Estrutura *)estrutura)->sockRequestComunicacao, msg, msgTam, 0); // envia os dados para o request
            if (numBytes < 0)                                                                                 // erro no send
            {
                error("send()", "A funcao retornou um erro"); // encerra a aplicação
                return -1;
            }
        }
        else // se não há conexão, avisa e na tela
        {
            puts("A Request fechou a conexão, não é possivel enviar dados");
            return -1;
        }

        //retorna 0 se tudo deu certo
        return 0;
    }
    /* REQUEST */
    else if (type == REQ)
    {
        /* Faz o controle de quando o Request pode enviar dados ou não */
        if (((struct Estrutura *)estrutura)->estadoAtual != SEND) // se o estado for diferente de SEND, então não pode enviar
        {
            error("Request", "Não é permitido enviar um pedido enquanto não receber uma resposta do pedido anterior"); // encerra a aplicação
            return -1;
        }
        else // senao (estado == RECV), então ele pode enviar e muda o estado
        {
            ((struct Estrutura *)estrutura)->estadoAtual = RECV; // muda o estado
        }

        // Envia os dados ao Reply
        ssize_t numBytes = send(((struct Estrutura *)estrutura)->sock, msg, msgTam, 0);
        if (numBytes < 0)
        {
            error("send()", "A funcao retornou um erro"); // encerra a aplicação
            return -1;
        }
        else if (numBytes != msgTam)
        {
            error("send()", "A funcao enviou um número inesperado de bytes"); // encerra a aplicação
            return -1;
        }
    }
    /* PUBLISH */
    else if (type == PUB)
    {
        int i, erro = FALSE;
        for (i = 0; i < LIMITE_SUBSCRIBES; i++) // percorre toda a lista de Subscribers
        {
            /* Entra nessa condicao quando o Publish já sabe que não existe Subscribe nessa posicao */
            if (((struct Estrutura *)estrutura)->listaSockSubscribe[i] == -1)
            {
                continue; // vou para a proxima posicao
            }

            int conexaoSubscribeStatus = 0;                 // variavel para verificar se existe conexão atraves da funcaosockopt()
            socklen_t len = sizeof(conexaoSubscribeStatus); // tamanho dessa variavel

            /* Caso exista conexao com o Subscrib, preciso verificar se o Subscrib não fechou a conexão antes de dá send() através da função getsockopt
                # retorna 0 em conexaoSubscribeStatus se estiver ok
                # retorna != 0 conexaoSubscribeStatus se estiver fechado
            */
            getsockopt(((struct Estrutura *)estrutura)->listaSockSubscribe[i], SOL_SOCKET, SO_ERROR, &conexaoSubscribeStatus, &len);

            /* 
                O Publish ainda não sabe se ainda tem conexão com o Subscribe, por isso esta verificação
            */
            if (conexaoSubscribeStatus != 0) // Entra nessa condição se a conexão foi fechada pelo subscribe, bloqueando o envio para ele e liberando a vaga
            {
                close(((struct Estrutura *)estrutura)->listaSockSubscribe[i]); // fecho a conexão também com o Subscribe
                ((struct Estrutura *)estrutura)->listaSockSubscribe[i] = -1;   // defino o subscribe como -1 para liberar a vaga
                ((struct Estrutura *)estrutura)->totalSubscribes--;            // Atualizo o total de subscribes inscritos
                erro = TRUE;                                                   // usarei para retorna -1 para a aplicaçãop que está usando a API
                continue;                                                      // continuo com o proximo subscrib
            }

            if (((struct Estrutura *)estrutura)->listaSockSubscribe[i] != -1) // Essa condicional garante que o Publish não enviará para um Subscribe desconectado
            {
                ssize_t numBytes = send(((struct Estrutura *)estrutura)->listaSockSubscribe[i], msg, msgTam, 0); // enviando os dados
                if (numBytes < 0)
                {
                    error("send()", "A funcao retornou um erro"); // encerra a aplicação
                    erro = TRUE;                                  // houve erro, a funcao send() nao conseguiu enviar para o subscribe
                }
            }
        }

        /* Se um ou mais subscrib se desconectaram, retorno -1 para identificar o que houve*/
        if (erro == TRUE)
        {
            return -1;
        }

        //retorna 0 se tudo deu certo (enviou para todos os Subscribers )
        return 0;
    }
    /* ERROR */
    else
    {
        error("my_send()", "O type da estrutura deve conter tipos <REP>, <REQ> ou <PUB> para usar a funcao"); // encerra a aplicação
        return -1;
    }

    //retorna 0 se tudo tiver ok
    return 0;
}

int my_close(void *estrutura)
{
    // Válido os parametros recebidos
    if (estrutura == NULL)
    {
        error("<void *estrutura>", "O parametro está nulo"); // encerra a aplicação
        return -1;
    }

    if (close(((struct Estrutura *)estrutura)->sock) < 0)
    {
        error("close()", "a funcao retornou um erro");
        return -1; // houve
    }
    else
    {
        //retorna 0 se tudo deu certo
        return 0;
    }
}

/* Funcões Auxiliares */

/* Recebe 2 strings que identificam o erro e encerra a aplicação*/
void error(char *erro, char *mensagem)
{
    printf("ERROR: %s -- %s\n", erro, mensagem);
    exit(EXIT_FAILURE);
}

void *aceitarSubscribes(void *estrutura)
{
    struct sockaddr_in subscribeEndereco;                       // Endereço cliente
    socklen_t subscribeEnderecoTam = sizeof(subscribeEndereco); // Define o tamanho da struct de endereço do cliente

    /* Executa para sempre esperando Subscribers */
    while (TRUE)
    {
        puts("Esperando Conexões De Novos Subscribers");                                                                                            // aviso na tela
        int sockSubscribeComunicacao = accept(((struct Estrutura *)estrutura)->sock, (struct sockaddr *)&subscribeEndereco, &subscribeEnderecoTam); // aceito um novo subscriber
        if (sockSubscribeComunicacao < 0)                                                                                                           // houve erro no accept()
        {
            error("accept()", "A função retornou um erro."); // encerra a aplicação
            continue;
        }
        else // se nao, guardará o sock do subscrib na lista
        {
            int i, isVaga = FALSE;
            for (i = 0; i < LIMITE_SUBSCRIBES; i++) // percorre toda a lista a procura de uma vaga livre
            {
                if (((struct Estrutura *)estrutura)->listaSockSubscribe[i] == -1) // == -1, então há vaga
                {
                    ((struct Estrutura *)estrutura)->listaSockSubscribe[i] = sockSubscribeComunicacao; // atribui o sock do retorno do accpet a essa posição livre
                    ((struct Estrutura *)estrutura)->totalSubscribes++;                                // incrementa o numero de subscribers inscritos
                    isVaga = TRUE;                                                                     // variavel vai servir para mostrar informação na tela logo abaixo
                    break;
                }
            }

            if (isVaga == TRUE)
            {
                puts("Um Novo Subscrib Se Inscreveu"); // mostra informacao na tela que um novo subscribe se inscreveu
            }
            else
            {
                puts("Um Novo Subscrib Tentou Se Inscrever, Mas Não Há Vagas"); // mostra na tela que um subscriber tentou se inscrever mas não houve vaga
                close(sockSubscribeComunicacao);                                // fecha a conexão com esse subscriber
            }
        }
    }
}

// inicializa a lista de subscribers com -1 para identificar que há vaga
void inicializarListaDeSubscribers(void *estrutura)
{
    int i;
    for (i = 0; i < LIMITE_SUBSCRIBES; i++)
    {
        ((struct Estrutura *)estrutura)->listaSockSubscribe[i] = -1;
    }
}

void imprimirInformacoes(void *estrutura)
{
    struct Estrutura *informacoes = (struct Estrutura *)estrutura;
    puts("Informações: ");
    printf("pointer: %p\n", informacoes);
    printf("sock: %d\n", informacoes->sock);
    printf("type: %d\n", informacoes->type);
    printf("protocolo: %d\n", informacoes->protocolo);
    printf("estadoAtual: %d\n", informacoes->estadoAtual);
}