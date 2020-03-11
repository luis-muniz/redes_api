#include "mysocket.h"
#include <stdio.h>

int aux;

int main()
{
    void *request = my_socket(REQ);
    aux = my_connect(request, "TCP", "127.0.0.1", "5555");

    if (aux != 0)
    {
        puts("C: my_connect falhou");
    }

    int i;
    for (i = 0; i<5; i++)
    {
        aux = my_send(request, "Oi", 2, 0);
        printf("Enviando: Oi %d...\n",i);
        if (aux != 0)
        {
            puts("C: my_send falhou");
        }
        sleep(1);
        char buffer[10] = "\0";
        aux = my_recv(request, buffer, 10, 0);
        if (aux != 0)
        {
            puts("C: my_recv falhou");
        }

        printf("Recebendo: %s\n", buffer);

        // puts("++REQ++");
        // imprimirInformacoes(request);
    }
    my_close(request);
}