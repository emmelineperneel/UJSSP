#include "Data.h"
#include<algorithm>
#include<iostream>
#include<fstream>
#include<vector>
#include<chrono>

#define PRINT_INFO false
#define SPEEDUPS true

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cout << "Usage: " << argv[0] << " < filename > " << std::endl;
		return 0;
	}

	Data::ReadData(argv[1]);
	Data::SortData();
	Data::Print_Data();
	
	//Start timer
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

	//Initialize values
	double ub_R = 0;
	double prob = 1.0;
	for (int i = 0; i < Data::n; i++)
	{
		prob *= Data::p[i];
		ub_R += Data::r[i] * prob;
	}
	
	double lb_R = 0;

#if PRINT_INFO
	std::cout << "\nUpper Bound R: " << ub_R << std::endl;
	std::cout << "Lower Bound R: " << lb_R << std::endl;
#endif

	std::vector<std::vector<bool>> sets;
	std::vector<double> intercept;
	std::vector<double> slope;
	std::vector<double> optimality_limit;

	std::vector<bool> empty(Data::n, false);
	sets.push_back(empty);
	intercept.push_back(0.0);
	slope.push_back(1.0);
	optimality_limit.push_back(std::numeric_limits<double>::max());


	//Algorithm
	for (int j = 0; j < Data::n; j++)
	{

		//Add extra subsets
		std::vector<std::vector<bool>> new_sets;
		std::vector<double> new_intercept;
		std::vector<double> new_slope;
		for (int s = 0; s < sets.size(); s++)
		{
#if SPEEDUPS
			if (optimality_limit[s] < Data::p[j]*Data::r[j])
			{
				continue; //Never optimal to add this job, as revenue will be too high
			}
#endif

			std::vector<bool> new_set = sets[s];
			new_set[j] = true;
			new_sets.push_back(new_set);
			new_intercept.push_back(intercept[s] + slope[s] * Data::r[j] * Data::p[j] - Data::c[j]);
			new_slope.push_back(slope[s] * Data::p[j]);
		}

		//Update upperbound
		ub_R = ub_R / Data::p[j] - Data::r[j];

#if PRINT_INFO
		std::cout << "\n\nSTEP " << j << std::endl;
		std::cout << "\n\nNew sets:\n" << std::endl;
		for (int s = 0; s < new_sets.size(); s++)
		{
			std::cout << "[";
			for (int i = 0; i < Data::n - 1; i++)
			{
				std::cout << new_sets[s][i] << ",";
			}
			std::cout << new_sets[s][Data::n - 1] << "]\t intercept: " << new_intercept[s] << "\t slope: " << new_slope[s] << std::endl;
			std::cout << std::endl;
		}
		std::cout << "Upper Bound R: " << ub_R << std::endl;
		std::cout << "Lower Bound R: " << lb_R << std::endl;
#endif

		//Eliminate dominated subsets based on upperbound 
		bool dominated = true;
		while (dominated && sets.size() >= 2)
		{
			if (optimality_limit[sets.size() - 2] > ub_R)
			{
				sets.erase(sets.begin() + sets.size() - 1);
				intercept.erase(intercept.begin() + intercept.size() - 1);
				slope.erase(slope.begin() + slope.size() - 1);
				optimality_limit.erase(optimality_limit.begin() + optimality_limit.size() - 1);
			}
			else
			{
				dominated = false;
			}
		}

		//Consider new sets
		for (int s = 0; s < new_sets.size(); s++)
		{
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
			

			double x_r;
			double x_l;
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
			}
			if (idx_l == -1 && x_r <= lb_R)
			{
				continue; //new subset is dominated
			}
			if (idx_r == sets.size() && x_l >= ub_R)
			{
				continue; //new subset is dominated
			}

			//Find smallest index for which the intersection is the highest
			while (idx_l >= 1)
			{
				double new_x_l = (intercept[idx_l - 1] - new_intercept[s]) / (new_slope[s] - slope[idx_l - 1]);
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
				double new_x_r = (new_intercept[s] - intercept[idx_r + 1]) / (slope[idx_r + 1] - new_slope[s]);
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
			}
			if (idx_l == 0 && x_l <= lb_R)
			{
				sets.erase(sets.begin());
				intercept.erase(intercept.begin());
				slope.erase(slope.begin());
				optimality_limit.erase(optimality_limit.begin());
				idx_l = -1;
				idx_r -= 1;
			}
			if (idx_r == sets.size() - 1 && x_r >= ub_R)
			{
				sets.erase(sets.begin() + sets.size() - 1);
				intercept.erase(intercept.begin() + intercept.size() - 1);
				slope.erase(slope.begin() + slope.size() - 1);
				optimality_limit.erase(optimality_limit.begin() + optimality_limit.size() - 1);
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

#if PRINT_INFO
		std::cout << "\nOptimal sets after step " << j << ":\n" << std::endl;
		for (int s = 0; s < sets.size(); s++)
		{
			std::cout << "[";
			for (int i = 0; i < Data::n - 1; i++)
			{
				std::cout << sets[s][i] << ",";
			}
			std::cout << sets[s][Data::n - 1] << "]\t intercept: " << intercept[s] << "\t slope: " << slope[s] << "\t optimal up to: " << optimality_limit[s] << std::endl;
			std::cout << std::endl;
		}
#endif
	}

	//End timer
	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

	std::cout << "\nOptimal set:\n" << std::endl;
	for (int s = 0; s < sets.size(); s++)
	{
		std::cout << "[";
		for (int i = 0; i < Data::n - 1; i++)
		{
			std::cout << sets[s][i] << ",";
		}
		std::cout << sets[s][Data::n - 1] << "]\t with objective value: " << intercept[s] << std::endl;
		std::cout << std::endl;
	}

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
	size_t lastSlash = baseFilename.find_last_of("/");

	if (lastSlash != std::string::npos) {
		// There is a directory in the path
		std::string folder = baseFilename.substr(0, lastSlash); 
		std::string filenameOnly = baseFilename.substr(lastSlash + 1);

		// Go one level up from folder and then into "output/"
		outputFilename = folder + "/output/" + filenameOnly + ".out";
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

	// Write to output file
	outFile << intercept[0] << std::endl;
	outFile << std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count() << std::endl;
	outFile << Data::n << std::endl;
	for (int i = 0; i < Data::n; i++)
	{
		outFile << sets[0][i] << "\t" << Data::r[i] << "\t" << Data::c[i] << "\t" << Data::p[i] << std::endl;
	}

	return 0;
}