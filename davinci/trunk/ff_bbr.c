#include "parser.h"

/**
 ** black-body radiance support function
 ** 
 **/

#define C1 1.1909e-12
#define C2 1.43879

double 
bbr(double wn, double temp)
{
	/**
	 ** wn = wavenumber
	 ** temp = temperature in Kelvin
	 **/
    if (temp <= 0)
        return (0.0);
    return ((C1 * (wn * wn * wn)) / (exp(C2 * wn / temp) - 1.0));
}

/**
 ** compute brightness temperature.
 **
 ** Try to catch the cases where divison by zero would occur.
 **/

double
btemp(double f, double radiance)
{
	double x;

	if (radiance <= 0)
		return (0.0);

	x = log(1.0 + (C1 * f * f * f / radiance));
	if (x <= 0)
		return (0.0);

	return (C2 * f / log(1.0 + (C1 * f * f * f / radiance)));
}
