#include "integrate_x_sq.h"

#include <iomanip>
#include <iostream>

#include "utill.h"

void integrate_x_sq::calcXSq()
{
	int N = 1000000;
	auto sum = 0.0;
	for (int i = 0; i < N; i++) 
	{
        auto x = pow(random_double(0,8), 1./3.);
        sum += x*x / pdf(x);
	}
	std::cout << std::fixed << std::setprecision(12);
	std::cout << "I = " << 2 * sum / N << '\n';
}

inline double pdf(double x) {
     return 3*x*x/8;
}