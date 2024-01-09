#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

sem_t atendente;
sem_t cliente;
sem_t exclusao_mutua;

int fila_espera = 0;
int qtde_linhas_telefonicas;

void *inicia_thread_atendente(void *arg) 
{
    srand(time(NULL));
    while (1) {
        sem_wait(&cliente); // atendente AGUARDA até o cliente solicitar atendimento, cliente fica bloqueado
        sem_wait(&exclusao_mutua); // Entra na secao critica e bloqueia ela

        fila_espera--; // diminui o numero de clientes que estao na fila de espera
        sem_post(&atendente); // libera atendente, desbloqueado, pode atender o cliente
        sem_post(&exclusao_mutua); // sai da secao critica, que fica desbloqueada

        printf("Atendente esta falando com algum cliente! Ha %d chamada(s) em espera \n", fila_espera);
        sleep(rand() % 6 + 3); // Simula a chamada com duração entre 3 e 8 segundos
    }
}

void *inicia_thread_cliente(void *arg) 
{
    srand(time(NULL));
    int id = *((int*)arg);

    sem_wait(&exclusao_mutua); // cliente aguarda na secao critica, esta bloqueado

    if (fila_espera < qtde_linhas_telefonicas) { // se há linha de espera disponivel
        printf("Cliente %d esta ligando! Havia %d chamada(s) em espera\n", id, fila_espera);

        fila_espera++; // mais clientes na fila de espera
        sem_post(&cliente); // cliente deseja atendimento
        sem_post(&exclusao_mutua); // sai da seção crítica, desbloqueia

        sem_wait(&atendente); // cliente aguarda até que o atendente estar disponivel.
    } else {
        sem_post(&exclusao_mutua); // cliente nao conseguiu realizar a chamada todas as linhas ocupadas
        printf("Cliente %d nao consegue realizar a chamada. Todas as linhas ocupadas\n", id);
    }
}

int main(int argc, char *argv[]) 
{
    srand(time(NULL));

    char *end_ptr_linhas;

    if (argc != 2) {
        printf("Quantidade de argumentos invalida.\n");
        exit(1);
    } else {
        qtde_linhas_telefonicas = strtol(argv[1], &end_ptr_linhas, 10);
        if (qtde_linhas_telefonicas <= 0) {
            printf("O argumento precisa ser um numero positivo maior que zero.\n");
            exit(1);
        }
    }

    pthread_t atendente_thread;
    pthread_t cliente_thread;

    int clienteID = 1;

    sem_init(&atendente, 0, 0); // inicializa o semaforo de atendente com valor 0
    sem_init(&cliente, 0, 0); // inicializa o semáforo de cliente com valor 0
    sem_init(&exclusao_mutua, 0, 1); // inicializa o semáforo de exclusão mútua com valor 1

    printf("--------------------------------------------------------------------\n");
    printf("\tCall Center do CT (número de linhas de espera: %d)\n", qtde_linhas_telefonicas);
    printf("--------------------------------------------------------------------\n\n");

    pthread_create(&atendente_thread, NULL, inicia_thread_atendente, NULL);

    while (1) {
        pthread_create(&cliente_thread, NULL, inicia_thread_cliente, &clienteID);

        if (fila_espera == qtde_linhas_telefonicas) { // se todas as linhas estiverem ocupadas, espera 
            if(qtde_linhas_telefonicas == 1){
                sleep(rand() % 3 + 8);
            } else {
                sleep(rand() % (3*(qtde_linhas_telefonicas/2 )) + (8*(qtde_linhas_telefonicas/2)));
            }
        } else {
            sleep(rand() % 3 + 3); // gera clientes em intervalos entre 3 e 5 segundos
        }
        clienteID++;
    }

    pthread_join(atendente_thread, NULL);

    return 0;
}
