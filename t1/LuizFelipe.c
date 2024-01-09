#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>


void criaArvore(int argc, char* argv[], int nivel){
    int i, idProcesso;

    //condicao de parada
    if (nivel == argc - 1) {
        return; 
    }

    int qtdeFilhos = atoi(argv[nivel + 1]);

    // laço para criacao dos filhos do nó
    for (i=0; i<qtdeFilhos; i++) {

        idProcesso = fork();

        if (idProcesso == 0) { // processo filho
            printf("PPID = %d\n",getppid());
            printf("PID = %d\n",getpid());
            printf("--------------\n");
            criaArvore(argc,argv,nivel+1);
            exit(0);
        } else if (idProcesso == -1) {
            printf("Fork falhou\n");
            exit(1);
        } else { // processo pai
            int status;
            pid_t pidFilho;
            while ((pidFilho = wait(&status)) > 0) {
                printf("Processo pai %d: Processo filho com PID %d terminou\n", getpid(), pidFilho);
            }
        }
    }
}

int main(int argc, char *argv[]) 
{
    int i, num;//, numTotalFolhas = 1, qtdeFolhasCriados = 0;
    clock_t startTime = 0;

    // Verificação de consistência de entrada
    if (argc < 2) {
        printf("Faltam argumentos\n");
        exit(1);
    } else {
        for (i=1; i<argc; i++) {
            char* endptr; 
            long valor = strtol(argv[i], &endptr, 10);
            //numTotalFolhas *= valor;
            // Verifica se a conversão foi bem-sucedida
            if (*endptr != '\0' || valor <= 0) {
                printf("O argumento não é um número inteiro válido ou não é maior que zero.\n");
                exit(1);
            }
        }
    } 
    
    // Inicio da contagem do tempo, chamada da funcao de criacao da arvore e final da contagem de tempo
    startTime = clock();
    criaArvore(argc,argv,0);
    clock_t endTime = clock();

    // calculo do tempo
    double executionTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
    printf("Tempo de criação da árvore: %f segundos\n", executionTime);

    return 0;
}