# Installation
To install spindler, run the following commands
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