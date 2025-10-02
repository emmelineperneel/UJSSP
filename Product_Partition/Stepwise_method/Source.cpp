#define MPFR_USE_NO_MACRO
#define MPFR_USE_INTMAX_T

#include <iostream> 
#include <random>
#include <math.h> 
#include <fstream> 
#include <sstream>
#include <algorithm>
#include <numeric>
#include<chrono>

#pragma warning(push)
#pragma warning(disable: 4146)
#include <gmpxx.h>
#include <mpfr.h>
#include "mpreal.h"
#pragma warning(pop)

#define PRINT_INFO false
#define SORTING_METHOD 1 // 1 is from small to large, 2 is from large to small and 3 is random

void ReadData(int& n, std::vector<int>& a, const char* filename)
{
    //Open file
    std::ifstream input_file(filename);
    if (input_file.is_open())
    {
        std::string line;
        int line_nr = 0;
        while (std::getline(input_file, line))
        {
            std::istringstream my_stream(line);
            if (line_nr == 0)
            {
                //we get nr of integers
                int nr_int;
                while (my_stream >> nr_int)
                {
                    n = nr_int;
                    a.resize(n);
                }
            }
            else
            {
                //Everything else: integer itself
                int number;
                my_stream >> number;
				a[line_nr - 1] = number;
            }
            std::cout << line << std::endl;
            line_nr++;
        }
        //close file
        input_file.close();
    }
    else
    {
        std::cout << "Unable to open input file." << std::endl;
    }
}

void SortData(std::vector<int>& a, int method) //Method: 1 is from small to large, 2 is from large to small and 3 is random
{
    if (method < 1 || method > 3)
    {
        std::cout << "Invalid method for sorting" << std::endl;
        return;
    }
	if (method == 1)
	{
		std::sort(a.begin(), a.end());
	}
	else if (method == 2)
	{
		std::sort(a.rbegin(), a.rend());
	}
    else if (method == 3)
    {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(a.begin(), a.end(), g);
    }
}

