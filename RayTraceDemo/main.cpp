//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"
#include "ray.h"
#include "vec3.h"
#include "color.h"
#include "sphere.h"
#include "utill.h"
#include "hittable_list.h"
#include "camera.h"
#include <iostream>
#include <thread>
#include "aarect.h"
#include "box.h"
#include "material.h"
#include "memory.h"
#include "moving_sphere.h"

const auto num_threads = std::thread::hardware_concurrency() - 1;
int index;
RGB* data;

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

color ray_color(const ray& r, const color& background, const hittable& world, int depth) {
	hit_record rec;

	// If we've exceeded the ray bounce limit, no more light is gathered.
	if (depth <= 0)
		return color(0, 0, 0);

	// If the ray hits nothing, return the background color.
	if (!world.hit(r, 0.001, infinity, rec))
		return background;

	ray scattered;
	color attenuation;
	color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

	if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
		return emitted;

	return emitted + attenuation * ray_color(scattered, background, world, depth - 1);
}

hittable_list random_scene() {
	hittable_list world;

	auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
	world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(checker)));

	//auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
	//world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			auto choose_mat = random_double();
			point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

			if ((center - point3(4, 0.2, 0)).length() > 0.9) {
				shared_ptr<material> sphere_material;

				if (choose_mat < 0.8)
				{
					// diffuse
					auto albedo = color::random() * color::random();
					sphere_material = make_shared<lambertian>(albedo);
					auto center2 = center + vec3(0, random_double(0, .5), 0);
					world.add(make_shared<moving_sphere>(center, center2, 0.0, 1.0, 0.2, sphere_material));
				}
				else if (choose_mat < 0.95)
				{
					// metal
					auto albedo = color::random(0.5, 1);
					auto fuzz = random_double(0, 0.5);
					sphere_material = make_shared<metal>(albedo, fuzz);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
				else
				{
					// glass
					sphere_material = make_shared<dielectric>(1.5);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
			}
		}
	}

	auto material1 = make_shared<dielectric>(1.5);
	world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

	auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
	world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

	auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
	world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

	return world;
}

hittable_list two_spheres()
{
	hittable_list objects;
	auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
	objects.add(make_shared<sphere>(point3(0, -10, 0), 10, make_shared<lambertian>(checker)));
	objects.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));
	return objects;
}

hittable_list two_perlin_spheres()
{
	hittable_list objects;
	auto pertext = make_shared<noise_texture>(4);
	objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
	objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));
	return objects;
}

hittable_list earth()
{
	auto earth_texture = make_shared<image_texture>("earthmap.jpg");
	auto earth_surface = make_shared<lambertian>(earth_texture);
	auto globe = make_shared<sphere>(point3(0, 0, 0), 2, earth_surface);

	return hittable_list(globe);
}

hittable_list simple_light() {
	hittable_list objects;

	auto pertext = make_shared<noise_texture>(4);
	objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
	objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

	auto difflight = make_shared<diffuse_light>(color(4, 4, 4));
	objects.add(make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));

	return objects;
}

hittable_list cornell_box()
{
	hittable_list objects;
	auto red = make_shared<lambertian>(color(.65, .05, .05));
	auto white = make_shared<lambertian>(color(.73, .73, .73));
	auto green = make_shared<lambertian>(color(.12, .45, .15));
	auto light = make_shared<diffuse_light>(color(15, 15, 15));
	objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
	objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
	objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
	objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));
	objects.add(make_shared<box>(point3(130, 0, 65), point3(295, 165, 230), white));
	objects.add(make_shared<box>(point3(265, 0, 295), point3(430, 330, 460), white));
	shared_ptr<hittable> box1 = make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
	box1 = make_shared<rotate_y>(box1, 15);
	box1 = make_shared<translate>(box1, vec3(265, 0, 295));
	objects.add(box1);

	shared_ptr<hittable> box2 = make_shared<box>(point3(0, 0, 0), point3(165, 165, 165), white);
	box2 = make_shared<rotate_y>(box2, -18);
	box2 = make_shared<translate>(box2, vec3(130, 0, 65));
	objects.add(box2);
	return objects;
}

//Adapted from https://www.geeksforgeeks.org/how-to-create-a-command-line-progress-bar-in-c-c/
void loadingBar(int current, int mult)
{
	auto temp = (float)current / (float)mult;
	//system("color 0A");
	char a = 177, b = 219;
	printf("\n\n\n\n");
	printf("\n\n\n\n\t\t\t\t\t Loading...\n\n");
	printf("\t\t\t\t\t");

	//Background loading bar
	for (int i = 0; i < 24; i++)
	{
		printf("%c", a);
	}

	printf("\r");
	printf("\t\t\t\t\t");

	//Filled loading bar
	for (int i = 0; i < temp * 24; i++)
	{
		printf("%c", b);
	}
	printf("\n\n");
	printf("\t\t\t\t\t");

	printf("%4.4f %%", temp * 100);
}

