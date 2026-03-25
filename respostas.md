# Simulado da Avaliação Intermediária de Supercomp, 1 semestre de 2026

## Questões Teóricas:
A resposta para questões teóricas devem estar em um arquivo "respostas_teoricas.txt"

1) O que é um sistema de HPC? (0.5 ponto)

Um sistema de HPC, é comummente composto por um sistema de cluster, sejam eles
especializados ou de prateleira. Esse clusters são formados por diversas
máquinas que podem ter recursos diferentes para tarefas diferentes, como por
exemplo CPU, Memória, GPU, entre outros. Eles são utilzados para tarefas que são
muito grandes, ou computacionalmente pesadas para serem processadas localmente
por computadores comuns, para isso utilizam sistemas de gerênciamento como o
SLURM

Pontos de atenção: 

- Identifica corretamente que sistemas modernos de HPC são baseados em
  arquiteturas de clusters.
- Destaca a integração e o uso de recursos de hardware heterogêneos para
  maximizar o desempenho. 
- Relaciona o uso dessas arquiteturas à resolução de problemas cujas
  categorias demandam poder de processamento e dados superiores ao de
  computadores convencionais.
- Cita o uso indispensável de um gerenciador de cargas de trabalho (como o
SLURM) para orquestrar os recursos

2) Qual é a função do SLURM em um sistema de HPC? (0.5 ponto)

O SLURM é sistema de gestão de recurso e trabalho, ele é responsável por
distribuir o que está disponível no cluster para os usuários, a depender do que
foi pedido, e utiliza diversas métricas para esse fornecimento. 

Pontos de atenção: 

- Define com precisão o SLURM como um gerenciador de recursos e escalonador de trabalhos (jobs) no cluster.
- Compreende que a alocação do hardware é feita sob demanda, baseando-se nos
recursos que o usuário pede de forma explícita, como partições, tempo,
memória e núcleos.

3) Explique por que um loop com dependência entre iterações não pode ser paralelizado diretamente. (0.5 ponto)

Não conseguimos paralelizar um loop com dependência entre as iterações de forma
direta pois ao fazer isso estaremos adicionando um erro de concorrência entre as
threads que farão acesso de um mesmo recurso dependente, e dessa forma é
possivel que cada thread utilize dados diferêntes para seus calculos, o que irá
gerar um erro, a "race condition". Ainda que utilizemos um sistema semfórico,
como atomic, ou critical, perderemos o ganho do processamento paralelo já que
cada thread ficará com gargalo no dado dependente. 

Pontos de atenção: 

- Aponta corretamente que a paralelização direta de iterações dependentes causa uma condição de corrida (race condition), onde múltiplas threads acessam variáveis desatualizadas ou inconsistentes, gerando resultados incorretos.
- Demonstra profundo entendimento arquitetural e lógico ao explicar que as soluções de sincronização (atomic ou critical), embora corrijam a integridade dos dados, serializam a execução e criam gargalos, anulando os ganhos de desempenho do paralelismo


4) Qual a diferença entre paralelismo com memória compartilhada e paralelismo com memória distribuída. (0.5 ponto)

No paralelimo com memória compartilhada, estamos trabalhando dentro de um mesmo
nó de computação, ou seja todas a threads tem acesso aos mesmos bancos de
memória, conseguimos gerênciar o acesso a essa memória
com os pragmas critica e atomic por exemplo.
Já na memória distribuida estamos trabalhando com vários nós de computação, cada
um com seu banco de memória, e precisamos fazer a comunicação ativamente via
mensagens. 

Pontos de atenção: 

- Explica perfeitamente que o modelo de memória compartilhada funciona dentro de um espaço de endereçamento comum para as threads (geralmente no mesmo nó), exigindo a coordenação do acesso aos dados com mecanismos de proteção como critical e atomic.
- Diferencia a arquitetura corretamente ao detalhar que, na memória distribuída, os nós computacionais são independentes e possuem memórias isoladas, exigindo que qualquer troca de informações seja feita ativamente e explicitamente através de redes com o envio/recebimento de mensagens (como no modelo MPI)

