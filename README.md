# Spindler
A python package to compute the long-term orbital evolution of a binary interacting with a circumbinary disk.

Based on [Valli et al. 2024](https://arxiv.org/abs/2401.17355).

Find the c-based version at https://github.com/ruggero-valli/spindler-c

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

## Documentation
https://spindler.readthedocs.io/
