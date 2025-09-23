# Exceptions

```{topic} This page is **up-to-date**! 
The information on this page pertains to the engine post-restructuring.
```

Cacao Engine exceptions come in a variety of different classes (though they are functionally identical) to give further context about what has happened. Each exception also has an associated message. 

## Built-In Exceptions
* `External` - The exception came from a non-Cacao library and is not associated with the engine itself, but is being forwarded
* `FileNotFound` - A requested file was not found
* `IO` - An I/O error occurred
* `FileOpen` - A file could not be opened
* `InvalidYAML` - YAML input followed a different schema than what was expected
* `BadInitState` - Something was in the wrong initialization state
* `BadRealizeState` - A resource's realized representation existed when it should not have, or did not exist when it should have
* `BadState` - Something was in the wrong state
* `BadThread` - A function was called on an unsupported thread
* `BadType` - An object of a different type than the expected one was passed
* `BadValue` - A value was passed that did not meet constraints
* `NonexistentValue` - A value was expected, but an error or null value was returned or the value did not exist
* `ExistingValue` - A value was expected not to exist when it did
* `Misc` - Anything that doesn't fit the above categories
