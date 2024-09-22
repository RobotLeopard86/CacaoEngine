# Cacao Engine Documentation Build Instructions

## Prerequisites
* Python 3 (with Pip)
* Doxygen

## 1. Install Python Dependencies
Open this directory in your terminal, and run `python3 -m pip install -U -r requirements.txt`. This will install all necessary Python packages.

## 2. Build the Docs
Run `sphinx-build . (desired output directory) -a -b html -j auto` to generate the HTML documentation. It will be outputted into the directory specified in place of `(desired output directory)`.