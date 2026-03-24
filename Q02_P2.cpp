#include <iostream>
#include <random>
#include <chrono>
#include <iomanip>
#include <omp.h>
#include <mpi.h>

using namespace std;

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    long long N = (argc > 1 ? atoll(argv[1]) : 10000000000); // quantidade de pontos

    long long localN = N/size;

    long long dentro = 0;

    chrono::high_resolution_clock::time_point t0 = chrono::high_resolution_clock::now();

    #pragma omp parallel reduction(+:dentro) 
    {
        int thread_id = omp_get_thread_num();
        mt19937 rng(rank * 4200 + thread_id * 752 ); // gerador de números aleatórios
        uniform_real_distribution<double> dist(0.0, 1.0);



        // Código sequencial
        #pragma omp for schedule(dynamic)
        for (long long i = 0; i < localN; i++) {
            double x = dist(rng);
            double y = dist(rng);
            if (x * x + y * y <= 1.0) {
                dentro++;
            }
        }
    }

    chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

    long long reduce_dentro = 0;
    MPI_Reduce(&dentro, 
                &reduce_dentro, 
                1, 
                MPI_LONG_LONG, 
                MPI_SUM, 
                0,
                MPI_COMM_WORLD);

    if(rank == 0){
        double pi = 4.0 * (double)reduce_dentro / (double)N;
        double tempo = chrono::duration<double>(t1 - t0).count();
        cout << fixed << setprecision(4);
        cout << "N = " << N << "  pi = " << pi << "  tempo = " << tempo << "s" << endl;
    }

    MPI_Finalize();

    return 0;
}

/** 
 * Mudanças:
 * 1. Adicionado o Pragma parallel com reduction no loop que calcula o dentro
 * 2. Vou usar MPI reduce também. 
 **/