//Clears screens with commands for Windows, Linux, macOS
void ClearScreen()
{
	system("clear");
	system("cls");
	std::cout << "\033[2J\033[1;1H";
}

void colorCalc(int startValue, int endValue, int image_width, int samples_per_pixel, int max_depth, hittable_list world, color background, const int image_height, RGB*& data, camera cam, int total)
{
	//Max -> 0
	for (int j = startValue; j >= endValue; --j)
	{
		//std::cerr << "\rScanlines remaining: " << j << ' ' << '\n' << std::flush;
		ClearScreen();
		loadingBar(total - (image_height- index), total);
		for (int i = 0; i < image_width; ++i)
		{
			//RGB* temp = new RGB;
			color pixel_color(0, 0, 0);
			for (int s = 0; s < samples_per_pixel; ++s)
			{
				auto u = double(i + random_double()) / (image_width - 1);
				auto v = double(j + random_double()) / (image_height - 1);
				ray r = cam.get_ray(u, v);
				pixel_color += ray_color(r, background, world, max_depth);
			}
			auto temp = write_color(std::cout, pixel_color, samples_per_pixel);
			data[(j * image_width) + i].R = temp.R;
			data[(j * image_width) + i].G = temp.G;
			data[(j * image_width) + i].B = temp.B;
		}
		index++;
	}
}

void threadRun(int image_width, int samples_per_pixel, int max_depth, hittable_list world, color background,
               const int image_height, RGB* data, camera cam, int total)
{
	std::vector<std::thread> threads(num_threads);
	//std::vector<int> in_count(num_threads);
	//in_count.resize(num_threads);
	int countTab = image_height/num_threads;
	for (size_t i = 0; i < num_threads; ++i) 
	{
		//int total_iterations = total_count / num_threads;
		//if (i == 0) {
		//	total_iterations += total_count % num_threads;
		//}

		threads.emplace_back(colorCalc, countTab * (i+1), countTab * i,
			image_width, samples_per_pixel, max_depth, 
			world, background, image_height, std::ref(data), cam, total);
	}

	for (auto& thread : threads) {
		if (thread.joinable()) {
			thread.join();
		}
	}
}

int main()
{
	auto aspect_ratio = 16.0 / 9.0;
	int image_width = 400;
	int samples_per_pixel = 100;
	int max_depth = 50;

	// World

	hittable_list world;

	point3 lookfrom;
	point3 lookat;
	auto vfov = 40.0;
	auto aperture = 0.0;
	color background(0, 0, 0);

	switch (0)
	{
		default:
		case 1:
		world = random_scene();
		background = color(0.70, 0.80, 1.00);
		lookfrom = point3(13, 2, 3);
		lookat = point3(0, 0, 0);
		vfov = 20.0;
		aperture = 0.1;
		break;
	case 2:
		world = two_spheres();
		background = color(0.70, 0.80, 1.00);
		lookfrom = point3(13, 2, 3);
		lookat = point3(0, 0, 0);
		vfov = 20.0;
		break;
	case 3:
		world = two_perlin_spheres();
		background = color(0.70, 0.80, 1.00);
		lookfrom = point3(13, 2, 3);
		lookat = point3(0, 0, 0);
		vfov = 20.0;
		break;
	case 4:
		world = earth();
		background = color(0.70, 0.80, 1.00);
		lookfrom = point3(13, 2, 3);
		lookat = point3(0, 0, 0);
		vfov = 20.0;
		break;

	case 5:
		world = simple_light();
		samples_per_pixel = 400;
		background = color(0, 0, 0);
		lookfrom = point3(26, 3, 6);
		lookat = point3(0, 2, 0);
		vfov = 20.0;
		break;
	
	case 6:
		world = cornell_box();
		aspect_ratio = 1.0;
		image_width = 400;
		samples_per_pixel = 200;
		background = color(0, 0, 0);
		lookfrom = point3(278, 278, -800);
		lookat = point3(278, 278, 0);
		vfov = 40.0;
		break;
	}
	const vec3 vup(0, 1, 0);
	const auto dist_to_focus = 10.0;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	data = (RGB*)malloc(image_height * image_width * sizeof(RGB));
	int index = 0;
	camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0); int total = image_height - 1;
	threadRun(image_width, samples_per_pixel, max_depth, world, background, image_height, data, cam, total);
	stbi_write_jpg("raytrace_02.jpg", image_width, image_height, sizeof(RGB), data, 100);
	std::cerr << "\nDone.\n";
}