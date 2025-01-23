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
#include <cmath>  // Pour std::erf et std::sqrt
#include <random>
#include <vector>
#include <limits>
#include <algorithm>
#include <iomanip>   // For setting precision
#include <armpl.h>

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


// Function to generate Gaussian noise using ArmPL
void gaussian_armpl(const int taille, double* noise, VSLStreamStatePtr stream) {
    vdRngGaussian(VSL_RNG_METHOD_GAUSSIAN_BOXMULLER2, stream, taille, noise, 0, 1);
}


// Unused
// Function to generate Gaussian noise using Box-Muller transform
double gaussian_box_muller() {
    static std::mt19937 generator(std::random_device{}());
    static std::normal_distribution<double> distribution(0.0, 1.0);
    return distribution(generator);
}

// Function to calculate the Black-Scholes call option price using Monte Carlo method
double black_scholes_monte_carlo(ui64 S0, ui64 K, ui64 num_simulations, double precomputed_stuff, double precomputed_return, VSLStreamStatePtr stream, double* Z_tab, double* tmpliste) {
    double sum_payoffs = 0.0;
    gaussian_armpl(num_simulations, Z_tab, stream);
    // Attempting to vectorize some parts of the computation
    // This might be slower due to reading tmplist multiple times
    // #pragma vector always
    // for (ui64 i = 0; i < num_simulations; ++i) {
    //     tmpliste[i] = precomputed_stuff * Z_tab[i];
    // }
    // for (ui64 i = 0; i < num_simulations; ++i) {
    //     tmpliste[i] = exp(tmpliste[i]);
    // }
    // #pragma vector always
    // for (ui64 i = 0; i < num_simulations; ++i) {
    //     tmpliste[i] = S0 * tmpliste[i] - K;
    // }
    // #pragma vector always
    // for (ui64 i = 0; i < num_simulations; ++i) {
    //     // double Z = gaussian_box_muller();
    //     if (tmpliste[i] > 0.0)
    //         sum_payoffs += tmpliste[i];
    // }

    // #pragma vector always
    // for (ui64 i = 0; i < num_simulations; ++i) {
    //     tmpliste[i] = exp(precomputed_stuff * Z_tab[i]);
    // }
    // #pragma omp unroll
    // for (ui64 i = 0; i < num_simulations; ++i) {
    //     double payoff = S0 * tmpliste[i] - K;
    //     sum_payoffs += std::max(payoff, 0.0);
    // }

    for (ui64 i = 0; i < num_simulations; ++i) {
        double ST = (S0 * exp(precomputed_stuff * Z_tab[i])) - K;
        double payoff = std::max(ST, 0.0);
        sum_payoffs += payoff;
    }
    return sum_payoffs * precomputed_return;
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

    // https://developer.arm.com/documentation/101004/2410/Open-Random-Number-Generation--OpenRNG--Reference-Guide/Examples/skipahead-c?lang=en#skipahead-c
    VSLStreamStatePtr parallel_streams[num_runs];
    // VSLStreamStatePtr stream;
    // Seems faster than VSL_BRNG_MT19937. DO NOT USE VSL_BRNG_NONDETERM AS IT IS SUPER SLOW AND DISREGARDS SEED!!!
    // vslNewStream(&stream, VSL_BRNG_MCG59, global_seed);
    vslNewStream(&parallel_streams[0], VSL_BRNG_MCG59, global_seed);
    for (ui64 i = 1; i < num_runs; ++i)
    {
        vslCopyStream(&parallel_streams[i], parallel_streams[i - 1]);
        vslSkipAheadStream(parallel_streams[i], num_simulations * i);
    }

    double precomputed_return = exp(-r * T) * (1.0 / num_simulations);  // This might lose in precision!!!
    double precomputed_start = (r - q - 0.5 * sigma * sigma) * T + sigma * sqrt(T);
    #pragma omp parallel default(shared)
    {
        double partial_sum = 0.0;
        double* Z_tab = (double*)malloc(num_simulations * sizeof(double));
        double* tmpliste = (double*)malloc(num_simulations * sizeof(double));
        #pragma omp for schedule(runtime)
        for (ui64 run = 0; run < num_runs; ++run) {
            partial_sum += black_scholes_monte_carlo(S0, K, num_simulations, precomputed_start, precomputed_return, parallel_streams[run], Z_tab, tmpliste);
            // partial_sum += black_scholes_monte_carlo(S0, K, num_simulations, precomputed_start, precomputed_return, stream, Z_tab, tmpliste);
        }
        free(Z_tab);
        free(tmpliste);
        #pragma omp atomic
        sum += partial_sum;
    }
    // vslDeleteStream(&stream);
    for (ui64 i = 0; i < num_runs; ++i)
    {
        vslDeleteStream(&parallel_streams[i]);
    }

    double t2=dml_micros();
    std::cout << std::fixed << std::setprecision(6) << " value= " << sum/num_runs << " in " << (t2-t1)/1000000.0 << " seconds" << std::endl;

    return 0;
}
