# Spindler
A python package to compute the long-term orbital evolution of a binary interacting with a circumbinary disk.

Based on Valli et al. 2024 (in prep.)

## Installation:
```bash
git clone https://github.com/ruggero-valli/spindler.git
cd spindler
# (Optional) create an empty conda environment
conda create --name spindler-env python=3.11
conda activate spindler-env
pip install .
```
You can test wether the installation was succesfull by running
```bash
python tests/test_spindler.py
```
If the last line of the output is `OK`, then the installation was succesfull and the program is working.

## Description
spindler provides three alternative models of binary-disk interaction
- **Siwek23**: based on the simulations in
   - Siwek et al. 2023 (2023MNRAS.518.5059S)
   - Siwek et al. 2023 (2023MNRAS.522.2707S)
    
    It is defined for eccentricity between 0 and 0.8, and mass ratio between 0.1
    and 1.
- **DD21**: based on the simulations in
    - D'Orazio and Duffell 2021 (2021ApJ...914L..21D)
    
    It is defined for eccentricity between 0 and 0.8, and mass ratio equal to 1.
- **Zrake21**: based on the simulations in
    - Zrake et al. 2021 (2021ApJ...909L..13Z)
    
    It is defined for eccentricity between 0 and 0.8, and mass ratio equal to 1.

## Usage
There are two basic ways to use the spindler.

- Compute the evolution of a binary system
- Provide the derivatives of the orbital parameters, which can then be used by another package to compute the evolution separately.

### Computing the evolution

```python
import numpy as np
import matptplotlib.pyplot as plt

from spindler.solver import Solver_Siwek23, Solver_DD21, Solver_Zrake21

# Compute the evolution of a binary system interacting
# with a circumbinary disk.

# Define the initial conditions
q0 = 0.7 # initial mass ratio
e0 = 0.1 # initial eccentricity

# Define the mass that will be accreted from the disk
# in units of the initial mass of the binary.
accreted_mass = 0.4 # 40% of the initial mass
 
a, q, e, m = Solver_Siwek23().evolve(accreted_mass, q0, e0)

plt.plot(m, e)
plt.xlabel("binary mass / initial mass")
plt.ylabel("eccentricity")
plt.show()

plt.plot(m, a)
plt.xlabel("binary mass / initial mass")
plt.ylabel("separation / initial separation")
plt.show()
```
The code would be analogous when using `Solver_DD21` or `Solver_Zrake21`, except that they only support `q0=1`.

### Computing the derivatives of the orbital parameters
The derivatives are expressed in logaritmic form i.e., the derivative of the quantity $X$ will be given as
$$\frac{d \log X}{d\log m},$$

where $m$ is the mass of the binary (the sum of the two components).

Spindler can compute the derivatives of the following quantities:
- eccentricity $e$
- mass ratio $q$
- semimajor axis $a$
- orbital energy $E$
- orbital angular momentum $J$

```python
from spindler.solver import Solver_Siwek23, Solver_DD21, Solver_Zrake21

solver = Solver_Siwek23()
q = 0.7
e = 0.1

De = solver.get_De(q,e)
Dq = solver.get_Dq(q,e)
Da = solver.get_Da(q,e)
DE = solver.get_DE(q,e)
DJ = solver.get_DJ(q,e)
```
The code would be analogous when using `Solver_DD21` or `Solver_Zrake21`, except that they only support `q=1`.
  

## API documentation
https://spindler.readthedocs.io/