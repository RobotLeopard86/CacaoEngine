# Cacao Engine Contribution Guidelines

Welcome, contributors! If you want to contribute to Cacao Engine, there are some guidelines to follow. These are outlined below.  

As a side note, we recommend using the `clangd` extension if using Visual Studio Code instead of the Microsoft C/C++ extension, as it supports Objective-C(++) and is in some ways superior, though this is obviously not a requirement and is simply a suggestion.

## Formatting & Style
When submitting code, it must be formatted using [`clang-format`](https://clang.llvm.org/docs/ClangFormat.html) using the provided formatting configuration. It must also follow the Cacao Engine capitalization and naming conventions, those being:
* Classes and methods should be named in PascalCase
* Member fields and variables should be named in camelCase
* Do not use Hungarian notation
  
Please also avoid the use of `auto`, except for iterators and map iteration destructuring (`auto& [one, two] : someMap`).  
In addition, `#include` directives must be grouped (one for Cacao Engine includes, one for STL types and system headers, and one for external libraries). Do not use angle-bracketed includes except for system and STL headers.

## Branching Scheme
Cacao Engine has two primary branches: `main` and `dev`. `dev` is an **unstable** branch where all core development should happen. `dev` should be squash-merged into `main` when a set of changes is complete and working. `main` **must** remain somewhat stable; in-progress work must not be merged into `main` (except work existing prior to the branch split).  
During release preparation, a branch should be made from `main` named `staging/<nickname of release>`. All alpha and beta testing should continue in `dev` and be merged into that branch until the stable release is completed. Once the release is made, the staging branch should be merged back into `main`, synchronizing it with `dev`, and the staging branch should be deleted. Release tags should be made from `main`.

## Contributing Your Changes
All code contributions must be submitted as GitHub pull requests; standalone patches will not be accepted.  
Before making changes, make a fork of the repository on GitHub. Ensure that the "Copy the `main` branch only" option is unchecked. Please make all changes in your copy of the `dev` branch.  
Before submitting your changes, ensure to pull from the upstream `dev` branch and reconcile any changes. Then, open a GitHub pull request into the upstream `dev` branch. Be sure to include a detailed summary of what changes you made and why. Only pull requests in English will be accepted.