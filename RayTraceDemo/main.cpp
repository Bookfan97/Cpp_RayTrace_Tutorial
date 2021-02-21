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

#include "aarect.h"
#include "box.h"
#include "bvh.h"
#include "constant_medium.h"
#include "material.h"
#include "memory.h"
#include "moving_sphere.h"
#include "External/GLFW/include/GLFW/glfw3.h"
#include <chrono>
#include <thread>

#include "pdf.h"
#include "External/IMGui/imgui.h"
#include "External/IMGui/imgui_impl_win32.h"

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

color ray_color(const ray& r, const color& background, const hittable& world,  shared_ptr<hittable>& lights, int depth) {
	hit_record rec;

	if (depth <= 0)
	{
		return color(0, 0, 0);
	}

	if (!world.hit(r, 0.001, infinity, rec))
	{
		return background;
	}

	ray scattered;
	color attenuation;
	color emitted = rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);
	double pdf_val;
	color albedo;
	if (!rec.mat_ptr->scatter(r, rec, albedo, scattered, pdf_val))
	{
		return emitted;
	}
    auto p0 = make_shared<hittable_pdf>(lights, rec.p);
    auto p1 = make_shared<cosine_pdf>(rec.normal);
    mixture_pdf mixed_pdf(p0, p1);
    scattered = ray(rec.p, mixed_pdf.generate(), r.time());
    pdf_val = mixed_pdf.value(scattered.direction());

	return emitted + albedo * rec.mat_ptr->scattering_pdf(r, rec, scattered) * ray_color(scattered, background, world, lights, depth - 1) / pdf_val;
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
	auto earth_texture = make_shared<image_texture>("./earthmap.jpg");
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
	objects.add(make_shared<flip_face>(make_shared<xz_rect>(213, 343, 227, 332, 554, light)));
	objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
	objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));
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

hittable_list cornell_smoke()
{
	hittable_list objects;

	auto red = make_shared<lambertian>(color(.65, .05, .05));
	auto white = make_shared<lambertian>(color(.73, .73, .73));
	auto green = make_shared<lambertian>(color(.12, .45, .15));
	auto light = make_shared<diffuse_light>(color(7, 7, 7));

	objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
	objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
	objects.add(make_shared<xz_rect>(113, 443, 127, 432, 554, light));
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
	objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

	shared_ptr<hittable> box1 = make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
	box1 = make_shared<rotate_y>(box1, 15);
	box1 = make_shared<translate>(box1, vec3(265, 0, 295));

	shared_ptr<hittable> box2 = make_shared<box>(point3(0, 0, 0), point3(165, 165, 165), white);
	box2 = make_shared<rotate_y>(box2, -18);
	box2 = make_shared<translate>(box2, vec3(130, 0, 65));

	objects.add(make_shared<constant_medium>(box1, 0.01, color(0, 0, 0)));
	objects.add(make_shared<constant_medium>(box2, 0.01, color(1, 1, 1)));

	return objects;
}

hittable_list final_scene() {
	hittable_list boxes1;
	auto ground = make_shared<lambertian>(color(0.48, 0.83, 0.53));
	const int boxes_per_side = 20;
	for (int i = 0; i < boxes_per_side; i++)
	{
		for (int j = 0; j < boxes_per_side; j++)
		{
			auto w = 100.0;
			auto x0 = -1000.0 + i * w;
			auto z0 = -1000.0 + j * w;
			auto y0 = 0.0;
			auto x1 = x0 + w;
			auto y1 = random_double(1, 101);
			auto z1 = z0 + w;
			boxes1.add(make_shared<box>(point3(x0, y0, z0), point3(x1, y1, z1), ground));
		}
	}

	hittable_list objects;
	objects.add(make_shared<bvh_node>(boxes1, 0, 1));
	auto light = make_shared<diffuse_light>(color(7, 7, 7));
	objects.add(make_shared<xz_rect>(123, 423, 147, 412, 554, light));
	auto center1 = point3(400, 400, 200);
	auto center2 = center1 + vec3(30, 0, 0);
	auto moving_sphere_material = make_shared<lambertian>(color(0.7, 0.3, 0.1));
	objects.add(make_shared<moving_sphere>(center1, center2, 0, 1, 50, moving_sphere_material));
	objects.add(make_shared<sphere>(point3(260, 150, 45), 50, make_shared<dielectric>(1.5)));
	objects.add(make_shared<sphere>(point3(0, 150, 145), 50, make_shared<metal>(color(0.8, 0.8, 0.9), 1.0)));
	auto boundary = make_shared<sphere>(point3(360, 150, 145), 70, make_shared<dielectric>(1.5));
	objects.add(boundary);
	objects.add(make_shared<constant_medium>(boundary, 0.2, color(0.2, 0.4, 0.9)));
	boundary = make_shared<sphere>(point3(0, 0, 0), 5000, make_shared<dielectric>(1.5));
	objects.add(make_shared<constant_medium>(boundary, .0001, color(1, 1, 1)));
	auto emat = make_shared<lambertian>(make_shared<image_texture>("earthmap.jpg"));
	objects.add(make_shared<sphere>(point3(400, 200, 400), 100, emat));
	auto pertext = make_shared<noise_texture>(0.1);
	objects.add(make_shared<sphere>(point3(220, 280, 300), 80, make_shared<lambertian>(pertext)));
	hittable_list boxes2;
	auto white = make_shared<lambertian>(color(.73, .73, .73));
	int ns = 1000;
	for (int j = 0; j < ns; j++)
	{
		boxes2.add(make_shared<sphere>(point3::random(0, 165), 10, white));
	}
	objects.add(make_shared<translate>(make_shared<rotate_y>(make_shared<bvh_node>(boxes2, 0.0, 1.0), 15), vec3(-100, 270, 395)));

	return objects;
}

