#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"

#include <iostream>

#include "utill.h"

RGB write_color(std::ostream& out, color pixel_color, int samples_per_pixel)
{
	auto r = pixel_color.x();
	auto g = pixel_color.y();
	auto b = pixel_color.z();
	auto scale = 1.0 / samples_per_pixel;
	r = sqrt(scale * r);
	g = sqrt(scale * g);
	b = sqrt(scale * b);
	int R = static_cast<int>(256 * clamp(r, 0.0, 0.999));
	int G = static_cast<int>(256 * clamp(g, 0.0, 0.999));
	int B = static_cast<int>(256 * clamp(b, 0.0, 0.999));

	out << R << ' ' << G << ' ' << B << '\n';
	return { (unsigned char)R, (unsigned char)G, (unsigned char)B };
}

#endif