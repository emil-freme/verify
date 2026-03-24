#include <iostream>
#include <random>
#include <chrono>
#include <iomanip>
#include <omp.h>

using namespace std;

int main(int argc, char** argv) {
    long long N = (argc > 1 ? atoll(argv[1]) : 1000000000); // quantidade de pontos

    mt19937 rng(42); // gerador de números aleatórios
    uniform_real_distribution<double> dist(0.0, 1.0);

    long long dentro = 0;

    chrono::high_resolution_clock::time_point t0 = chrono::high_resolution_clock::now();

    // Código sequencial
    #pragma omp parallel for reduction(+:dentro)
    for (long long i = 0; i < N; i++) {
        double x = dist(rng);
        double y = dist(rng);
        if (x * x + y * y <= 1.0) {
            dentro++;
        }
    }

    chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

    double pi = 4.0 * (double)dentro / (double)N;
    double tempo = chrono::duration<double>(t1 - t0).count();
    cout << fixed << setprecision(4);
    cout << "N = " << N << "  pi = " << pi << "  tempo = " << tempo << "s" << endl;

    return 0;
}

/** 
 * Mudanças:
 * 1. Adicionado o Pragma parallel com reduction no loop que calcula o dentro
 *
 **/
