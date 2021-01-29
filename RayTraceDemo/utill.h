#pragma once
#include <cmath>
#include <limits>
#include <memory>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#ifdef _MSC_VER
    // Microsoft Visual C++ Compiler
    #pragma warning (push, 0)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Restore warning levels.
#ifdef _MSC_VER
    // Microsoft Visual C++ Compiler
    #pragma warning (pop)
#endif

struct RGB
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
};

using std::shared_ptr;
using std::make_shared;
using std::sqrt;
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;


double closest_so_far = 0;


inline double degrees_to_radians(double degrees)
{
	return degrees * pi / 180.0;
}
inline double random_double()
{
	return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max)
{
	return min + (max - min) * random_double();
}

inline double clamp(double x, double min, double max)
{
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

inline int random_int(int min, int max)
{
    return static_cast<int>(random_double(min, max+1));
}