#include "Data.h"
#include <iostream> 
#include <random>
#include <math.h> 
#include <fstream> 
#include <sstream>
#include <algorithm>

int Data::n;

std::vector<double> Data::p;
std::vector<int> Data::c;
std::vector<int> Data::r;

std::vector<double> Data::ratio;



void Data::GenerateData(int random_seed, int nr_tests, int prob_gen_method)
{
    srand(random_seed);
    Data::n = nr_tests;

    std::mt19937 gen(random_seed);

    //Generate revenues
    std::uniform_int_distribution<> distrr(50, 500);
    std::vector<int> revenues(Data::n);
    
    for (int i = 0; i < Data::n; i++)
    {
        revenues[i] = distrr(gen);
    }

    //Generate probabilities 
    double joint_prob = 1.0;
	std::vector<double> probabilities(Data::n);
	if (prob_gen_method == 0) //uniform distribution
	{
		std::uniform_real_distribution<> distrp(0.01, 0.99);
		for (int i = 0; i < Data::n; i++)
		{
			probabilities[i] = distrp(gen);
            while (std::floor(probabilities[i] * revenues[i]) < 1)
            {
                probabilities[i] = distrp(gen);
            }
			joint_prob *= probabilities[i];
		}
	}
	else 
	{
        if (prob_gen_method == 1)
        {
            std::uniform_real_distribution<> distrjp(0.01, 0.10);
            joint_prob = distrjp(gen);
        }
        else if (prob_gen_method == 2)
        {
            std::uniform_real_distribution<> distrjp(0.10, 0.40);
            joint_prob = distrjp(gen);
        }
        else if (prob_gen_method == 3)
        {
            std::uniform_real_distribution<> distrjp(0.40, 0.90);
            joint_prob = distrjp(gen);
        }
        else
        {
			std::cout << "Problem with probability generation method!" << std::endl;
        }
        std::uniform_int_distribution<> distrw(1, 1000);
        std::vector<int> weights(Data::n);
        int total_weight = 0;
        for (int i = 0; i < Data::n; i++)
        {
            weights[i] = distrw(gen);
            total_weight += weights[i];
        }
        for (int i = 0; i < Data::n; i++)
        {
            probabilities[i] = exp(log(joint_prob) * ((double)weights[i]) / ((double)total_weight));
        }
	}

    //Generate costs
    std::vector<int> costs(Data::n);
    for (int i = 0; i < Data::n; i++)
    {
        int lowerbound = std::ceil(joint_prob * revenues[i]);
        int upperbound = std::floor(probabilities[i] * revenues[i]);
        std::uniform_int_distribution<> distrc(lowerbound, upperbound);
        costs[i] = distrc(gen);
    }

    //Calculate ratios
    std::vector<double> ratios(Data::n);
    for (int i = 0; i < Data::n; i++)
    {
        ratios[i] = ((double)revenues[i] * probabilities[i]) / (1 - probabilities[i]);
    }

    Data::p = probabilities;
    Data::c = costs;
    Data::r = revenues;
    Data::ratio = ratios;
}

void Data::SortData()
{
    //sort by ratios
    std::vector<int> indices(n);
    for (int i = 0; i < Data::n; i++)
    {
        indices[i] = i;
    }
    std::stable_sort(indices.begin(), indices.end(), [&](int a, int b) {
        return (Data::ratio[a] > Data::ratio[b]);
        });

    //Copy values
    std::vector<double> ratios(Data::n);
    std::vector<double> probabilities(Data::n);
    std::vector<int> costs(Data::n);
    std::vector<int> revenues(Data::n);
    for (int i = 0; i < Data::n; i++) {
        probabilities[i] = Data::p[i];
        costs[i] = Data::c[i];
        revenues[i] = Data::r[i];
        ratios[i] = Data::ratio[i];
    }

    //Reshuffle tests
    for (int i = 0; i < Data::n; i++) {
        Data::p[i] = probabilities[indices[i]];
        Data::r[i] = revenues[indices[i]];
        Data::c[i] = costs[indices[i]];
        Data::ratio[i] = ratios[indices[i]];
    }
}

void Data::WriteData(const char* filename)
{
    //Open file
    std::ofstream output_file(filename);
    if (output_file.is_open())
    {
        //First line: write number of tasks
        output_file << Data::n << std::endl;

        //Next lines: write revenue, cost and probability for each task, separated by a tab 
        for (int i = 0; i < Data::n; i++)
        {
            output_file << Data::r[i] << "\t" << Data::c[i] << "\t" << Data::p[i] << std::endl;
        }

        //close file
        output_file.close();
    }
    else
    {
        std::cout << "Unable to open output file." << std::endl;
    }
}

void Data::ReadData(const char* filename)
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
                //we get nr of tests
                int nr_tests;
                while (my_stream >> nr_tests)
                {
                    Data::n = nr_tests;
                    Data::p.resize(Data::n);
                    Data::r.resize(Data::n);
                    Data::c.resize(Data::n);
                    Data::ratio.resize(Data::n);
                }
            }
            else
            {
                //Everything else: index of task (+1) followed by cost and then probability
                int revenue;
                int cost;
                double prob;
                my_stream >> revenue;
                my_stream >> cost;
                my_stream >> prob;
                Data::r[line_nr - 1] = revenue;
                Data::c[line_nr - 1] = cost;
                Data::p[line_nr - 1] = prob;
                Data::ratio[line_nr - 1] = ((double)Data::r[line_nr - 1] * Data::p[line_nr - 1]) / (1 - Data::p[line_nr - 1]);
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

void Data::Print_Data()
{
    std::cout << "\n \n \t Nr of Tests/Tasks: " << Data::n << std::endl;
    for (int i = 0; i < Data::n; i++)
    {
        std::cout << "Task " << i << " probability = " << p[i] << " cost = " << c[i] << " revenue = " << r[i];
        std::cout << "\tratio = " << ratio[i] << std::endl;
    }
}