void PrintData(std::vector<int> a)
{
    std::cout << "Data:" << std::endl;
    for (const auto& value : a)
    {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[])
{
	mpfr::mpreal::set_default_prec(512);
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " < filename > " << std::endl;
        return 0;
    }

    int n = 0;
	std::vector<int> a; 
    ReadData(n, a, argv[1]);
    SortData(a, SORTING_METHOD);
    PrintData(a);

    //Start algorithm
	//Start timer
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

	//Initialize values
	mpz_class P_remaining = 1;
	for (int i = 0; i < n; i++)
	{
		P_remaining *= a[i];
	}
	mpfr::mpreal joint_productf(P_remaining.get_str());
	mpfr::mpreal root = pow(joint_productf, 0.5);
	
	//Update bounds on remaining used probability
	mpfr::mpreal P_ub = root; // Upper bound on remaining probability
	mpfr::mpreal P_lb = root; // Lower bound on remaining probability

#if PRINT_INFO
	std::cout << "\nProduct of all integers: " << P_remaining << std::endl;
	std::cout << "Root of product: " << root << std::endl;
#endif

	std::vector<std::vector<bool>> sets;
	std::vector<mpfr::mpreal> intercept;
	std::vector<mpfr::mpreal> slope;
	std::vector<mpfr::mpreal> optimality_limit;

	std::vector<bool> empty(n, false);
	sets.push_back(empty);
	intercept.push_back(0.0);
	slope.push_back(-root);
	optimality_limit.push_back(std::numeric_limits<double>::max());

	int nr_sets_dominated = 0;

	//Algorithm
	bool optimal_solution_found = false;
	int optimal_new_set = 0; // Index of the optimal new set, if found
	bool time_limit_reached = false;
	int nr_sets = 0;
	for (int j = 0; j < n; j++)
	{

		//Add extra subsets
		std::vector<std::vector<bool>> new_sets;
		std::vector<mpfr::mpreal> new_intercept;
		std::vector<mpfr::mpreal> new_slope;
		mpfr::mpreal ma = a[j]; // promote to mpreal
		mpfr::mpreal loga = log(ma);
		for (int s = 0; s < sets.size(); s++)
		{
			nr_sets++;
			std::vector<bool> new_set = sets[s];
			new_set[j] = true;
			mpfr::mpreal product = exp(- (intercept[s] - loga));
			if (product < (root + 1e-10) && product > (root - 1e-10))
			{
				//This set is optimal!
				optimal_solution_found = true;
				optimal_new_set = new_sets.size(); // Store the index of the optimal new set
			}
			if (product > (root + 1e-5))
			{
				continue; // skip this set
			}
			mpfr::mpreal joint_productf(P_remaining.get_str());
			if (product * joint_productf < (root - 1e-5))
			{
				continue; // skip this set
			}
			new_sets.push_back(new_set);
			new_intercept.push_back(intercept[s] - loga);
			new_slope.push_back(slope[s] / ma);
			nr_sets++;
		}

		//Update upperbound on remaining probability
		P_remaining /= a[j];
		mpfr::mpreal P_remaining_f(P_remaining.get_str());
		//Update bounds on remaining used probability
		bool ub_went_down = false;
		mpfr::mpreal old_P_ub = P_ub;
		P_ub = min(root / exp(-intercept[0]), P_remaining_f); // Upper bound on remaining probability
		if (P_ub < old_P_ub)
		{
			ub_went_down = true;
		}
		bool lb_went_up = false;
		mpfr::mpreal old_P_lb = P_lb;
		P_lb = max(1.0, root / exp(-intercept[intercept.size() - 1] + loga)); // Lower bound on remaining probability
		if (P_lb > old_P_lb)
		{
			lb_went_up = true;
		}
		mpfr::mpreal lb_product = 1.0 / P_ub;
		mpfr::mpreal ub_product = 1.0 / P_lb;

#if PRINT_INFO
		std::cout << "\n\nSTEP " << j << std::endl;
		std::cout << "\n\nNew sets:\n" << std::endl;
		for (int s = 0; s < new_sets.size(); s++)
		{
			std::cout << "[";
			for (int i = 0; i < n - 1; i++)
			{
				std::cout << new_sets[s][i] << ",";
			}
			std::cout << new_sets[s][n - 1] << "]\t intercept: " << new_intercept[s] << "\t slope: " << new_slope[s] << "\t product: " << exp(-(new_intercept[s])) <<  std::endl;
			std::cout << std::endl;
		}
		std::cout << "Upper Bound P: " << P_ub << " so lowerbound product: " << lb_product << std::endl;
		std::cout << "Lower Bound P: " << P_lb << " so upperbound product: " << ub_product << std::endl;
		std::cout << "Remaining product: " << P_remaining << std::endl;
#endif

		//Eliminate dominated subsets
		// If the optimal solution is found we only need this set
		if (optimal_solution_found)
		{
			sets.clear();
			intercept.clear();
			slope.clear();
			optimality_limit.clear();
			sets.push_back(new_sets[optimal_new_set]);
			intercept.push_back(new_intercept[optimal_new_set]);
			slope.push_back(new_slope[optimal_new_set]);
			optimality_limit.push_back(std::numeric_limits<double>::max());
			break;
		}
		
		//Otherwise: first check if existing sets can still form the root
		for (int s = 0; s < sets.size(); s++)
		{
			mpfr::mpreal product = exp(-intercept[s]);
			mpfr::mpreal joint_productf(P_remaining.get_str());
			if (product * joint_productf < root - 1e-5)
			{
				sets.erase(sets.begin());
				intercept.erase(intercept.begin());
				slope.erase(slope.begin());
				optimality_limit.erase(optimality_limit.begin());
			}
			else
			{
				break;
			}
		}
		
		//Then eliminate based on bounds
		if (ub_went_down)
		{
			//Lowerbound on joint probability of success went up
			bool dominated = true;
			while (dominated && sets.size() >= 2)
			{

				if (optimality_limit[0] < lb_product)
				{
					sets.erase(sets.begin());
					intercept.erase(intercept.begin());
					slope.erase(slope.begin());
					optimality_limit.erase(optimality_limit.begin());
					nr_sets_dominated++;
				}
				else
				{
					dominated = false;
				}
			}
		}
		if (lb_went_up)
		{
			//Upperbound on joint probability of success went down
			bool dominated = true;
			while (dominated && sets.size() >= 2)
			{
				if (optimality_limit[optimality_limit.size() - 2] > ub_product)
				{
					sets.erase(sets.end() - 1);
					intercept.erase(intercept.end() - 1);
					slope.erase(slope.end() - 1);
					optimality_limit.erase(optimality_limit.end() - 1);
					nr_sets_dominated++;	
				}
				else
				{
					dominated = false;
				}
			}
		}
		

		//Consider new sets
		for (int s = 0; s < new_sets.size(); s++)
		{
			std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
			double nr_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();
			if (nr_seconds > 1200)
			{
				time_limit_reached = true;
			}

			// Search for biggest existing slope for which the new slope ≤ existing_slope
			auto lower = std::lower_bound(slope.begin(), slope.end(), new_slope[s]);
			int idx_r = std::distance(slope.begin(), lower);
			int idx_l = idx_r - 1;
			if (idx_r != slope.size())
			{
				if (slope[idx_r] == new_slope[s]) //In case of equal slopes: keep the one with the highest intercept
				{
					if (intercept[idx_r] >= new_intercept[s])
					{
						continue;
					}
					else
					{
						idx_r += 1;
					}
				}
			}


			mpfr::mpreal x_r;
			mpfr::mpreal x_l;
			if (idx_r == slope.size())
			{
				x_r = std::numeric_limits<double>::max();
			}
			else
			{
				x_r = (new_intercept[s] - intercept[idx_r]) / (slope[idx_r] - new_slope[s]);
			}
			if (idx_l == -1)
			{
				x_l = std::numeric_limits<double>::lowest();
			}
			else
			{
				x_l = (intercept[idx_l] - new_intercept[s]) / (new_slope[s] - slope[idx_l]);
			}

			if (x_r <= x_l)
			{
				continue; // new subset is dominated
				nr_sets_dominated++;
			}
			if (idx_l == -1 && x_r <= lb_product)
			{
				continue; //new subset is dominated
				nr_sets_dominated++;
			}
			if (idx_r == sets.size() && x_l >= ub_product)
			{
				continue; //new subset is dominated
				nr_sets_dominated++;
			}

			//Find smallest index for which the intersection is the highest
			while (idx_l >= 1)
			{
				mpfr::mpreal new_x_l = (intercept[idx_l - 1] - new_intercept[s]) / (new_slope[s] - slope[idx_l - 1]);
				if (new_x_l >= x_l)
				{
					x_l = new_x_l;
					idx_l -= 1;
				}
				else
				{
					break;
				}
			}

			//Find largest index for which the intersection is the lowest
			while (idx_r <= (static_cast<int>(slope.size()) - 2))
			{
				mpfr::mpreal new_x_r = (new_intercept[s] - intercept[idx_r + 1]) / (slope[idx_r + 1] - new_slope[s]);
				if (new_x_r <= x_r)
				{
					x_r = new_x_r;
					idx_r += 1;
				}
				else
				{
					break;
				}
			}

			//Remove dominated subsets
			if (idx_r - idx_l > 1)
			{
				sets.erase(sets.begin() + (idx_l + 1), sets.begin() + idx_r);
				intercept.erase(intercept.begin() + (idx_l + 1), intercept.begin() + idx_r);
				slope.erase(slope.begin() + (idx_l + 1), slope.begin() + idx_r);
				optimality_limit.erase(optimality_limit.begin() + (idx_l + 1), optimality_limit.begin() + idx_r);
				idx_r -= (idx_r - idx_l - 1);
				nr_sets_dominated += (idx_r - idx_l - 1);
			}
			if (idx_l == 0 && x_l <= lb_product)
			{
				sets.erase(sets.begin());
				intercept.erase(intercept.begin());
				slope.erase(slope.begin());
				optimality_limit.erase(optimality_limit.begin());
				idx_l = -1;
				idx_r -= 1;
				nr_sets_dominated++;
			}
			if (idx_r == sets.size() - 1 && x_r >= ub_product)
			{
				sets.erase(sets.begin() + sets.size() - 1);
				intercept.erase(intercept.begin() + intercept.size() - 1);
				slope.erase(slope.begin() + slope.size() - 1);
				optimality_limit.erase(optimality_limit.begin() + optimality_limit.size() - 1);
				nr_sets_dominated++;
			}
			if (idx_r == sets.size())
			{
				x_r = std::numeric_limits<double>::max();
			}


			//Add new subset
			sets.insert(sets.begin() + (idx_l + 1), new_sets[s]);
			intercept.insert(intercept.begin() + (idx_l + 1), new_intercept[s]);
			slope.insert(slope.begin() + (idx_l + 1), new_slope[s]);
			optimality_limit.insert(optimality_limit.begin() + (idx_l + 1), x_r);
			if (idx_l >= 0)
			{
				optimality_limit[idx_l] = x_l;
			}

		}

		if (time_limit_reached)
		{
			break;
		}

#if PRINT_INFO
		std::cout << "\nOptimal sets after step " << j << ":\n" << std::endl;
		for (int s = 0; s < sets.size(); s++)
		{
			std::cout << "[";
			for (int i = 0; i < n - 1; i++)
			{
				std::cout << sets[s][i] << ",";
			}
			std::cout << sets[s][n - 1] << "]\t intercept: " << intercept[s] << "\t slope: " << slope[s] << "\t optimal up to: " << optimality_limit[s] << std::endl;
			std::cout << std::endl;
		}
		std::cout << "Number of dominated sets so far: " << nr_sets_dominated << std::endl;
#endif
	}

	//End timer
	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

	//Write results to file
	std::string inputFilename = argv[1];
	std::string baseFilename;
	std::string outputFilename;

	// Check if it ends with ".dat"
	if (inputFilename.size() >= 4 && inputFilename.substr(inputFilename.size() - 4) == ".dat") {
		// Strip .dat extension
		baseFilename = inputFilename.substr(0, inputFilename.size() - 4);
	}
	else {
		std::cerr << "Input file does not have a .dat extension." << std::endl;
		return 1;
	}

	// Find last slash
	size_t lastSlash = baseFilename.find_last_of("\\");

	if (lastSlash != std::string::npos) {
		// There is a directory in the path
		std::string folder = baseFilename.substr(0, lastSlash);
		std::string filenameOnly = baseFilename.substr(lastSlash + 1);

		// Go one level up from folder and then into "output/"
		std::string sorting_method_str = std::to_string(SORTING_METHOD);
		outputFilename = folder + "\\output\\" + filenameOnly + "_m_" + sorting_method_str + ".out";
	}
	else {
		// No slashes, just replace .dat with .out
		outputFilename = baseFilename + ".out";
	}


	// Open output file for writing
	std::ofstream outFile(outputFilename);
	if (!outFile) {
		std::cerr << "Could not open file for writing: " << outputFilename << std::endl;
		return 1;
	}

	if (time_limit_reached)
	{
		std::cout << "Time limit reached. Stopping algorithm." << std::endl;
		outFile << std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count() << std::endl;
		return 0;
	}

	//Calculate objective value
	mpz_class product = 1;
	bool yes_answer = false;
	for (int i = 0; i < n; i++)
	{
		if (sets[0][i])
		{
			product *= a[i];
		}
	}

	//Check if the root is equal to the product
	mpfr::mpreal productf(product.get_str());
	if (abs(productf - root) < 1e-10)
	{
		yes_answer = true;
	}

	std::cout << "\nOptimal set:\n" << std::endl;
	for (int s = 0; s < sets.size(); s++)
	{
		std::cout << "[";
		for (int i = 0; i < n - 1; i++)
		{
			std::cout << sets[s][i] << ",";
		}
		std::cout << sets[s][n - 1] << "]\t with objective value: " << product << std::endl;
		std::cout << "Number of sets considered: " << nr_sets << " out of " << (std::pow(2, n)) << std::endl;
		std::cout << "Root was equal to: " << root << std::endl;
		if (yes_answer)
			std::cout << "Yes, the root is equal to the product." << std::endl;
		else
			std::cout << "No, the root is not equal to the product." << std::endl;
		std::cout << std::endl;
	}

	// Write to output file
	outFile << std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count() << std::endl;
	outFile << product << std::endl;
	outFile << root << std::endl;
	outFile << yes_answer << std::endl;
	outFile << n << std::endl;
	outFile << nr_sets << std::endl;
	for (int i = 0; i < n; i++)
	{
		outFile << sets[0][i] << "\t" << a[i] << std::endl;
	}


}