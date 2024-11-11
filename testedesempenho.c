#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

#define ARRAY_SIZE 50000000
#define NUM_EXECUCOES 3

typedef struct {
    int num_threads;
    double tempo_medio;
    double speedup;
    double eficiencia;
} MetricaDesempenho;

void preencher_vetor(double *vetor, int tamanho) {
    #pragma omp parallel for
    for (int i = 0; i < tamanho; i++) {
        vetor[i] = 1.0 + (rand() % 100) / 1000.0;
    }
}

double calcular_produto_sequencial(double *vetor, int tamanho) {
    double produto = 1.0;
    for (int i = 0; i < tamanho; i++) {
        produto *= vetor[i];
    }
    return produto;
}

double calcular_produto_paralelo(double *vetor, int tamanho, int num_threads) {
    double produto = 1.0;
    omp_set_num_threads(num_threads);
    
    #pragma omp parallel for reduction(*:produto)
    for (int i = 0; i < tamanho; i++) {
        produto *= vetor[i];
    }
    return produto;
}

double executar_teste(double *vetor, int tamanho, int num_threads, int is_sequential) {
    double inicio, fim, tempo_total = 0.0;
    double resultado;

    for (int i = 0; i < NUM_EXECUCOES; i++) {
        inicio = omp_get_wtime();
        
        if (is_sequential) {
            resultado = calcular_produto_sequencial(vetor, tamanho);
        } else {
            resultado = calcular_produto_paralelo(vetor, tamanho, num_threads);
        }
        
        fim = omp_get_wtime();
        tempo_total += (fim - inicio);
    }

    return tempo_total / NUM_EXECUCOES;
}

int main() {
    srand(time(NULL));
    
    double *vetor = (double *)malloc(ARRAY_SIZE * sizeof(double));
    if (vetor == NULL) {
        printf("Erro na alocação de memória!\n");
        return 1;
    }

    preencher_vetor(vetor, ARRAY_SIZE);

    int threads[] = {1, 2, 4, 8, 16};
    int num_configs = sizeof(threads) / sizeof(threads[0]);
    MetricaDesempenho metricas[sizeof(threads) / sizeof(threads[0])];

    printf("\nIniciando testes de desempenho...\n");
    printf("Tamanho do vetor: %d\n", ARRAY_SIZE);
    printf("Execuções por teste: %d\n\n", NUM_EXECUCOES);

    double tempo_seq = executar_teste(vetor, ARRAY_SIZE, 1, 1);
    printf("Tempo sequencial médio: %.6f segundos\n\n", tempo_seq);

    printf("| Threads | Tempo Médio (s) | Speedup | Eficiência |\n");
    printf("|---------|----------------|---------|------------|\n");

    for (int i = 0; i < num_configs; i++) {
        metricas[i].num_threads = threads[i];
        metricas[i].tempo_medio = executar_teste(vetor, ARRAY_SIZE, threads[i], 0);
        metricas[i].speedup = tempo_seq / metricas[i].tempo_medio;
        metricas[i].eficiencia = metricas[i].speedup / threads[i];

        printf("| %7d | %14.6f | %7.2f | %10.2f%% |\n",
               threads[i],
               metricas[i].tempo_medio,
               metricas[i].speedup,
               metricas[i].eficiencia * 100);
    }

    free(vetor);

    printf("\nInformações adicionais para plotagem de gráficos:\n\n");
    printf("Dados para gráfico de Speedup:\n");
    printf("threads = [");
    for (int i = 0; i < num_configs; i++) {
        printf("%d%s", threads[i], i < num_configs - 1 ? ", " : "]\n");
    }
    printf("speedup = [");
    for (int i = 0; i < num_configs; i++) {
        printf("%.2f%s", metricas[i].speedup, i < num_configs - 1 ? ", " : "]\n");
    }

    printf("\nDados para gráfico de Eficiência:\n");
    printf("efficiency = [");
    for (int i = 0; i < num_configs; i++) {
        printf("%.2f%s", metricas[i].eficiencia * 100, i < num_configs - 1 ? ", " : "]\n");
    }

    return 0;
}