#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"

#include <iostream>

#include "utill.h"

RGB write_color(std::ostream& out, color pixel_color)
{
	int R = static_cast<int>(255.999 * pixel_color.x());
	int G = static_cast<int>(255.999 * pixel_color.y());
	int B = static_cast<int>(255.999 * pixel_color.z());
	
	out << R << ' ' << G << ' '<< B << '\n';
	return {(unsigned char) R, (unsigned char)G, (unsigned char)B};
}

#endif