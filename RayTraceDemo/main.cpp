#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "color.h"
#include "vec3.h"
#include "utill.h"
#include <iostream>

int main() {
	const int image_width = 256;
	const int image_height = 256;
	struct RGB data[image_height][image_width];
	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
	for (int j = image_height - 1; j >= 0; --j)
	{
		std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < image_width; ++i)
		{
			auto r = double(i) / (image_width - 1);
			auto g = double(image_height - 1 - j) / (image_height - 1);
			auto b = 0.25;
			/*int ir = static_cast<int>(255.999 * r);
			int ig = static_cast<int>(255.999 * g);
			int ib = static_cast<int>(255.999 * b);*/
			
			color pixel_color(r,g,b);
			 RGB temp = write_color(std::cout, pixel_color);
			data[j][i].R = temp.R;
			data[j][i].G = temp.G;
			data[j][i].B = temp.B;
			//std::cout << ir << ' ' << ig << ' ' << ib << '\n';
		}
	}
	stbi_write_jpg("raytrace.jpg", image_width, image_height, sizeof(RGB), data, 100);
    std::cerr << "\nDone.\n";
}