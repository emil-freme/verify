#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <omp.h>
#include <iomanip>

// Função ineficiente: inicializa vetor por valor
// O que foi feito: 
// foi trocado a cópia do objeto vetor por uma referência
// N mantive como valor pois size_t é um dado muito pequeno, e não estamos
// alterando ele
void inicializar_vetor(std::vector<float> &v, size_t N) {
    //Como não preciso recuperar dados, é uma inicialização simples, não é
    //necessário fazer tilling
    //Utilizado simd para inicialização mais rápida do vetor
    #pragma omp simd
    for (size_t i = 0; i < N; i++) {
        v[i] = (i % 1000) * 0.123f;
    }
}

// Função muito ruim: processa os dados por valor
// O que foi feito: 
// Modificado para que a função para que não seja necessário fazer cópias dos
// vetores, no lugar de retornar utilizamos uma referência de saida também, que
// estará pré alocada.
void processar_dados(
                    std::vector<float> &v,
                    size_t N,
                    double &soma,
                    size_t &num_picos,
                    size_t &num_vales,
                    std::vector<float> &out
                    ) {

    soma = 0.0;
    num_picos = 0;
    num_vales = 0;

    //O que  foi feito:
    //Tilling aplicado no próximo loop

    size_t tile_size = 2048;


    //Paralelização do for externo do tilling
    #pragma omp parallel for reduction(+:soma, num_picos, num_vales)
    for (size_t i = 1; i < N - 1; i+=tile_size) {

        //Simd utilizado para pois já fizemos ele caber no Cahe L1
        #pragma omp simd reduction(+:soma, num_picos, num_vales)
        for(size_t j = i; j < std::min(i+tile_size, N-1); j++){

            float a = v[j - 1];
            float b = v[j];
            float c = v[j + 1];

            // Média local
            float media_local = (a + b + c) / 3.0f;

            // Pico
            bool pico = (b > a && b > c);

            // Vale
            bool vale = (b < a && b < c);

            if (pico) num_picos++;
            if (vale) num_vales++;

            out[j] = media_local + (pico ? b : 0.0f);

            soma += out[j];
        }
    }
}

// Função horrivel: calcula média por valor
// Removi o parametro não utilizado e inseri a diretiva inline que indica para
// o compilador que pode substituir o chamada da função por seu corpo.
inline double calcular_media(double soma, size_t N) {
    return soma / N;
}


int main() {
    const size_t N = 123'456'789;

    // Vetor inicial
    std::vector<float> v(N);
    std::vector<float> out(N, 0);

    std::cout << "Processando vetor de " << N << " elementos...\n";

    // Inicialização
    inicializar_vetor(v, N);

    auto t0 = std::chrono::high_resolution_clock::now();

    double soma = 0.0;
    size_t num_picos = 0;
    size_t num_vales = 0;

    // Processamento sequencial do vetor
    processar_dados(v, N, soma, num_picos, num_vales, out);

    // Mais uma cópia desnecessária de dados para calcular média :'(
    double media_final = calcular_media(soma, N);

    auto t1 = std::chrono::high_resolution_clock::now();
    double tempo_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::cout << "Soma final: "           << soma        << "\n";
    std::cout << "Média final: "          << media_final << "\n";
    std::cout << "Quantidade de picos: "  << num_picos   << "\n";
    std::cout << "Quantidade de vales: "  << num_vales   << "\n";
    std::cout << "Tempo CPU sequencial: " << tempo_ms    << " ms\n";

    return 0;
}

/**
 * Log:
 * 1. Melhorando o acesso dos dados passando por referência
 * 2. Aplicar Tilling, selecionei 2048 com tile_size
 * 3. Aplicar o OpemMP para paralelização de loops
 **/
