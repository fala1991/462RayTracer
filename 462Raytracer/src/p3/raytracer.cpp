/**
* @file raytacer.cpp
* @brief Raytracer class
*
* A basic Raytracer class with reflection, refraction, gloss
* and depth of field effect.
*
* @author Yiling Chen(yilingc)
*/

#include "raytracer.hpp"
#include "scene/scene.hpp"
#include "math/quickselect.hpp"
#include "p3/randomgeo.hpp"
#include <SDL_timer.h>

namespace _462 {

#define MAX_RECURSIVE_DEPTH 3

//number of rows to render before updating the result
static const unsigned STEP_SIZE = 1;
static const unsigned CHUNK_SIZE = 1;

Raytracer::Raytracer() {
        scene = 0;
        width = 0;
        height = 0;
    }

Raytracer::~Raytracer() { }

/**
 * Initializes the raytracer for the given scene. Overrides any previous
 * initializations. May be invoked before a previous raytrace completes.
 * @param scene The scene to raytrace.
 * @param width The width of the image being raytraced.
 * @param height The height of the image being raytraced.
 * @return true on success, false on error. The raytrace will abort if
 *  false is returned.
 */
bool Raytracer::initialize(Scene* scene, size_t num_samples,
               size_t width, size_t height, const RaytracerOptions& opt)
{
    printf("Initializing Raytracer... \n");

    this->scene = scene;
    this->num_samples = num_samples;
    this->width = width;
    this->height = height;

    current_row = 0;

    projector.init(scene->camera);
    scene->initialize();
    photonMap.initialize(scene);
	bvh_root = scene->gen_bvh_tree();
    gloss = opt.gloss;
    
    return true;
}

/**
* Trace a ray at giving depth, include reflection color 
* and refraction color.
* @param ray The ray to trace.
* @param depth Current recursive depth.
* @return the result color of the input ray.
*/
Color3 Raytracer::trace_ray(Ray &ray, size_t depth){
	if (depth > MAX_RECURSIVE_DEPTH) return Color3::Black();

	//Geometry* const * geoms = scene->get_geometries();
	real_t min_t = INFINITY;
	//size_t min_i = 0;
	Intersection info = default_intersection();

	bvh_root->intersect_test(ray, min_t, info);
	if (min_t != INFINITY){
		// caculate reflection color
		Vector3 reflect_dir = ray.d - (real_t)(2) * (ray.d * info.normal) * info.normal;

		// gloss effect
        if(gloss > EPS)
            reflect_dir += random_orthnormal_square(reflect_dir,gloss);

		Ray reflect_ray = Ray(info.position, normalize(reflect_dir));
		Vector3 refract_dir;
		real_t c = (real_t)0;
        real_t one = (real_t)1;
		// if specular color of material is black, then we don't need compute the reflection color
		Color3 reflect_color = info.specular == Color3::Black() ? Color3::Black() : info.specular * info.tex_Color * trace_ray(reflect_ray, depth + 1);

		if (info.refractive_index > EPS){
			// caculate refraction color
			real_t rafractive_ratio = info.refractive_index / scene->refractive_index;
			if (ray.d * info.normal < 0){
				// entering ray
				refract(ray.d, info.normal, rafractive_ratio, refract_dir);
				c = -ray.d * info.normal;
			}
			else{
				// leaving ray
				if (refract(ray.d, -info.normal, ec_reciprocal(rafractive_ratio), refract_dir)){
					c = refract_dir * info.normal;
				}
				else{
					return reflect_color;
				}
			}
			real_t R0 = (info.refractive_index - one) * (info.refractive_index - one)
				/ ((info.refractive_index + one) * (info.refractive_index + one));
			real_t R = R0 + (one - R0) * std::pow(one - c, (real_t)5);

			Ray refrac_ray = Ray(info.position, refract_dir);
			Color3 refract_color = refract_dir == Vector3::Zero() ? Color3::Black() : trace_ray(refrac_ray, depth + 1);
			return  R * reflect_color + (one - R) * refract_color;
		}
		else{
			// only have reflection color
            Color3 color = compute_illumination(info);
			return color + reflect_color;
		}
		
	}
	
	// rendering skybox
	if (scene->skybox != NULL){
		return scene->skybox->texCube(ray.d);
	}
	else{
		return scene->background_color;
	}
}


/**
* Compute lighting by giving intersection information
* @param info The intersection information include material and position
* @return result color of lighting
*/
Color3 Raytracer::compute_illumination(const Intersection& info){
	Color3 res = info.ambient * scene->ambient_light;
	const SphereLight* lights = scene->get_lights();
	for (size_t i = 0; i < scene->num_lights(); ++i){
		
		// basical blin-phone lighting
		const SphereLight& light = lights[i];
		Vector3 l = light.position - info.position;
		real_t d = length(l);
		real_t atten = (light.attenuation.constant
						+ d * light.attenuation.linear
						+ d * d * light.attenuation.quadratic);
        atten = real_t(1) / atten;
        
		real_t zero = (real_t)0;

		// shadow test, for soft shadow effect, will emit several rays with shadow test.
		// Blend result based on the percentage of rays pass the shadow test.
        real_t b = real_t(0);
        for (size_t si = 0; si < DIRECT_SAMPLE_COUNT; ++si){
            Vector3 soft_light_position = l + random_sphere() * light.radius;
            Ray s_r = Ray(info.position, normalize(soft_light_position));

			if (bvh_root->shadow_test(s_r, d)) b++;
        }
        b /= (real_t)DIRECT_SAMPLE_COUNT;
        b = real_t(1) - b;
		res += light.color * atten * b * info.diffuse * std::max(info.normal * normalize(l), zero);
	}
	return res * info.tex_Color;
}

/**
* Compute refraction direction of giving incoming direction
* @param dir The incoming ray direction
* @param norm The normal direction of rafraction point
* @param n The relative rafraction index of rafractin point
* @param t_dir The refracted direction
* @return False if there is a full reflection otherwise return True.
*/
bool Raytracer::refract(const Vector3& dir, const Vector3& norm, real_t n, Vector3& t_dir){
	real_t dn = dir * norm;
	real_t sq = (real_t)1 - (((real_t)1 - dn * dn) / (n * n));

	if (sq < 0) {
		// full reflection
		t_dir = Vector3::Zero();
		return false;
	}
	t_dir = (dir - norm * dn) / n - norm * std::sqrt(sq);
	t_dir = normalize(t_dir);
	return true;
}

/**
 * Performs a raytrace on the given pixel on the current scene.
 * The pixel is relative to the bottom-left corner of the image.
 * @param scene The scene to trace.
 * @param x The x-coordinate of the pixel to trace.
 * @param y The y-coordinate of the pixel to trace.
 * @param width The width of the screen in pixels.
 * @param height The height of the screen in pixels.
 * @return The color of that pixel in the final image.
 */
Color3 Raytracer::trace_pixel(size_t x,
                  size_t y,
                  size_t width,
                  size_t height)
{
    assert(x < width);
    assert(y < height);

    real_t dx = real_t(1)/width;
    real_t dy = real_t(1)/height;

    Color3 res = Color3::Black();

    for (unsigned int iter = 0; iter < num_samples; iter++)
    {
        // pick a point within the pixel boundaries to fire our
        // ray through.
        real_t i = real_t(2)*(real_t(x)+random_uniform())*dx - real_t(1);
        real_t j = real_t(2)*(real_t(y) + random_uniform())*dy - real_t(1);

        Ray r = Ray(scene->camera.get_position(), projector.get_pixel_dir(i, j));

		// Depth of View
        if(focus > EPS){
            real_t ap = (real_t)0.3f;
            Vector3 focus_point = r.atTime(focus);
            r.e += random_orthnormal_square(r.d, ap);
            r.d = normalize(focus_point - r.e);
        }
        
		res += trace_ray(r, 0);
    }
    return res*(real_t(1)/num_samples);
}


/**
* Find focus plane by giving screen point coordinate.
* @param x The x coordinate of screen point.
* @param y The y coordinate of screen point.
* @return void. Will set focus to the focus plane if find one. 
*  Otherwise set focus to INFINITY.
*/
void Raytracer::trace_focus(size_t x, size_t y){
    assert(x < width);
    assert(y < height);
    
    y = height - y;
    
    real_t dx = real_t(1)/width;
    real_t dy = real_t(1)/height;
    
    real_t i = real_t(2) * real_t(x) * dx - real_t(1);
    real_t j = real_t(2) * real_t(y) * dy - real_t(1);

    Ray r = Ray(scene->camera.get_position(), projector.get_pixel_dir(i, j));
    real_t t = INFINITY;
    Intersection info;
    
    bvh_root->intersect_test(r,t,info);
    if(t != INFINITY){
        std::cout<< "focus at : " << t<<std::endl;
        focus = t;
    }else{
        focus = real_t(0);
    }
}
    
/**
 * Raytraces some portion of the scene. Should raytrace for about
 * max_time duration and then return, even if the raytrace is not copmlete.
 * The results should be placed in the given buffer.
 * @param buffer The buffer into which to place the color data. It is
 *  32-bit RGBA (4 bytes per pixel), in row-major order.
 * @param max_time, If non-null, the maximum suggested time this
 *  function raytrace before returning, in seconds. If null, the raytrace
 *  should run to completion.
 * @return true if the raytrace is complete, false if there is more
 *  work to be done.
 */
bool Raytracer::raytrace(unsigned char* buffer, real_t* max_time)
{
    
    static const size_t PRINT_INTERVAL = 64;

    // the time in milliseconds that we should stop
    unsigned int end_time = 0;
    bool is_done;

    if (max_time)
    {
        // convert duration to milliseconds
        unsigned int duration = (unsigned int) (*max_time * 1000);
        end_time = SDL_GetTicks() + duration;
    }

    // until time is up, run the raytrace. we render an entire group of
    // rows at once for simplicity and efficiency.
    for (; !max_time || end_time > SDL_GetTicks(); current_row += STEP_SIZE)
    {
        // we're done if we finish the last row
        is_done = current_row >= height;
        // break if we finish
        if (is_done) break;

        int loop_upper = std::min(current_row + STEP_SIZE, height);

        for (int c_row = current_row; c_row < loop_upper; c_row++)
        {
            /*
             * This defines a critical region of code that should be
             * executed sequentially.
             */
#pragma omp critical
            {
                if (c_row % PRINT_INTERVAL == 0)
                    printf("Raytracing (Row %d)\n", c_row);
            }
            
        // This tells OpenMP that this loop can be parallelized.
#pragma omp parallel for schedule(dynamic, CHUNK_SIZE)
            for (int x = 0; x < width; x++)
            {
                // trace a pixel
                Color3 color = trace_pixel(x, c_row, width, height);
                // write the result to the buffer, always use 1.0 as the alpha
                color.to_array4(&buffer[4 * (c_row * width + x)]);
            }
#pragma omp barrier

        }
    }

    if (is_done) printf("Done raytracing!\n");

    return is_done;
}

} /* _462 */
