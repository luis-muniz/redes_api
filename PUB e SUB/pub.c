#include "mysocket.h"
#include <stdio.h>

int aux;

int main()
{
    void *publisher = my_socket(PUB);
    aux = my_bind(publisher, "tcp", "127.0.0.1", "55555");

    if (aux != 0)
    {
        puts("S: my_bind falhou");
    }

    time_t t;
    srand((unsigned)time(&t));

    int i = 0;
    while (TRUE)
    {

        char update[20];
        int temperature;
        temperature = (rand() % 215) - 80;
        sprintf(update,"%d",temperature);
        sleep(1);
        my_send(publisher, update,strlen(update),0);
    }
    puts("saiu do server");
}