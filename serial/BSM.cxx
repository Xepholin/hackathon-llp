/* 

    Monte Carlo Hackathon created by Hafsa Demnati and Patrick Demichel @ Viridien 2024
    The code compute a Call Option with a Monte Carlo method and compare the result with the analytical equation of Black-Scholes Merton : more details in the documentation

    Compilation : g++ -O BSM.cxx -o BSM

    Exemple of run: ./BSM #simulations #runs

    We want to measure 1000 runs and get the average error below a specific level 
    Adjust the parameter #simulations to achieve the expected Average Relative Error

    points given for achieving Average Relative Error for 1000 runs < Average Relative Error: 0.01%     : short           ~20mn tuned and all cores 
    points given for achieving Average Relative Error for 1000 runs < Average Relative Error: 0.005%    : normal          ~1h
    points given for achieving Average Relative Error for 1000 runs < Average Relative Error: 0.002%    : long            ~8h  
    points given for achieving Average Relative Error for 1000 runs < Average Relative Error: 0.001%    : super long :    ~24h 

    You can observe that from run to run there is a small difference caused using a different seed 
    Deliver the full logs that show the randomly selected seed ; it will permit us to raproduce and verify the results

    You need to run 10 times the program; with the same parameter 1 #simulations and 1000 as parameter 2 

    The performance is printed in the logs : more points given for each objective to the team with the best performance, the second, third and so on ...

    0.773595%    0.896091%      0.5748%    0.621321%    0.620323%    0.854219%    0.697301%    0.526567%    0.607043%    0.906975% ./BSM 100000    10 
     0.75403%    0.727078%     0.63101%    0.753609%    0.733543%    0.728597%    0.753131%    0.859521%    0.696769%    0.699988% ./BSM 100000    100

    0.282992%    0.181664%    0.317491%    0.254558%    0.194851%     0.22103%   0.0953011%    0.250809%    0.310949%    0.211331% ./BSM 1000000   10
    0.224017%    0.230809%    0.239547%    0.217105%    0.258575%      0.1944%    0.228919%    0.258778%    0.235938%     0.25739% ./BSM 1000000   100

    0.056911%   0.0929754%   0.0599475%   0.0681029%   0.0618026%    0.128031%   0.0389641%   0.0588954%   0.0651689%    0.122257% ./BSM 10000000  10
   0.0625289%   0.0785358%   0.0781138%   0.0781734%   0.0736234%   0.0811247%    0.076021%   0.0773279%   0.0867399%   0.0765197% ./BSM 10000000  100

   0.0200822%   0.0257806%   0.0207118%   0.0179176%   0.0191748%    0.024724%   0.0185942%   0.0138896%    0.027215%   0.0257985% ./BSM 100000000 10
   0.0227214%   0.0213892%   0.0198618%   0.0229917%   0.0213438%   0.0252195%   0.0235354%    0.022934%   0.0243098%   0.0221371% ./BSM 100000000 100

    As you can see the first parameter define the average precision 
    The second parameter as an average of multiple runs offer a smaller volativity of the result; that's why we ask for 1000 runs as second parameter "imposed" 
    You can run smaller values of parameter 2 while you experiment ; but for the final results use strictly 1000 

    The performance is somehow linear with the parameter 1 then multiple actions are expected to achieve all objectives
    Using the internet of chatgpt you can find and use another random generator; but you need to achieve similar numerical results since we use BSM algorithm to verify we are OK
    Except if you have a Nobel Price, you cannot change the code not measured by the performance mecanism
    You can use any method of parallelization or optimization
    You can use any compiler; vectorization; trigonometric library; we judge only numericla precision and performance 

    Provide the traces of the 10 runs 

*/

#include <iostream>
#include <cmath>
#include <random>
#include <vector>
#include <limits>
#include <algorithm>
#include <iomanip>   // For setting precision

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

// Function to generate Gaussian noise using Box-Muller transform
double gaussian_box_muller() {
    static std::mt19937 generator(std::random_device{}());
    static std::normal_distribution<double> distribution(0.0, 1.0);
    return distribution(generator);
}

// Function to calculate the Black-Scholes call option price using Monte Carlo method
double black_scholes_monte_carlo(ui64 S0, ui64 K, double T, double r, double sigma, double q, ui64 num_simulations) {
    double sum_payoffs = 0.0;
    for (ui64 i = 0; i < num_simulations; ++i) {
        double Z = gaussian_box_muller();
        double ST = S0 * exp((r - q - 0.5 * sigma * sigma) * T + sigma * sqrt(T) * Z);
        double payoff = std::max(ST - K, 0.0);
        sum_payoffs += payoff;
    }
    return exp(-r * T) * (sum_payoffs / num_simulations);
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

    std::vector<double> errors;
    double t1=dml_micros();
    for (ui64 run = 0; run < num_runs; ++run) {
        double theoretical_price = black_scholes_monte_carlo(S0, K, T, r, sigma, q, num_simulations);
        double actual_price = black_scholes_monte_carlo(S0, K, T, r, sigma, q, num_simulations);
        double relative_error = std::abs(theoretical_price - actual_price) / actual_price;
        errors.push_back(relative_error);

        // Print the values on one line with precision
        /*
        std::cout << std::fixed << std::setprecision(6)
                  << "Run " << run + 1 << ": "
                  << "Theoretical Price: " << theoretical_price << ", "
                  << "Actual Price: " << actual_price << ", "
                  << "Difference: " << theoretical_price - actual_price << std::endl; 
        */
    }
    double t2=dml_micros();

    double min_error     = *std::min_element(errors.begin(), errors.end());
    double max_error     = *std::max_element(errors.begin(), errors.end());
    double average_error =  std::accumulate (errors.begin(), errors.end(), 0.0) / errors.size();

    //std::cout << "%Best    Relative Error: " << min_error     * 100 << std::endl;
    //std::cout << "%Worst   Relative Error: " << max_error     * 100 << std::endl;
    std::cout << "%Average Relative Error: " << std::setprecision(9) << average_error * 100 << std::endl;
    std::cout << "Performance in seconds : " << std::setprecision(3) << (t2-t1)/1000000.0   << std::endl;

    return 0;
}
