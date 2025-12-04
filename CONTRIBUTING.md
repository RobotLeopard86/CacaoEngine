# Cacao Engine Contribution Guidelines

Welcome, contributors! If you want to contribute to Cacao Engine, there are some guidelines to follow. These are outlined below.  

As a side note, we recommend using the `clangd` extension if using Visual Studio Code instead of the Microsoft C/C++ extension, as it supports Objective-C(++) and is in some ways superior, though this is obviously not a requirement and is simply a suggestion.

## Formatting & Style
When submitting code, it must be formatted using [`clang-format`](https://clang.llvm.org/docs/ClangFormat.html) using the provided formatting configuration. It must also follow the Cacao Engine capitalization and naming conventions, those being:
* Classes and methods should be named in PascalCase
* Member fields and variables should be named in camelCase
* Do not use Hungarian notation

For consistency, please use American English spellings in interface naming.  
  
Please also avoid the use of `auto`, except for the below cases:  
* Iterators
* Map iteration destructuring (`auto& [one, two] : someMap`)
* Lambda functions/`std::bind`

In addition, `#include` directives must be grouped (one for Cacao Engine includes, one for STL types and system headers, and one for external libraries). Do not use angle-bracketed includes except for system and STL headers.

All code **must** include clear comments in English with a brief summary of what the particular section of code is doing. All declarations in the public header files (see below) must also have Doxygen annotations as shown here:
```cpp
/**
 * @brief The description...
 * @details (optional)
 *
 * [random notes]
 *
 * [ @note | @warning ]
 *
 * @param a Parameter A...
 * @param b Parameter B...
 *
 * @return The return value description...
 *
 * @throws ExceptionType When this exception is thrown...
 */
```  
Note the newlines between each section and the consistent prefixing asterisks.

The public header files are those located in the following locations:
* `engine/include`
* `libs/audiodecoder/include`
* `libs/common/include`
* `libs/formats/include`
* `libs/image/include`

The only exceptions to the code-commenting rule in function implementations are:
1. `Check` calls at the top of the function to validate input
2. Small functions whose effect is self-descriptive in what methods are called

## Branching Scheme
Cacao Engine has two primary branches: `main` and `dev`. `dev` is an **unstable** branch where all core development should happen. `dev` should be squash-merged into `main` when a set of changes is complete and working. `main` **must** remain somewhat stable; in-progress work must not be merged into `main` (except work existing prior to the branch split).  

During release preparation, a branch should be made from `main` named `staging/<nickname of release>`. All alpha and beta testing should continue in the staging branch until the stable release is completed. When a release is finished, the staging branch should be squash-merged back into `dev` and `main`, with release tags being made from `main`. Once the release's lifecycle is complete (i.e. no more patch releases will be made and the next release will be a minor release instead), the staging branch should be squash-merged back into `dev` and `main` once more and deleted.

## Contributing Your Changes
All code contributions must be submitted as GitHub pull requests; standalone patches will not be accepted.  

Before making changes, make a fork of the repository on GitHub. Ensure that the "Copy the `main` branch only" option is unchecked. Please make all changes in your copy of the `dev` branch.  

Before submitting your changes, ensure to pull from the upstream `dev` branch and reconcile any changes. Then, open a GitHub pull request into the upstream `dev` branch. Be sure to include a detailed summary of what changes you made and why. Only pull requests in English will be accepted.

## AI Policy
While generative AI is extremely useful for programming, **no AI-generated code** is allowed in Cacao Engine. It is perfectly fine to use AI for brainstorming, prototyping, debugging, and such, but no AI code may be submitted as a pull request. It is acceptable to use AI to write a concept for you, but sufficient human modification must be done to the concept if using it as a base. If you use AI in this manner, please include all prompts and the original AI code in your pull request for reference. **Any unauthorized contributions using AI-generated code will be rejected.**  

The reason for this is that the legality and copyrightability of AI-generated code has not been fully tested, and many AI tools have been trained on code that may not be suitably licensed for use under Cacao Engine's license. Even if they weren't, it would be impossible to find the correct license for each generation.

This policy only applies to **primary code**. This means that the `scripts` directory, for example, is not subject to this policy. Primary code directories are those listed below:
* `engine`
* `libs`
* `tools`
* `sandbox`