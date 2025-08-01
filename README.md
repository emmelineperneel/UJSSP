# Unreliable Job Selection and Sequencing Problem (UJSSP)

This repository contains all the code associated with **“The Unreliable Job Selection and Sequencing Problem”**.

📄 **Paper**: See .. 
📦 **DOI**: [10.5281/zenodo.15684074](https://doi.org/10.5281/zenodo.15684074)  
📜 **License**: [MIT License](./LICENSE)

---

## 📁 Repository Structure

- `data_code/`  
  Scripts for generating, reading, and processing data used in the UJSSP experiments.

- `MILP_Implementation_Gurobi/`  
  C++ implementation of the MILP formulation using Gurobi.

- `Forward_Stepwise_Exact_algorithm/`  
  Implementation of the forward stepwise exact algorithm for solving UJSSP instances.

- `backward_stepwise_exact_algorithm/`  
  Implementation of the backward stepwise exact algorithm for solving UJSSP instances.

- `Product_Partition/`  
  Code related to experiments based on reductions from the **Product Partition** problem:
  - `Data_code/`: Scripts to generate Product Partition instances.
  - `Stepwise_algorithm/`: Stepwise exact algorithm applied to these instances.

---

## ▶️ Running the Code

### Requirements

- C++ compiler (e.g. `g++`)
- [Gurobi Optimizer](https://www.gurobi.com/) with C++ API installed and licensed