## Questões práticas
Para cada questão, você deve organizar sua resposta da seguinte forma:
* Questões de implementação devem ser entregues em arquivos no formato: Qx.cpp (onde x é o número da questão)

* Questões que exigem execução em cluster devem incluir também o respectivo arquivo de submissão: run_Qx.slurm

* Questões teóricas devem ser respondidas em arquivos de texto: Qx.txt


## **Exercício 1 - Otimizações em CPU - (2 pontos)**

Abaixo temos um código base implementado de forma sequencial em CPU, da forma menos eficiente que eu consegui imaginar, ele realiza o cálculo da soma, média, quantidade de picos e quantidade de vales de um sensor fictício. Quando executado na fila **gpu** do cluster Franky, o programa produz os seguintes resultados:

```bash
Processando vetor de 123456789 elementos...
Soma final: 7.60016e+09
Média final: 61.5613
Quantidade de picos: 123456
Quantidade de vales: 123456
Tempo CPU sequencial: 991.454 ms
```

### Implemente as seguintes otimizções:

- Melhore o acesso aos dados, usando passagem por referência ou por ponteiro **+0.2 pontos**
- Aplicar Tilling para melhorar a localidade espacial e temporal e aproveitar melhor a memória cache **+0.5 pontos**
- Aplicar paralelismo em CPU usando OpenMP **+0.8 pontos**
- Criar um arquivo 'run.slurm' que solicita adequadamete os recursos de hardware para o Cluster Franky executar a versão com paralelismo em **CPU**. **+0.5 pontos**

```cpp
#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <iomanip>

// Função ineficiente: inicializa vetor por valor
std::vector<float> inicializar_vetor(std::vector<float> v, size_t N) {
    for (size_t i = 0; i < N; i++) {
        v[i] = (i % 1000) * 0.123f;
    }
    return v; // retorna uma cópia
}

// Função muito ruim: processa os dados por valor
std::vector<float> processar_dados(
    std::vector<float> v,
    size_t N,
    double &soma,
    size_t &num_picos,
    size_t &num_vales
) {
    std::vector<float> out(N, 0.0f);

    soma = 0.0;
    num_picos = 0;
    num_vales = 0;

    for (size_t i = 1; i < N - 1; i++) {

        float a = v[i - 1];
        float b = v[i];
        float c = v[i + 1];

        // Média local
        float media_local = (a + b + c) / 3.0f;

        // Pico
        bool pico = (b > a && b > c);

        // Vale
        bool vale = (b < a && b < c);

        if (pico) num_picos++;
        if (vale) num_vales++;

        out[i] = media_local + (pico ? b : 0.0f);

        soma += out[i];
    }

    return out; // retorna outra cópia gigante </3
}

// Função horrivel: calcula média por valor
double calcular_media(std::vector<float> out, double soma, size_t N) {
    return soma / N;
}


int main() {
    const size_t N = 123'456'789;

    // Vetor inicial
    std::vector<float> v(N);

    std::cout << "Processando vetor de " << N << " elementos...\n";

    // Inicialização
    v = inicializar_vetor(v, N);

    auto t0 = std::chrono::high_resolution_clock::now();

    double soma = 0.0;
    size_t num_picos = 0;
    size_t num_vales = 0;

    // Processamento sequencial do vetor
    std::vector<float> out = processar_dados(v, N, soma, num_picos, num_vales);

    // Mais uma cópia desnecessária de dados para calcular média :'(
    double media_final = calcular_media(out, soma, N);

    auto t1 = std::chrono::high_resolution_clock::now();
    double tempo_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::cout << "Soma final: "           << soma        << "\n";
    std::cout << "Média final: "          << media_final << "\n";
    std::cout << "Quantidade de picos: "  << num_picos   << "\n";
    std::cout << "Quantidade de vales: "  << num_vales   << "\n";
    std::cout << "Tempo CPU sequencial: " << tempo_ms    << " ms\n";

    return 0;
}


```




