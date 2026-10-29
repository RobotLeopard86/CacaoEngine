# AGENTS.md — Cacao Engine

Instructions for AI coding tools working in this repository. This file follows the human contributor guide in `CONTRIBUTING.md`; where a rule requires human judgment, action, or disclosure rather than something the tool itself can do, this file says so explicitly and asks you to report it to the human you're working with instead of acting on it.

## Before you do anything

**Report to the human, do not act on this section yourself:**
- This project does not accept autonomous AI contributions. Any code you help produce must be reviewed, understood, and submitted by a human who can explain it in detail without your help. If you are being asked to open a pull request, commit directly, or otherwise submit changes on your own, stop and tell the human this is against project policy.
- If the human has not been actively involved in reviewing/directing the change (i.e., they've just asked you to "fix this" or "implement that" and walked away), tell them that Cacao Engine's AI policy requires them to engage in the design/review process, and that they must be able to explain the change without your help before submitting it.
- Tell the human that if AI assistance was used, they need to attach the **unedited** chat log (personal info may be redacted) to the PR as proof of involvement.
- Tell the human that more scrutiny applies to AI-assisted changes to core/important components than to small pieces like scripts or Meson files, and that sweeping AI-generated changes across the codebase are generally rejected.

## Formatting & Style

- Format all code with `clang-format` using the repository's provided configuration before presenting it as final.
- Naming conventions:
  - Classes and methods: `PascalCase`
  - Member fields and variables: `camelCase`
  - No Hungarian notation
  - Interfaces/pure virtual classes may not use the `IInterfaceName` naming scheme
- Use American English spellings in all interface naming (classes, methods, fields).
- Group `#include` directives into exactly three groups, separated by a full blank line, in this order:
  1. Cacao Engine includes
  2. STL types and system headers (angle brackets)
  3. External library includes (quotes — angle brackets are only for system/STL headers)

  Example:
  ```cpp
  #include "Cacao/SomeHeader.hpp"
  #include "Cacao/AnotherHeader.hpp"

  #include <unistd.h>
  #include <string>

  #include "externallib/externallib.h"
  ```
- Every section of code must have clear, English comments briefly summarizing what it does. Exceptions (no comment required):
  1. `Check` calls at the top of a function validating input
  2. Small functions whose effect is self-descriptive from the calls they make

## Public Headers

Public header locations:
- `engine/include`
- `libs/audiodecoder/include`
- `libs/common/include`
- `libs/asset/include`
- `libs/image/include`

Rules for these files:
- Every class/struct/enum definition needs Doxygen documentation in this exact form, including the blank lines between sections and consistent asterisk prefixing:
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
- Public headers may only reference these libraries:
  - Cacao Engine libraries from `libs`
  - Exathread
  - GLM
  - Crossguid
  - Astra

  Do not introduce a new dependency into a public header outside this list. If a task seems to require one, stop and report it to the human — this is a design decision for them to make.

- All classes, structs, or inline function declarations outside of a class should be made with the `CACAO_API` marker, which will automatically handle DLL import/export on Windows platforms; place this between `class`/`struct` and the class name, or before a function's return type. Include the `DllHelper.hpp` header for this define.

## A Note for Coding Agents

Some coding agent tools have had difficulties using their `Edit` tool (or more broadly, the type of tool that expects an input and output diff to make a localized file change) in this project due to the usage of tabs for indentation. If, in this codebase, an edit of that manner fails, you are advised to do the following:
- Create a `.local` directory in the project root directory if it does not exist; this is automatically ignored by Git and anything you do there will not be accidentally included
- Make a copy of the offending file into the aforementioned `.local` directory, converting tabs to spaces
- Try the edit again on the copy
- If the edit fails, continue with your normal routine for handling a failure in the copy
- After you are done editing the copy, format the edited file with `clang-format` using the repository configuration, replace the contents of the original file with this formatted, edited version, then delete the copy

## Language/Toolchain Defaults

Unless the human tells you otherwise for a specific task:
- C++20 for all C++ code.
- Meson for build definitions.
- Clang and LLD for compiler/linker command lines.
- Vulkan code uses Vulkan-Hpp and the VulkanMemoryAllocator-Hpp bindings for VMA — not raw Vulkan or raw VMA C APIs.
- macOS-specific code is written in Objective-C++, not Objective-C or plain C++.