void CalcTimeRemaining(int current, int mult, long long count)
{
	int hours, minutes, seconds;
	int timeLeft = (int)(mult - current) * ((int)count / current);
	seconds = timeLeft;
	if (timeLeft < 60)
	{
		printf("<1 min.");//, timeLeft);
		//printf("%i sec.", timeLeft);
	}
	else if (timeLeft < 60 * 60)
	{
		minutes = seconds / 60;
		seconds = timeLeft - (minutes * 60);
		printf("%i min. %i sec.", minutes, seconds);
	}
	else
	{
		minutes = seconds / 60;
		hours = (minutes / 60) - minutes;
		seconds = timeLeft - (minutes * 60);
		printf("%i hr. %i min. %i sec.", hours, minutes, seconds);
	}
	//printf("%4.4f sec", timeLeft);
}

//Adapted from https://www.geeksforgeeks.org/how-to-create-a-command-line-progress-bar-in-c-c/
void loadingBar(int current, int mult, long long count)
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
	printf("\n");
	printf("\t\t\t\t\t");
	printf("\n");
	printf("\t\t\t\t\t");
	printf("%4.4f %%", temp * 100);
	printf("\n\n");
	printf("\t\t\t\t\t");
	if (current == 0)
	{
		current = 1;
	}
	CalcTimeRemaining(current, mult, count);
}

//Clears screens with commands for Windows, Linux, macOS
void ClearScreen()
{
	system("clear");
	system("cls");
	std::cout << "\033[2J\033[1;1H";
}

GLint GL_CLAMP_TO_EDGE;

GLuint* out_texture;

int* out_width;

int* out_height;

//bool LoadTextureFromFile(const char* filename, GLuint* my_image_texture, int* my_image_width, int* my_image_height)
//{
//	// Load from file
//	int image_width = 0;
//	int image_height = 0;
//	unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
//	if (image_data == NULL)
//		return false;
//
//	// Create a OpenGL texture identifier
//	GLuint image_texture;
//	glGenTextures(1, &image_texture);
//	glBindTexture(GL_TEXTURE_2D, image_texture);
//
//	// Setup filtering parameters for display
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same
//
//	// Upload pixels into texture
//#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
//	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
//#endif
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
//	stbi_image_free(image_data);
//
//	*out_texture = image_texture;
//	*out_width = image_width;
//	*out_height = image_height;
//
//	return true;
//}

void colorCalc(int startValue, int endValue, int image_width, int samples_per_pixel, int max_depth, hittable_list world, color background, std::chrono::steady_clock::time_point begin, const int image_height, camera cam, int total, shared_ptr <hittable> lights)
{
	for (int j = startValue; j >= endValue; --j)
	{
		//std::cerr << "\rScanlines remaining: " << j << ' ' << '\n' << std::flush;
		ClearScreen();
		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		loadingBar(index, total, std::chrono::duration_cast<std::chrono::seconds>(now - begin).count());
		for (int i = 0; i < image_width; ++i)
		{
			//RGB* temp = new RGB;
			color pixel_color(0, 0, 0);
			for (int s = 0; s < samples_per_pixel; ++s)
			{
				auto u = double(i + random_double()) / (image_width - 1);
				auto v = double(j + random_double()) / (image_height - 1);
				ray r = cam.get_ray(u, v);
				pixel_color += ray_color(r, background, world, lights, max_depth);
			}
			auto temp = write_color(std::cout, pixel_color, samples_per_pixel);
			auto dataIndex = (j * image_width) + i;
			if (dataIndex < image_width * image_height)
			{
				data[dataIndex].R = temp.R;
				data[dataIndex].G = temp.G;
				data[dataIndex].B = temp.B;
			}
		}

		if (index <= total)
		{
			index++;
		}
	}
}

