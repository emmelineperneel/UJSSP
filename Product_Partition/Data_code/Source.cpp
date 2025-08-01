#define MPFR_USE_NO_MACRO
#define MPFR_USE_INTMAX_T

#include <iostream> 
#include <random>
#include <math.h> 
#include <fstream> 
#include <sstream>
#include <algorithm>
#include <numeric>
#pragma warning(push)
#pragma warning(disable: 4146)
#include <gmpxx.h>
#include <mpfr.h>
#include "mpreal.h"
#pragma warning(pop)



bool GenerateData(int random_seed, int nr_int, bool needs_yes, int ub) // returns if creation was succesful
{
    srand(random_seed);
    int n = nr_int;

    std::mt19937 gen(random_seed);

    std::uniform_int_distribution<> distra(2, ub);
    std::vector<int> a(n);

    if (!needs_yes)
    {
        for (int i = 0; i < n; i++)
        {
            a[i] = distra(gen);
        }
    }
    else
    {
        while (true)
        {
            a.assign(a.size(), 0);
            std::uniform_int_distribution<> distrj(1, n - 1);
            int j = distrj(gen);
            mpz_class P = 1;
            for (int i = 0; i < j; i++)
            {
                a[i] = distra(gen);
                P *= a[i];
            }
            
            // Convert to float
            mpfr::mpreal Pf(P.get_str());
            if (pow(Pf, (1.0/(double) (n - j))) > ub)
            {
                continue;
            }

            //Prime factorization of P
            mpz_class P_copy = P;
            std::vector<int> primes;

            while (P_copy % 2 == 0) {
                primes.push_back(2);
                P_copy = P_copy / 2;
            }

            for (int i = 3; i * i <= P_copy; i = i + 2) {
                while (P_copy % i == 0) {
                    primes.push_back(i);
                    P_copy = P_copy / i;
                }
            }

            if (P_copy > 2) { primes.push_back(P_copy.get_si()); }

            int r = n - j;

			if (primes.size() < r) //if there are not enough prime factors
			{
				continue;
			}

            while (1 < r && r < primes.size())
            {
                int nr_tryouts = 0;
                while (true)
                {
                    nr_tryouts += 1;
                    if (nr_tryouts > 1e5)
                    {
                        std::cout << "This does not seem to work" << std::endl;
                        return false;
                    }
                    std::uniform_int_distribution<> distrk(1, primes.size() - r + 1);
                    int k = distrk(gen);
                    // Generate a vector of indices
                    std::vector<size_t> indices(primes.size());
                    std::iota(indices.begin(), indices.end(), 0);

                    // Shuffle and pick the first k indices
                    std::shuffle(indices.begin(), indices.end(), gen);
                    indices.resize(k);

                    // Sort descending to safely delete by index later
                    std::sort(indices.rbegin(), indices.rend());

                    // Compute product of selected elements
                    mpz_class m = 1;
                    for (size_t i : indices) {
                        m *= primes[i];
                    }
                    mpfr::mpreal mf(m.get_str());
					if (m > ub || pow((Pf/mf), (1.0 / (double)(r - 1))) > ub) {
						continue;
					}

                    a[n - r] = m.get_si();
                    r -= 1;
                    P = P / m;
                    Pf = Pf / mf;
                    // Remove selected elements from original vector
                    for (size_t i : indices) {
                        primes.erase(primes.begin() + i);
                    }

                    break;
                }

            }

			if (r == 1) //if there is only one prime factor left
			{
				a[n - 1] = P.get_si();
			}
			else if (r == primes.size()) //if all prime factors are used
			{
				for (int i = 0; i < r; i++)
				{
					a[n - r + i] = primes[i];
				}
			}
			
            break;
			
        }

    }
    
    mpz_class joint_product = 1;
    for (int i = 0; i < n; i++)
    {
        std::cout << a[i] << std::endl;
		joint_product *= a[i];
    }
    mpfr::mpreal joint_productf(joint_product.get_str());
    mpfr::mpreal root = pow(joint_productf, 0.5);
    mpz_class product = 1;
    mpfr::mpreal productf(product.get_str());
	for (int i = 0; i < n; i++)
	{
		product *= a[i];
        productf *= a[i];
        if (productf > root - 1 && productf < root + 1) // Check if product is close to the square root of the joint product
        {
			std::cout << "Up to index " << i << " the product is the root " << product << std::endl;
            product = 1;
            productf = 1.0;
        }
	}

    //If valid: write output to file
    std::string fileStr = "n_" + std::to_string(n) + "_rep_" + std::to_string(random_seed) + ".dat";
    const char* filename = fileStr.c_str();

    std::ofstream output_file(filename);
    if (output_file.is_open())
    {
        //First line: write number of tasks
        output_file << n << std::endl;

        //Next lines: write revenue, cost and probability for each task, separated by a tab 
        for (int i = 0; i < n; i++)
        {
            output_file << a[i] << std::endl;
        }

        //close file
        output_file.close();

        return true;
    }
    else
    {
        std::cout << "Unable to open output file." << std::endl;
        return false;
    }
}

int main()
{
    mpfr::mpreal::set_default_prec(256);
    for (int n = 5; n <= 100; n += 5) // Issues from 120 onwards
    {
        int ub = 10;
        for (int i = 0; i < ub; i++)
        {
            if (!GenerateData(i, n, false, 100)) ub++;
        }
    }
}