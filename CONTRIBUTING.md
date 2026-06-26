# Cacao Engine Contribution Guidelines

Welcome, contributors! If you want to contribute to Cacao Engine, there are some guidelines to follow. These are outlined below.  

As a side note, we recommend using the `clangd` extension if using Visual Studio Code instead of the Microsoft C/C++ extension, as it supports Objective-C(++) and is in some ways superior, though this is obviously not a requirement and is simply a suggestion.

## Formatting & Style
When submitting code, it must be formatted using [`clang-format`](https://clang.llvm.org/docs/ClangFormat.html) using the provided formatting configuration. It must also follow the Cacao Engine capitalization and naming conventions, those being:
* Classes and methods should be named in PascalCase
* Member fields and variables should be named in camelCase
* Do not use Hungarian notation

For consistency, please use American English spellings in interface naming (class, method, and field names). 

All `#include` directives must be grouped (one for Cacao Engine includes, one for STL types and system headers, and one for external libraries) with full blank lines between groups. Only use angle-bracketed includes for system and STL headers, **not** external libraries. Example:
```cpp
#include "Cacao/SomeHeader.hpp"
#include "Cacao/AnotherHeader.hpp"

#include <unistd.h>
#include <string>

#include "externallib/externallib.h"
```

All code **must** include clear comments in English with a brief summary of what the particular section of code is doing. All declarations in the public header files (see below) must also have Doxygen annotations as shown here:
```cpp
/**
 * @brief The description...
 * @details (optional)
 *
 * [any other information]
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
* `libs/asset/include`
* `libs/image/include`

The only exceptions to the code-commenting rule in function implementations are:
1. `Check` calls at the top of the function to validate input
2. Small functions whose effect is self-descriptive in what methods are called

## Branching Scheme
Cacao Engine has two primary branches: `main` and `dev`. `dev` is an **unstable** branch where all development should happen. `dev` should be squash-merged into `main` when a set of changes is complete and working. `main` **must** remain somewhat stable; in-progress work must not be merged into `main` (except work existing prior to the branch split).  

During release preparation, a branch should be made from `main` named `staging/<nickname of release>`. All pre-release development and testing should continue in the staging branch until the stable release is completed. No work for any release should take place in `dev` during staging, `dev` should be devoted only to work on the next release. Once the stable release is complete, the staging branch should be squash-merged back into `main`, and `dev` should be rebased on top of the new changes, resolving any and all merge conflicts if necessary.

Release tags should be made for all releases, regardless of status (alpha, beta, RC, stable), and created from the staging branch, being named exactly the same as the version number (see the [versioning info page on the documentation site](https://robotleopard86.github.io/CacaoEngine/dev/manual/versioning.html) for further information); no prefix or suffix should be used (`2026.1.0a`, not `v2026.1.0a`)

## AI Policy
While generative AI is extremely useful for programming, there are rules regarding how generative AI may be used in Cacao Engine.  

It is perfectly acceptable to use AI for brainstorming, prototyping, debugging, and code generation (provided you actually engage in the design process and **manually review the code** prior to submission). However, you **must be able to explain _in detail_ what your code is doing** without the AI's help and are wholly responsible for the code you submit.  

If you use AI in this manner, please include the **unedited** chat log for your change (although it's fine to censor stuff like personal information) as an attachment on your PR. You must show that sufficient human effort was put into the PR. If you cannot explain your patch without the AI's help or cannot demonstrate that you involved yourself in the development process (aside from tiny, self-explanatory patches), **your PR will be rejected**.  

As for what counts as sufficient human involvement, things such as involved planning and design conversation, multiple rounds of iteration and feedback, and/or manual edits will generally qualify. For the most part, you should be fine as long as you don't just tell the AI "fix this bug" or "implement this feature" and leave it at that.

More scrutiny will be applied to AI-generated changes to more important components, and less for smaller pieces like scripts/Meson files. Sweeping AI-generated changes to the codebase will generally be rejected, especially if no clear or satisfactory reason can be provided.

## Contributing Your Changes
All code contributions must be submitted as GitHub pull requests; standalone patches will not be accepted.  

Before making changes, make a fork of the repository on GitHub. **Ensure that the "Copy the `main` branch only" option is unchecked**. Please make all changes in your copy of the `dev` branch.  

Before submitting your changes, ensure to rebase your branch onto the upstream `dev` branch (`git pull --rebase`) and reconcile any changes. Then, open a GitHub pull request into the upstream `dev` branch. Be sure to include a detailed summary of what changes you made and why. Pull requests should be made in English, but the use of translation tools (including AI translation) is perfectly acceptable and encouraged for non-fluent speakers.
