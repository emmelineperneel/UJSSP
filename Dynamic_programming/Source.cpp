#include "Data.h"
#include<iostream>
#include<fstream>
#include<vector>
#include<chrono>


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

	int c_max = 0;
	for (int i = 0; i < Data::n; i++)
	{
		c_max += Data::c[i];
	}

	//Start timer
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

	std::vector<std::vector<double>> rev(2, std::vector<double>(c_max + 1)); //Expected revenues; only store last 2 to save memory
	std::vector<std::vector<std::vector<bool>>> solutions(2, std::vector<std::vector<bool>>(c_max + 1, std::vector<bool>(Data::n, false))); //Corresponding solution vectors
	//Initialize for n
	for (int b = 0; b < Data::c[Data::n - 1]; b++)
	{
		rev[0][b] = 0;
	}
	for (int b = Data::c[Data::n - 1]; b <= c_max; b++)
	{
		rev[0][b] = (Data::p[Data::n - 1] * ((double) Data::r[Data::n - 1]));
		solutions[0][b][Data::n - 1] = true;
	}
	//Dynamic programming
	int index = 0;
	int old_index = 1;
	for (int n = Data::n - 2; n >= 0; n--)
	{
		//Find indices
		if (old_index == 0) { old_index = 1; index = 0; }
		else { old_index = 0; index = 1; }

		for (int b = 0; b <= c_max; b++)
		{
			rev[index][b] = rev[old_index][b];
			double including = 0;
			if (Data::c[n] <= b) // Try including job if possible
			{
				including = Data::p[n] * (Data::r[n] + rev[old_index][b - Data::c[n]]);
			}
			if (including > rev[index][b]) // If including is better, do it
			{
				rev[index][b] = including;
				solutions[index][b] = solutions[old_index][b - Data::c[n]];
				solutions[index][b][n] = true;
			}
			else
			{
				solutions[index][b] = solutions[old_index][b];
			}
		}
	}

	//Find optimal solution, by looking at all possible budgets
	double optimal_profit = 0;
	int optimal_budget = 0;
	for (int b = 0; b < c_max + 1; b++)
	{
		if (rev[index][b] - b > optimal_profit)
		{
			optimal_profit = rev[index][b] - b;
			optimal_budget = b;
		}
	}

	//Stop timer
	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

	//Print solution
	std::cout << "Optimal solution = " << optimal_profit << std::endl;
	std::cout << "[ ";
	for (int i = 0; i < Data::n - 1; i++)
	{
		if (solutions[index][optimal_budget][i]) std::cout << " 1 ,";
		else std::cout << " 0 ,";
	}
	if (!solutions[index][optimal_budget][Data::n - 1]) std::cout << " 0 ]" << std::endl;
	else std::cout << " 1 ]" << std::endl;

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
		outputFilename = folder + "\\output\\" + filenameOnly + ".out";
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
	outFile << optimal_profit << std::endl;
	outFile << std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count() << std::endl;
	outFile << Data::n << std::endl;
	for (int i = 0; i < Data::n; i++)
	{
		outFile << solutions[index][optimal_budget][i] << "\t" << Data::r[i] << "\t" << Data::c[i] << "\t" << Data::p[i] << std::endl;
	}

	return 0;
}