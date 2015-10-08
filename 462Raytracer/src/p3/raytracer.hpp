/**
* @file raytacer.cpp
* @brief Raytracer class
*
* A basic Raytracer class with reflection, refraction, gloss
* and depth of field effect.
*
* @author Yiling Chen(yilingc)
*/

#ifndef _462_RAYTRACER_HPP_
#define _462_RAYTRACER_HPP_

#include "math/color.hpp"
#include "math/random462.hpp"
#include "p3/photon.hpp"
#include "p3/neighbor.hpp"
#include "application/opengl.hpp"
#include "p3/photonmap.hpp"
#include "p3/util.hpp"
#include "scene/bvhnode.hpp"
namespace _462 {

class Scene;
class Ray;
struct Intersection;
    
struct RaytracerOptions{
    real_t focus;
    real_t gloss;
};
    
class Raytracer
{
public:
    PhotonMap photonMap;

    Raytracer();

    ~Raytracer();

    bool initialize(Scene* scene, size_t num_samples,
                    size_t width, size_t height, const RaytracerOptions& opt);
    Color3 trace_ray(Ray &ray, size_t depth);
    
    bool raytrace(unsigned char* buffer, real_t* max_time);
    
    void trace_focus(size_t x, size_t y);
    
    Color3 trace_pixel(size_t x,
               size_t y,
               size_t width,
               size_t height);
private:
    // the scene to trace
    Scene* scene;
    Projector projector;
    
    // the dimensions of the image to trace
    size_t width, height;

    // the next row to raytrace
    size_t current_row;

    unsigned int num_samples;
    
    // focus
    real_t focus;
    
    // gloss
    real_t gloss;

	// bvhtree root
	BvhNode* bvh_root;

	Color3 compute_illumination(const Intersection& info);
	bool refract(const Vector3& dir, const Vector3& norm, real_t n, Vector3& t_dir);
};

} /* _462 */

#endif /* _462_RAYTRACER_HPP_ */
