# Unreliable Job Selection and Sequencing Problem (UJSSP)

This repository contains all the code associated with **“The Unreliable Job Selection and Sequencing Problem”**.

📄 **Paper**: See ..  
📦 **Zenodo Archive (version-specific DOI)**: [10.5281/zenodo.15684073](https://doi.org/10.5281/zenodo.15684073)  
📂 **GitHub Repository**: [https://github.com/emmelineperneel/UJSSP](https://github.com/emmelineperneel/UJSSP)
📜 **License**: [MIT License](./LICENSE)

---

## Citation

If you use this code, please cite:

```
@misc{perneel2025UJSSP,
  author = {Emmeline Perneel},
  title = {Unreliable Job Selection and Sequencing Problem — Code and Data},
  year = {2025},
  publisher = {Zenodo},
  doi = {10.5281/zenodo.15684075},
  url = {https://github.com/emmelineperneel/UJSSP}
}
```

and the corresponding paper: 

```
@article{agnetis2025UJSSP,
    title={The Unreliable Job Selection and Sequencing Problem},
    author={Agnetis, Alessandro and Leus, Roel and Perneel, Emmeline and Salvadori, Ilaria},
    year={2025},
    eprint={??},
    archivePrefix={arXiv}
}
```

---

## Repository Structure

```
UJSSP/
├── data/                               # Data used to run experiments.
│   ├── UJSSP/                              # Uniform instances according to method used to generate probabilities.
│   └── Product_Partition/                  # Instances for Product Partition that either allow a yes-answer or are randomly generated.
├── src/                                # C++ implementation of methods + code to generate and handle data.
│   ├── UJSSP/                              # Implementations used for uniform instances. 
│   │   ├── Data_code/                          # Code to generate and handle data.
│   │   ├── MILP_implementation_gurobi/         # C++ implementation of the MILP formulation using Gurobi.
│   │   ├── Dynamic_Programming/                # Implementation of the dynamic programming algorithm.
│   │   ├── Forward_stepwise_exact_method/      # Implementation of the forward stepwise exact algorithm.
│   │   ├── Backward_stepwise_exact_method/     # Implementation of the forward stepwise exact algorithm.
│   └── Product_Partition/                  # Implementations used for instances derived from Product Partition.
│   │   ├── Data_code/                          # Scripts to generate Product Partition instances.
│   │   ├── Stepwise_method/                    # Stepwise exact algorithm applied to Product Partition instances.
├── results/                            # Graphs found in paper
├── README.md
└── LICENSE
```

---

## Requirements

- C++ compiler (e.g., `g++`)  
- [Gurobi Optimizer](https://www.gurobi.com/) with C++ API installed and licensed (only required for MILP)
- Optional (Product Partition experiments only): GMP and MPFR libraries for high-precision arithmetic

---

## Running the algorithms
All executables write results to a user-created output/ folder located in the same directory as the data file used. This folder should be created before running.
The output includes the running time, optimal solution when found, and possibly additional metrics depending on the algorithm.

### 1. Dynamic progrmming and stepwise methods for UJSSP
To run the dynamic programming procedure or any of the stepwise methods use the following

```bash
g++ src/UJSSP/[algorithm to run]/Source.cpp src/UJSSP/Data_code/Data.cpp
./a.out data/UJSSP/[datafile to use]
```

Replace [algorithm_to_run] with one of: Dynamic_programming, Forward_stepwise_exact_method or Backward_stepwise_exact_method

### 2. MILP for UJSSP
To run the MILP use the following

```bash
g++ -I/path/to/gurobi/gurobi1202/linux64/include src/UJSSP/MILP_implementation_gurobi/Source.cpp src/UJSSP/Data_code/Data.cpp -L/path/to/gurobi/gurobi1202/linux64/lib -lgurobi_c++ -lgurobi120
export LD_LIBRARY_PATH=/path/to/gurobi/gurobi1202/linux64/lib:$LD_LIBRARY_PATH
export GRB_LICENSE_FILE=gurobi.lic
./a.out data/UJSSP/[datafile to use]
```

Replace /path/to/gurobi with your local Gurobi installation path.

### 3. Stepwise method for Product Partition
To run the stepwise method for instances derived from Product Partition use the following

```bash
g++ -std=c++17 src/Product_Partition/Stepwise_method/Source.cpp -I/path/to/gmp/highprecision/gmp/include -I/path/to/mpfr/highprecision/mpfr/include -I. -L/path/to/gmp/highprecision/gmp/lib -L/path/to/mpfr/highprecision/mpfr/lib -lgmpxx -lgmp -lmpfr
# Optional: add -L/path/to/GCCcore/13.3.0/lib64 and LD_LIBRARY_PATH=/path/to/GCCcore/13.3.0/lib64:$LD_LIBRARY_PATH if needed in your environment
export LD_LIBRARY_PATH=/path/to/gmp/highprecision/gmp/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/path/to/mpfr/highprecision/mpfr/lib:$LD_LIBRARY_PATH
./a.out data/UJSSP/[datfile to use]
```

Replace /path/to/... with your corresponding library directories.