void threadRun(int image_width, int samples_per_pixel, int max_depth, hittable_list world, color background, std::chrono::steady_clock::time_point begin, const int image_height, RGB* data, int index, camera cam, int total, shared_ptr
               <hittable> lights)
{
	std::vector<std::thread> threads(num_threads);
	int countTab = image_height / num_threads;
	for (size_t i = 0; i < num_threads; ++i)
	{
		threads.emplace_back(
			colorCalc, countTab * (i + 1), countTab * i, image_width,
			samples_per_pixel, max_depth, world, background, begin,
			image_height, cam, total, lights);
	}

	for (auto& thread : threads) {
		if (thread.joinable()) {
			thread.join();
			thread.hardware_concurrency();
		}
	}
}

int main()
{
	/*IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::ShowDemoWindow();*/
	//ImGui_ImplWin32_Init(nullptr);
	//ImGui::StyleColorsDark();
	//ImGui::ShowMetricsWindow();

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
		//default:
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
	default:
	case 6:
		world = cornell_box();
		aspect_ratio = 1.0;
		image_width = 600;
		samples_per_pixel = 200;
		background = color(0, 0, 0);
		lookfrom = point3(278, 278, -800);
		lookat = point3(278, 278, 0);
		vfov = 40.0;
		break;
	case 7:
		world = cornell_smoke();
		aspect_ratio = 1.0;
		image_width = 600;
		samples_per_pixel = 200;
		lookfrom = point3(278, 278, -800);
		lookat = point3(278, 278, 0);
		vfov = 40.0;
		break;
	case 8:
		world = final_scene();
		aspect_ratio = 1.0;
		image_width = 800;
		samples_per_pixel = 10000;
		background = color(0, 0, 0);
		lookfrom = point3(478, 278, -600);
		lookat = point3(278, 278, 0);
		vfov = 40.0;
		break;
	}
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	const vec3 vup(0, 1, 0);
	const auto dist_to_focus = 10.0;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	data = (RGB*)malloc(image_height * image_width * sizeof(RGB));
	index = 0;
	camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);
	  shared_ptr<hittable> lights = make_shared<xz_rect>(213, 343, 227, 332, 554, shared_ptr<material>());
	int total = image_height - 1;
	threadRun(image_width, samples_per_pixel, max_depth, world, background, begin, image_height, data, index, cam, total, lights);
	stbi_flip_vertically_on_write(true);
	stbi_write_jpg("raytrace_02.jpg", image_width, image_height, sizeof(RGB), data, 100);

	/*int my_image_width = 0;
	int my_image_height = 0;
	GLuint my_image_texture = 0;
	bool ret = LoadTextureFromFile("raytrace_02.jpg", &my_image_texture, &image_width, &my_image_height);
	IM_ASSERT(ret);
		ImGui::Begin("OpenGL Texture Text");
	ImGui::Text("pointer = %p", my_image_texture);
	ImGui::Text("size = %d x %d", my_image_width, my_image_height);
	ImGui::Image((void*)(intptr_t)my_image_texture, ImVec2(my_image_width, my_image_height));
	ImGui::End();*/
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	int totalTime = (int)std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
	std::cerr << "\nDone.\n";
	std::cerr << "\nTotal Time:\n";
	int hours, minutes, seconds;
	seconds = totalTime;
	if (totalTime < 60)
	{
		printf("%i sec.", totalTime);
	}
	else if (totalTime < 60 * 60)
	{
		minutes = seconds / 60;
		seconds = totalTime - (minutes * 60);
		printf("%i min. %i sec.", minutes, seconds);
	}
	else
	{
		minutes = seconds / 60;
		hours = minutes / 60;
		seconds = totalTime - (minutes * 60);
		printf("%i hr. %i min. %i sec.", hours, minutes, seconds);
	}
}