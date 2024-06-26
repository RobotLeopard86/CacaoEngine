//Okay so this file ONLY exists because of some weird crap with static libm that has these symbols undefined and causes linker errors
//So I just made a definition of them to fix that

#include <math.h>

float frexpf(float x, int *exp) {
    int e = 0;
    float mantissa = x;
    if (x != 0.0f) {
        while (fabsf(mantissa) >= 1.0f) {
            mantissa /= 2.0f;
            e++;
        }
        while (fabsf(mantissa) < 0.5f) {
            mantissa *= 2.0f;
            e--;
        }
    }
    *exp = e;
    return mantissa;
}

float __frexpf(float x, int* exp){
	return frexpf(x, exp);
}

float ldexpf(float x, int exp) {
    return x * powf(2.0f, (float)exp);
}

float __ldexpf(float x, int exp){
	return ldexpf(x, exp);
}