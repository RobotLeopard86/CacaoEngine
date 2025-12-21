# What Is This?

Cacao Engine uses a Sphinx extension called Exhale to handle automatic integration between Doxygen & Sphinx.  
Its job is to generate all the reStructuredText documents for API documentation by running Doxygen and setting everything up.  

However, Cacao Engine uses C++20 **concepts** (new feature), and the [upstream Exhale project (which you should check out)](https://github.com/svenevs/exhale) has not yet merged support for it.  
That support does exist in a side branch, but it's based off of an old version of Sphinx that isn't compatible.  
Long story short, I cloned the repo, did a merge to bring the concepts support into the modern version, and then got rid of the Git files since I don't intend on updating this any time soon unless necessary.  

The license is compatible and it's right here in the directory.  
Maybe at some point in the future I can go back to using an official release, but as of writing this the author appears to be on hiatus.  
Anyways, that's my spiel. Hope this clears at least a few things up.  

-RobotLeopard86