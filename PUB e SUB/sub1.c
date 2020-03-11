#include "mysocket.h"
#include <stdio.h>

int aux;

int main()
{
    printf("Coletando atualizacoes do servidor ...\n");
    
    void *subscriber = my_socket(SUB);
    aux = my_connect(subscriber, "tcp", "127.0.0.1", "55555");

    if (aux != 0)
    {
        puts("S: my_connect falhou");
    }

    puts("Aqui");
    
    
    // Processa 100 updates
    int update_nbr;
    long total_temp = 0;
    for (update_nbr = 0; update_nbr < 10; update_nbr++)
    {
        char buffer[10] = "\0";
        my_send(subscriber, buffer, 10, 0);

        int temperature;
        printf("Temp %d: %s\n",update_nbr,buffer);
        sscanf(buffer,"%d",&temperature);

        total_temp += temperature;
    }

    printf("Media de temperatura: %d", (int) (total_temp/10));

    my_close(subscriber);
    return 0;
}