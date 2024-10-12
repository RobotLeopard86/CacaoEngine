# Exception Codes

Cacao Engine exceptions rely on a code system to give further context about the exception. Engine core codes are a one or two-digit number. Backend codes are three digits, beginning with `1` and followed by a two-digit number, prefixed with a `0` if less than ten (example: `104`). All codes registered by your game, should follow a similar format, but beginning with any other number than `1` or `0` (though this is not enforced).  

## Engine Core Exception Codes
0. `External` - The exception came from an external library and is not associated with the engine itself
1. `FileNotFound` - A requested file was not found
2. `NonexistentValue` - A value was requested but does not exist
3. `InvalidYAML` - YAML input followed a different schema than what was expected
4. `BadInitState` - Something was in the wrong initialization state
5. `NullValue` - A null value was given when a non-null value was expected
6. `BadState` - Something was in the wrong state
7. `FileOpenFailure` - A file could not be opened
8. `EventManager` - Something went wrong in the event manager
9. `ContainerValue` - Something went wrong pertaining to a container
10. `WrongType` - A value of a different type than the expected one was passed
11. `IO` - An IO error occurred
12. `BadCompileState` - Something was in the wrong compilation state
13. `BadValue` - A given value was not the expected one