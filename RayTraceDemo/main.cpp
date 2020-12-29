#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "color.h"
#include "sphere.h"
#include "utill.h"
#include <iostream>
#include "hittable_list.h"

double hit_sphere(const point3& center, double radius, const ray& r)
{
	vec3 oc = r.origin() - center;
	auto a = r.direction().length_squared();
	auto half_b = dot(oc, r.direction());
	auto c = oc.length_squared() - radius * radius;
	auto discriminant = half_b * half_b - a * c;
	if (discriminant < 0)
	{
		return -1.0;
	}
	else
	{
		return (-half_b - sqrt(discriminant)) / (2.0 * a);
	}
}

color ray_color(const ray& r, const hittable& world)
{
	hit_record rec;
	/*auto t = hit_sphere(point3(0, 0, -1), 0.5, r);*/
	if (world.hit(r, 0, infinity, rec))
	{
		//vec3 N = unit_vector(r.at(t) - vec3(0, 0, -1));
		return 0.5 * (rec.normal + color(1, 1, 1));//color(N.x() + 1, N.y() + 1, N.z() + 1);
	}
	vec3 unit_direction = unit_vector(r.direction());
	auto t = 0.5 * (unit_direction.y() + 1.0);
	return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

int main()
{
	stbi_flip_vertically_on_write(true);
	const auto aspect_ratio = 16.0 / 9.0;
	const int image_width = 256;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	struct RGB data[image_height][image_width];
	hittable_list world;
	world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
	world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));
	auto viewport_height = 2.0;
	auto viewport_width = aspect_ratio * viewport_height;
	auto focal_length = 1.0;
	auto origin = point3(0, 0, 0);
	auto horizontal = vec3(viewport_width, 0, 0);
	auto vertical = vec3(0, viewport_height, 0);
	auto lower_left_corner = origin - horizontal / 2 - vertical / 2 - vec3(0, 0, focal_length);
	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
	for (int j = image_height - 1; j >= 0; --j)
	{
		std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < image_width; ++i)
		{
			auto u = double(i) / (image_width - 1);
			auto v = double(j) / (image_height - 1);
			ray r(origin, lower_left_corner + u * horizontal + v * vertical - origin);
			color pixel_color = ray_color(r, world);
			//color pixel_color(r,g,b);
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