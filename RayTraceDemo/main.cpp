#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "ray.h"
#include "vec3.h"
#include "color.h"
#include "sphere.h"
#include "utill.h"
#include "hittable_list.h"
#include "camera.h"
#include <iostream>

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

color ray_color(const ray& r, const hittable& world, int depth)
{
	hit_record rec;
	if (depth <= 0)
	{
		return color(0, 0, 0);
	}
	if (world.hit(r, 0.001, infinity, rec))
	{
		//Random Ray Direction
		//point3 target = rec.p + rec.normal + random_in_unit_sphere();

		//Replacement Diffusion
		//point3 target = rec.p + rec.normal + random_unit_vector();

		//hemispherical scattering
		point3 target = rec.p + random_in_hemisphere(rec.normal);

		return 0.5 * ray_color(ray(rec.p, target - rec.p), world, depth - 1);
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
	const int samples_per_pixel = 100;
	const int max_depth = 50;
	struct RGB data[image_height][image_width];
	hittable_list world;
	world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
	world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));
	camera cam;
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
			color pixel_color(0, 0, 0);
			for (int s = 0; s < samples_per_pixel; ++s)
			{
				auto u = double(i + random_double()) / (image_width - 1);
				auto v = double(j + random_double()) / (image_height - 1);
				ray r = cam.get_ray(u, v);
				pixel_color += ray_color(r, world, max_depth);
			}
			RGB temp = write_color(std::cout, pixel_color, samples_per_pixel);
			data[j][i].R = temp.R;
			data[j][i].G = temp.G;
			data[j][i].B = temp.B;
		}
	}

	stbi_write_jpg("raytrace.jpg", image_width, image_height, sizeof(RGB), data, 100);
	std::cerr << "\nDone.\n";
}