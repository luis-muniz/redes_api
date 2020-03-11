#include "mysocket.h"
#include <stdio.h>

int aux;

int main()
{
    void *responder = my_socket(REP);
    int rc = my_bind(responder, "tcp", "*", "5555");
    while (1)
    {
        char buffer[10] = "\0";
        if (my_recv(responder, buffer, 10, 0) == 0)
        {
            printf("Recebi:  %s\n", buffer);
        }else{
            printf("falhou\n");
        }
        sleep(1); // Finge trabalho
        my_send(responder, "World", 5, 0);
    }
    return 0;
}