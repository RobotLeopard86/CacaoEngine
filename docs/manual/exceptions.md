# Exceptions

Cacao Engine exceptions come in a variety of different classes (though they are functionally identical) to give further context about what has happened. Each exception also has an associated message. 

## Built-In Exceptions
0. `External` - The exception came from an non-Cacao library and is not associated with the engine itself
* `FileNotFound` - A requested file was not found
* `IO` - An I/O error occurred
* `FileOpen` - A file could not be opened
* `InvalidYAML` - YAML input followed a different schema than what was expected
* `BadInitState` - Something was in the wrong initialization state
* `BadGPUState` - A resource's associated GPU object existed when it should not have or did not exist when it should have
* `BadState` - Something was in the wrong state
* `BadType` - An object of a different type than the expected one was passed
* `BadThread` - A function was called on an unsupported thread
* `NonexistentValue` - A value was requested but an error value was returned instead
* `NullValue` - A null value was given when a non-null value was expected
* `Container` - Something went wrong pertaining to a list or map
* `Misc` - Anything that doesn't fit the above categories
