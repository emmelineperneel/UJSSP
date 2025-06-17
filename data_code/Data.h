#pragma once
#pragma once
#include <vector>

class Data
{
public:
    static int n;

    static std::vector<double> p;
    static std::vector<int> c;
    static std::vector<int> r;

    static std::vector<double> ratio;

    static void GenerateData(int random_seed, int nr_tests, int prob_gen_method);
    static void WriteData(const char* filename);
    static void ReadData(const char* filename);
    static void SortData();
    static void Print_Data();
};