## **Exercício 2 — Cálculo de PI com Monte Carlo**

O código abaixo implementa o cálculo de PI usando o método de Monte Carlo de forma **sequencial**. Seu objetivo é otimizar este programa usando OpenMP.

### **Parte 1 - Paralelismo com OpenMP (2 pontos)**

* **0 pontos** → O aluno **não paralelizou** o código, ou a versão apresentada **não compila/não executa**.
 
* **0.5 ponto** → O aluno tentou paralelizar, mas não tratou corretamente a variável compartilhada (`dentro`) causando **condição de corrida**. O programa roda, mas os resultados são inconsistentes.

* **1 ponto** → O aluno paralelizou corretamente, garantindo que o código execute sem conflitos.

* **2 pontos** -> O aluno paralelizou corretamente, garantindo que o código execute sem conflitos, ajustou o número de pontos e configurou corretamente os recursos no SLURM, de modo que o programa gera uma estimativa de PI com precisão 3,1415 em menos de 10 segundos.

---

### **Parte 2 - Distribuição com MPI (2 pontos)**
Você deve:
    - Distribuir o número total de pontos N entre os processos MPI
    - Cada processo deve calcular sua contagem local de pontos 
    - Utilizar sementes diferentes para o gerador de números aleatórios em cada processo
    - Centralizar as respostas no Rank0 para exibir o valor de PI calculado.

* **0 pontos** → Não implementou MPI ou o código não compila/não executa

* **0.5 ponto** → Implementação incompleta, apresentando problemas como:
    Todos os processos utilizam a mesma seed;
    Problema de distribuição de dados (todos os nós fazem o mesmo trabalho)

* **2 pontos** → Implementação correta, incluindo:
    Distribuição adequada de N entre os processos;
    Uso de seeds independentes por processo;
    Cálculo correto de PI exibido pelo Rank 0 usando o resultado das contas realizadas nos nós de computação


```cpp
#include <iostream>
#include <random>
#include <chrono>
#include <iomanip>
using namespace std;

int main(int argc, char** argv) {
    long long N = (argc > 1 ? atoll(argv[1]) : 100000); // quantidade de pontos

    mt19937 rng(42); // gerador de números aleatórios
    uniform_real_distribution<double> dist(0.0, 1.0);

    long long dentro = 0;

    chrono::high_resolution_clock::time_point t0 = chrono::high_resolution_clock::now();

    // Código sequencial
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
```

## **Exercício 3 — Otimização de Rotas para Entregas com Bicicleta (2 pontos)**

Voltamos ao problema das entregas, mas agora estamos auxiliando o Rodolfo, um jovem audacioso que realiza entregas de bicicleta.

Devemos considerar que Rodolfo possui limitações físicas e operacionais:
    - Ele consegue carregar no máximo 2 pacotes por vez
    - Precisa retornar ao ponto de coleta sempre que esvaziar a carga
    - Sua meta diária é realizar 60 entregas
    - Os pontos de entrega estão [disponívies aqui](pontos.txt)

O objetivo é:

Determinar uma rota de entregas que minimize a distância total percorrida (custo), respeitando a limitação de carga.

Rúbrica:

* **0** - Não compila, não é possível testar a implementação.   
* **+0.2** - Implementou a heuristica hill climbing aleatória, mas sem otimizações.
* **+0.2** - Otimizou a heurísitca melhorando seu tempo de execução, mas ainda tem uma implementação sequencial.
* **+0.6** - Paralelizou o algorítimo e conseguiu melhorar o tempo em relação a versão sequencial.
* **+0.4** - Montou o arquivo SLURM de forma correta e exibiu uma tabela comprovando que a versão paralela é mais eficiente que a versão sequencial
* **+0.6** - Conseguiu explicar os resultados obtidos nos seus testes com as otimizações aplicadas de forma clara e objetiva.


