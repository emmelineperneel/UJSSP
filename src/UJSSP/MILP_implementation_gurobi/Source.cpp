#include "Data.h"
#include<algorithm>
#include<iostream>
#include<sstream>
#include<fstream>
#include "gurobi_c++.h"

int main(int argc, char* argv[])
{
	//For specific data file:
	
	if (argc != 2)
	{
		std::cout << "Usage: " << argv[0] << " < filename > " << std::endl;
		return 0;
	}

	Data::ReadData(argv[1]);
	Data::SortData();
	Data::Print_Data();

	//Initialization
	double best_solution = 0.0;
	bool optimal_solution_found = false;
	std::vector<bool> best_solution_vector(Data::n, false);

	//Find optimal solution using gurobi
	GRBEnv env = GRBEnv();
	env.set(GRB_IntParam_OutputFlag, 0);
	GRBModel Model = GRBModel(env);
	Model.set(GRB_StringAttr_ModelName, "Simultaneous selection problem ordened");

	//Set time limit: 20 minutes
	Model.set(GRB_DoubleParam_TimeLimit, 1200.0);

	//Create variables
	//x-variables, indicating if a certain job is selected
	//p-variables, indicating the probability of successfully executing a job if selected
	std::vector<GRBVar> x(Data::n);
	std::vector<GRBVar> P(Data::n);
	for (int i = 0; i < Data::n; i++)
	{
		std::ostringstream vnamex;
		vnamex << "x_" << i;
		x[i] = Model.addVar(0.0, 1.0, -Data::c[i], 'B', vnamex.str());

		std::ostringstream vnameP;
		vnameP << "P_" << i;
		P[i] = Model.addVar(0.0, 1.0, Data::r[i], 'C', vnameP.str());
	}
	Model.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE);

	std::vector<GRBConstr> Link_P_x(Data::n);
	for (int i = 0; i < Data::n; i++)
	{
		GRBLinExpr link_P_x = P[i] - x[i] * Data::p[i]; //nr of tasks needs to become lowerbound
		std::ostringstream cnamelinkPx;
		cnamelinkPx << "Link_P_x_" << i;
		Link_P_x[i] = Model.addConstr(link_P_x <= 0, cnamelinkPx.str());
	}

	std::vector<std::vector<GRBConstr>> Link_P_P(Data::n, std::vector<GRBConstr>(Data::n));
	for (int i = 0; i < Data::n; i++)
	{
		for (int j = i + 1; j < Data::n; j++)
		{
			int index_i = i;
			int index_j = j;
			GRBLinExpr link_P_P = P[index_j] - P[index_i] * Data::p[index_j] + Data::p[index_j] * x[index_i]; //nr of tasks needs to become lowerbound
			std::ostringstream cnamelinkPP;
			cnamelinkPP << "Link_P_" << index_i << "_P_" << index_j;
			Link_P_P[index_i][index_j] = Model.addConstr(link_P_P <= Data::p[index_j], cnamelinkPP.str());
		}
	}

	//Optimize
	Model.update();
	Model.optimize();

	//Get solution status
	bool time_limit_reached = false;
	if (Model.get(GRB_IntAttr_Status) == GRB_OPTIMAL) optimal_solution_found = true;
	if (Model.get(GRB_IntAttr_Status) == GRB_TIME_LIMIT) time_limit_reached = true;
	if ((time_limit_reached && optimal_solution_found) || (!time_limit_reached && !optimal_solution_found))
	{
		std::cout << "Something went wrong when solving the model." << std::endl;
	}

	//Get best found solution
	best_solution = Model.getObjective().getValue();

	//Get solution LP-relaxation
	GRBModel relaxedModel = Model.relax();
	relaxedModel.optimize();
	double LP_gap;
	if (relaxedModel.get(GRB_IntAttr_Status) == GRB_OPTIMAL) {
		double LP_relaxation = relaxedModel.get(GRB_DoubleAttr_ObjVal);
		//Calculate LP gap : how much percent of the optimal solution is the LPrelaxation
		LP_gap = (LP_relaxation - best_solution) / best_solution;
	}
	else {
		std::cerr << "Optimization LP-relaxation was not successful. Status: "
			<< relaxedModel.get(GRB_IntAttr_Status) << std::endl;
	}

	//Get final gap
	double bestBound = Model.get(GRB_DoubleAttr_ObjBound);
	double final_gap = (bestBound - best_solution) / best_solution;

	//Get time needed to solve
	double time_needed = Model.get(GRB_DoubleAttr_Runtime);

	//Get x-values
	for (int i = 0; i < Data::n; i++)
	{
		if (x[i].get(GRB_DoubleAttr_X) > (1 - 1e-5))
		{
			best_solution_vector[i] = true;
		}
		else if (x[i].get(GRB_DoubleAttr_X) < 1e-5)
		{
			best_solution_vector[i] = false;
		}
		else
		{
			best_solution_vector[i] = 0;
			std::cout << "[ERROR] X-value with non-binary outcome." << std::endl;
		}
	}


	//Print outcome 
	std::cout << "[";
	for (int i = 0; i < Data::n - 1; i++)
	{
		std::cout << best_solution_vector[i] << ",";
	}
	std::cout << best_solution_vector[Data::n - 1] << "] : solution with profit " << best_solution << std::endl;

	
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
	outFile << best_solution << std::endl;
	outFile << time_needed << std::endl;
	outFile << LP_gap << std::endl;
	outFile << final_gap << std::endl;
	outFile << Data::n << std::endl;
	for (int i = 0; i < Data::n; i++)
	{
		outFile << best_solution_vector[i] << "\t" << Data::r[i] << "\t" << Data::c[i] << "\t" << Data::p[i] << std::endl;
	}

	return 0;
	
}