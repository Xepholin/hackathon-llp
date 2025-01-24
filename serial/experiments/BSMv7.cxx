/*

    Monte Carlo Hackathon created by Hafsa Demnati and Patrick Demichel @ Viridien 2024
    The code compute a Call Option with a Monte Carlo method and compare the result with the analytical equation of Black-Scholes Merton : more details in the documentation

    Compilation : g++ -O BSM.cxx -o BSM

    Exemple of run: ./BSM #simulations #runs

./BSM 100 1000000
Global initial seed: 21852687      argv[1]= 100     argv[2]= 1000000
 value= 5.136359 in 10.191287 seconds

./BSM 100 1000000
Global initial seed: 4208275479      argv[1]= 100     argv[2]= 1000000
 value= 5.138515 in 10.223189 seconds

   We want the performance and value for largest # of simulations as it will define a more precise pricing
   If you run multiple runs you will see that the value fluctuate as expected
   The large number of runs will generate a more precise value then you will converge but it require a large computation

   give values for ./BSM 100000 1000000
               for ./BSM 1000000 1000000
               for ./BSM 10000000 1000000
               for ./BSM 100000000 1000000

   We give points for best performance for each group of runs

   You need to tune and parallelize the code to run for large # of simulations

*/

#include <iostream>
#include <cmath> // Pour std::erf et std::sqrt
#include <random>
#include <vector>
#include <limits>
#include <algorithm>
#include <iomanip>   // For setting precision

// Addded includes
#include <armpl.h>
#ifdef _OPENMP
#include <omp.h>
#endif
//

#define ui64 u_int64_t

#include <sys/time.h>
double
dml_micros()
{
        static struct timezone tz;
        static struct timeval  tv;
        gettimeofday(&tv,&tz);
        return((tv.tv_sec*1000000.0)+tv.tv_usec);
}

// Helper function from ArmPL doc example
// https://developer.arm.com/documentation/101004/2410/Open-Random-Number-Generation--OpenRNG--Reference-Guide/Examples/skipahead-c?lang=en#skipahead-c
void assert_ok(int err, const char *message)
{
  if (err != VSL_ERROR_OK)
  {
    fprintf(stderr, "Error: %s\n", message);
    exit(EXIT_FAILURE);
  }
}


// Function to generate Gaussian noise using ArmPL
void gaussian_armpl(const int taille, double* noise, VSLStreamStatePtr stream)
{
    vdRngGaussian(VSL_RNG_METHOD_GAUSSIAN_BOXMULLER2,
                  stream, taille, noise, 0, 1);
}


// Function to calculate the Black-Scholes call option price using Monte Carlo method
double black_scholes_monte_carlo(ui64 S0, ui64 K, ui64 num_simulations,
                                 double precomp_one, double precomp_two,
                                 VSLStreamStatePtr stream,
                                 double* tmpliste, double* Z_tab)
{
    double sum_payoffs = 0.0;
    gaussian_armpl(num_simulations, Z_tab, stream);
    #pragma vector always
    for (ui64 i = 0; i < num_simulations; ++i)
    {
        tmpliste[i] = exp(precomp_one * Z_tab[i]);
    }
    #pragma vector always
    for (ui64 i = 0; i < num_simulations; ++i)
    {
        double payoff = S0 * tmpliste[i] - K;
        sum_payoffs += payoff > 0.0 ? payoff : 0.0;
    }
    return sum_payoffs * precomp_two;
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <num_simulations> <num_runs>" << std::endl;
        return 1;
    }

    ui64 num_simulations = std::stoull(argv[1]);
    ui64 num_runs        = std::stoull(argv[2]);

    // Input parameters
    ui64 S0      = 100;                   // Initial stock price
    ui64 K       = 110;                   // Strike price
    double T     = 1.0;                   // Time to maturity (1 year)
    double r     = 0.06;                  // Risk-free interest rate
    double sigma = 0.2;                   // Volatility
    double q     = 0.03;                  // Dividend yield

    // Generate a random seed at the start of the program using random_device
    std::random_device rd;
    unsigned long long global_seed = rd();  // This will be the global seed

    std::cout << "Global initial seed: " << global_seed << "      argv[1]= " << argv[1] << "     argv[2]= " << argv[2] <<  std::endl;

    double sum=0.0;
    double t1=dml_micros();

    // Trying to respect code compiling without -fopenmp
    #ifdef _OPENMP
    int num_threads;
    #pragma omp parallel default(shared)
    {
        #pragma omp single
        {
            num_threads = omp_get_num_threads();
            // std::cerr << "Using " << num_threads << " threads." << std::endl;
        }
    }
    #endif

    #ifdef _OPENMP
    VSLStreamStatePtr parallel_streams[num_threads];
    #else
    VSLStreamStatePtr stream;
    #endif

    // VSL_BRNG_MCG59 seems faster than VSL_BRNG_MT19937.
    // DO NOT USE VSL_BRNG_NONDETERM AS IT IS SUPER SLOW AND DISREGARDS SEED!!!

    #ifdef _OPENMP
    assert_ok(vslNewStream(&parallel_streams[0], VSL_BRNG_MCG59, global_seed),
              "vslNewStreamFailed");
    #else
    assert_ok(vslNewStream(&stream, VSL_BRNG_MCG59, global_seed),
              "vslNewStreamFailed");
    #endif

    // "Distributing" streams on each thread
    #ifdef _OPENMP  // Doing this part in parallel breaks things :/
    for (ui64 i = 1; i < num_threads; ++i)
    {
        assert_ok(vslCopyStream(&parallel_streams[i],
                                parallel_streams[i - 1]),
                  "vslCopyStream");
        // Taking more in case num_threads is not a factor
        assert_ok(vslSkipAheadStream(parallel_streams[i],
                                     ((num_simulations * num_runs)
                                     / num_threads)
                                     + (num_simulations * num_runs)),
                  "vslSkipAhead");
    }
    #endif

    double precomp_one = (r - q - 0.5 * sigma * sigma) * T + sigma * sqrt(T);
    double precomp_two = exp(-r * T) * (1.0 / num_simulations);
    #pragma omp parallel default(shared)
    {
        #ifdef _OPENMP
        int thread_rank = omp_get_thread_num();
        #endif
        double* tmpliste = (double*)malloc(num_simulations * sizeof(double));
        double* Z_tab    = (double*)malloc(num_simulations * sizeof(double));
        #pragma omp for reduction(+:sum)
        for (ui64 run = 0; run < num_runs; ++run)
        {
            #ifdef _OPENMP
            sum += black_scholes_monte_carlo(S0, K, num_simulations,
                                            precomp_one, precomp_two,
                                            parallel_streams[thread_rank],
                                            tmpliste, Z_tab);
            #else
            sum += black_scholes_monte_carlo(S0, K, num_simulations,
                                            precomp_one, precomp_two, stream,
                                            tmpliste, Z_tab);
            #endif
        }
        free(tmpliste);
    }
    double t2=dml_micros();
    std::cout << std::fixed << std::setprecision(6) << " value= " << sum/num_runs << " in " << (t2-t1)/1000000.0 << " seconds" << std::endl;

    return 0;
}
