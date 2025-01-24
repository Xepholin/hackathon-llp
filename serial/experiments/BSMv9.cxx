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

// Added includes
#include <armpl.h>
// #include <arm_sve.h>
// #include <arm_neon.h>
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
    assert_ok(vdRngGaussian(VSL_RNG_METHOD_GAUSSIAN_BOXMULLER2,
                            stream, taille, noise, 0, 1),
              "Number generation failed!");
}


// Unused
// Function to generate Gaussian noise using Box-Muller transform
double gaussian_box_muller() {
    static std::mt19937 generator(std::random_device{}());
    static std::normal_distribution<double> distribution(0.0, 1.0);
    return distribution(generator);
}


// Function to calculate the Black-Scholes call option price using
//  Monte Carlo method
double black_scholes_monte_carlo(ui64 S0, ui64 K, ui64 num_simulations,
                                 double precomputed_stuff,
                                 double precomputed_return,
                                 VSLStreamStatePtr stream, double* Z_tab,
                                 double* tmpliste)
{
    double sum_payoffs = 0.0;
    gaussian_armpl(num_simulations, Z_tab, stream);

    // Attempting to vectorize some parts of the computation
    // This might be slower due to reading tmplist multiple times (it is slower)
    // #pragma vector always
    // for (ui64 i = 0; i < num_simulations; ++i)
    // {
    //     tmpliste[i] = precomputed_stuff * Z_tab[i];
    // }
    // #pragma vector always
    // for (ui64 i = 0; i < num_simulations; ++i)
    // {
    //     tmpliste[i] = exp(tmpliste[i]);
    // }
    // #pragma vector always
    // for (ui64 i = 0; i < num_simulations; ++i)
    // {
    //     tmpliste[i] = S0 * tmpliste[i] - K;
    // }
    // #pragma vector always
    // for (ui64 i = 0; i < num_simulations; ++i)
    // {
    //     if (tmpliste[i] > 0.0)
    //         sum_payoffs += tmpliste[i];
    // }

    // THE FOLLOWING COMMENTED CODE DOES NOT WORK!!!
    // Attempt at vectorizing following loop using intrinsics
    // If it is efficient but compiler doesn't fusion loops,
    //  it might be a good idea to try to extend this to the other loop
    // The loop should end when Z_tab doesn't have any element anymore,
    //  and will compute a zero as the second element if num_simulations is odd
    // It doesn't compile with gcc, and SEGFAULT with armclang
    // However, armclang generates a vectorized loop so it's all good
//     ui64 vec_len = svcntd();  // Vector lenght
//     ui64 offset = 0;
//     svbool_t pg = svwhilelt_b64(offset, num_simulations);
//     do
//     {
//         // Load Z_tab into an SVE vector, with an offset
//         svfloat64_t vec_z = svld1_vnum(pg, Z_tab, offset);
//
//         // Multiply by precomputed_stuff
//         // Uses svdup to put it in a vector and allow svmul to work
//         svfloat64_t vec_precomp = svdup_f64(precomputed_stuff);
//         svfloat64_t vec_product = svmul_z(pg, vec_z, vec_precomp);
//
//         // This is from Neon and not SVE!!! That might be why GCC dies
//         // Trying to bypass svexpa_f64 wanting svuint64_t
//         //      (I'll be frank I've no idea how it works)
//         svuint64_t to_exp = svreinterpret_u64_f64(vec_product);
//
//         // Floating-point exponential accelerator
//         // "This instruction is unpredicated."" (cf arm's doc)
//         svfloat64_t vec_result = svexpa_f64(to_exp);
//
//         // Store result
//         svst1(pg, tmpliste, vec_product);
//         // svst1(pg, tmpliste, vec_result);
//         offset += vec_len;
//         pg = svwhilelt_b64(offset, num_simulations);
//     } while (svptest_any(svptrue_b64(), pg));

    // Said loop to be vectorized by above code
    #pragma vector always
    for (ui64 i = 0; i < num_simulations; ++i)
    {
        tmpliste[i] = exp(precomputed_stuff * Z_tab[i]);
    }
    #pragma vector always
    for (ui64 i = 0; i < num_simulations; ++i)
    {
        double payoff = S0 * tmpliste[i] - K;
        sum_payoffs += payoff > 0.0 ? payoff : 0.0;
    }

    // Enhanced initial loop
    // for (ui64 i = 0; i < num_simulations; ++i)
    // {
    //     double ST = (S0 * exp(precomputed_stuff * Z_tab[i])) - K;
    //     double payoff = std::max(ST, 0.0);
    //     sum_payoffs += payoff;
    // }
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

    #ifndef _OPENMP
    // VSLStreamStatePtr parallel_streams[num_threads];
    // #else
    VSLStreamStatePtr stream;
    #endif

    // VSL_BRNG_MCG59 seems faster than VSL_BRNG_MT19937.
    // DO NOT USE VSL_BRNG_NONDETERM AS IT IS SUPER SLOW AND DISREGARDS SEED!!!

    #ifndef _OPENMP
    assert_ok(vslNewStream(&parallel_streams[0], VSL_BRNG_MCG59, global_seed),
              "vslNewStreamFailed");
    // #else
    assert_ok(vslNewStream(&stream, VSL_BRNG_MCG59, global_seed),
              "vslNewStreamFailed");
    #endif

    // This precomputing might make us lose in precision!!!
    double precomputed_return = exp(-r * T) * (1.0 / num_simulations);
    double precomputed_start  = (r - q - 0.5 * sigma * sigma)
                                 * T + sigma * sqrt(T);

    // "Distributing" streams on each thread

    #pragma omp parallel default(shared)
    {
        #ifdef _OPENMP
        VSLStreamStatePtr stream;
        assert_ok(vslNewStream(&stream, VSL_BRNG_MCG59,
                               global_seed+omp_get_thread_num()),
                  "vslNewStreamFailed");
        #endif
        double* Z_tab    = (double*)malloc(num_simulations * sizeof(double));
        double* tmpliste = (double*)malloc(num_simulations * sizeof(double));
        // #ifdef _OPENMP
        // int thread_rank  = omp_get_thread_num();
        // #endif

        #pragma omp for schedule(runtime) reduction(+:sum)
        for (ui64 run = 0; run < num_runs; ++run)
        {
            // #ifdef _OPENMP
            // sum += black_scholes_monte_carlo(S0, K, num_simulations,
            //                                  precomputed_start,
            //                                  precomputed_return,
            //                                  parallel_streams[thread_rank],
            //                                  Z_tab, tmpliste);
            // #else
            sum += black_scholes_monte_carlo(S0, K, num_simulations,
                                             precomputed_start,
                                             precomputed_return,
                                             stream, Z_tab, tmpliste);
            // #endif
        }

        // Cleaning memory
        // #ifdef _OPENMP
        // #pragma omp for schedule(static, 1)
        // for (ui64 i = 0; i < num_threads; ++i)
        // {
        //     assert_ok(vslDeleteStream(&parallel_streams[i]), "vslDeleteStream");
        // }
        // #else
        assert_ok(vslDeleteStream(&stream), "vslDeleteStream");
        // #endif
        free(Z_tab);
        free(tmpliste);

        // #pragma omp atomic
        // sum += partial_sum;
    }

    double t2=dml_micros();
    std::cout << std::fixed << std::setprecision(6) << " value= " << sum/num_runs << " in " << (t2-t1)/1000000.0 << " seconds" << std::endl;

    return 0;
}
