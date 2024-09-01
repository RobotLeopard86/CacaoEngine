//As an exception to the overarching licensing of the Cacao Engine project, this file is designated by the author
//to the public domain to the full extent as permissible and applicable by law.

//Okay so this file ONLY exists because of some weird crap with static libm that has these symbols undefined and causes linker errors
//So I just made a definition of them to fix that

#include <math.h>

float __frexpf(float x, int* exp) {
	return frexpf(x, exp);
}

float __ldexpf(float x, int exp) {
	return ldexpf(x, exp);
}
