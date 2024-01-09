#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>

#define MAX 10000

typedef struct
{
    int qtde_elem;
    int* vetor;
} ParametrosThread;

// Verifica se um número é perfeito
bool eh_perfeito(int num);

// Executa o cálculo sequencial de números perfeitos
double sequencial(int vet[], int tam);

// Executa o cálculo de números perfeitos utilizando threads
double threads(int vet[], int tam_vet, int num_threads);

// Cria um vetor correspondente à parte de um vetor original destinada a uma thread
int* vetor_thread(int vet[], int tam, int pos_inicial);

// Função executada por cada thread
void* inicia_thread(void* parametros);

// Calcula a diferença de tempo em segundos entre duas instâncias de timespec
double diffTimeSec(struct timespec t0, struct timespec t1);

int main(int argc, char *argv[])
{
    srand(time(NULL));

    char *end_ptr_threads;
    char *end_ptr_vet;

    long num_threads;
    long tam_vet;

    // Verifica se o número correto de argumentos foi fornecido
    if (argc != 3) {
        printf("Quantidade de argumentos inválida\n");
        exit(1);
    } else {
        // Converte argumentos de linha de comando para números inteiros
        num_threads = strtol(argv[1], &end_ptr_threads, 10);
        tam_vet = strtol(argv[2], &end_ptr_vet, 10);

        // Verifica se os argumentos são válidos
        if ((*end_ptr_threads != '\0' || num_threads <= 0) || (*end_ptr_vet != '\0' || tam_vet <= 0)) {
            printf("Os valores devem ser positivos maiores que zero.\n");
            exit(1);
        }    
    }

    // Inicializa um vetor com valores aleatórios entre 1 e MAX
    int vet[tam_vet];
    for (int i = 0; i < tam_vet; i++) {
        vet[i] = (rand() % MAX + 1);
    }

    // Executa o cálculo sequencial de números perfeitos
    double tempo_sequencial = sequencial(vet, tam_vet);

    // Executa o cálculo de números perfeitos utilizando threads
    double tempo_threads = threads(vet, tam_vet, num_threads);

    // Imprime os resultados
    printf("-----------------------------\n");
    printf("Tempo sequencial: %lfs\n", tempo_sequencial);
    printf("Tempo com threads: %lfs\n", tempo_threads);
    printf("-----------------------------\n");

    return 0;
}

// Verifica se um número é perfeito
bool eh_perfeito(int num)
{
    int soma = 0;
    for (int i = 1; i < num; i++) {
        if (num % i == 0) {
            soma += i;
        }
    }
    return soma == num;
}

// Executa o cálculo sequencial de números perfeitos
double sequencial(int vet[], int tam)
{
    struct timespec start, end;

    // Marca o início do cálculo do tempo
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    int qtde_perfeitos = 0;
    for (int i = 0; i < tam; i++) {
        if (eh_perfeito(vet[i])) {
            qtde_perfeitos++;
        }
    }

    // Marca o fim do cálculo do tempo
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);

    // Imprime o resultado
    printf("-----------------------------\n");
    printf("* Sequencial: %d perfeito(s)\n", qtde_perfeitos);

    // Retorna o tempo decorrido
    return diffTimeSec(start, end);
}

// Executa o cálculo de números perfeitos utilizando threads
double threads(int vet[], int tam_vet, int num_threads)
{
    struct timespec start, end;

    int quociente = tam_vet / num_threads;
    int resto = tam_vet % num_threads;
    int i, qtde_elem_thread, pos_inicio = 0;

    // Cria arrays para armazenar IDs e parâmetros das threads
    pthread_t threads[num_threads];
    ParametrosThread parametros_thread[num_threads];

    // Itera sobre o número de threads
    for (i = 0; i < num_threads; i++) {
        if (i < resto) {
            qtde_elem_thread = quociente + 1;
        } else {
            qtde_elem_thread = quociente;
        }

        // Inicializa parâmetros da thread
        parametros_thread[i].qtde_elem = qtde_elem_thread;
        parametros_thread[i].vetor = vetor_thread(vet, qtde_elem_thread, pos_inicio);

        // Atualiza posição inicial para próxima thread
        pos_inicio += qtde_elem_thread;
    }

    intptr_t total_perfeitos = 0;

    // Aguarda as threads terminarem
    printf("-----------------------------\n");
    // Marca o início do cálculo de tempo
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    // Cria e inicia cada thread
    for (i = 0; i < num_threads; i++) {
        void* resultado_thread;
        
        // Cria a thread
        if (pthread_create(&(threads[i]), NULL, inicia_thread, (void*)&parametros_thread[i]) != 0) {
            fprintf(stderr, "Erro ao criar a thread %d\n", i);
            exit(1);
        }

        // Aguarda a thread terminar e obtém seu resultado
        if (pthread_join(threads[i], &resultado_thread) != 0) {
            fprintf(stderr, "Erro ao aguardar a thread %d\n", i);
            exit(1);
        }

        // Converte o resultado para um tipo inteiro
        intptr_t qtde_perfeitos = (intptr_t)resultado_thread;

        // Atualiza o total de números perfeitos
        total_perfeitos += qtde_perfeitos;

        // Imprime o resultado da thread
        printf("* Thread %d: %ld perfeito(s)\n", i + 1, qtde_perfeitos);
    }

    // Marca o fim do cálculo de tempo
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);

    // Imprime o total de números perfeitos
    printf(" [TOTAL] %ld perfeito(s)\n", total_perfeitos);

    // Retorna o tempo decorrido
    return diffTimeSec(start, end);
}

// Cria um vetor correspondente à parte de um vetor original destinada a uma thread
int* vetor_thread(int vet[], int tam, int pos_inicial)
{
    int* vet_thread = (int*)malloc(tam * sizeof(int));
    for (int i = 0; i < tam; i++) {
        vet_thread[i] = vet[pos_inicial];
        pos_inicial++;
    }

    return vet_thread;
}

// Função executada por cada thread
void* inicia_thread(void* parametros)
{
    ParametrosThread* parametro = (ParametrosThread*)parametros;
    int total_perfeitos = 0;

    // Itera sobre o vetor da thread, verificando números perfeitos
    for (int i = 0; i < parametro->qtde_elem; i++) {
        if (eh_perfeito(parametro->vetor[i])) {
            total_perfeitos++;
        }
    }

    // Retorna o total de números perfeitos como um ponteiro para evitar problemas de tipo
    return (void*)(intptr_t)total_perfeitos;
}

// Calcula a diferença de tempo em segundos entre duas instâncias de timespec
double diffTimeSec(struct timespec t0, struct timespec t1)
{
    return ((double)t1.tv_sec - t0.tv_sec) + ((double)(t1.tv_nsec - t0.tv_nsec) * 1e-9);
}
