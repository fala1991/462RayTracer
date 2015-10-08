/**
 * @file scene.hpp
 * @brief Class definitions for scenes.
 *
 */

#ifndef _462_SCENE_SCENE_HPP_
#define _462_SCENE_SCENE_HPP_

#include "math/vector.hpp"
#include "math/quaternion.hpp"
#include "math/matrix.hpp"
#include "math/camera.hpp"
#include "scene/material.hpp"
#include "scene/mesh.hpp"
#include "scene/cubemap.hpp"
#include "ray.hpp"
#include <string>
#include <vector>
#include <cfloat>
#include "scene/bound.hpp"

namespace _462 {
class BvhNode;

//represents an intersection between a ray and a geometry
struct Intersection{
	Vector3 position;
	Vector3 normal;
	Color3 tex_Color;
	Color3 ambient;
	Color3 diffuse;
	Color3 specular;
	real_t shininess;
	real_t refractive_index;
};

static Intersection default_intersection(){
	Intersection info;
	info.position = Vector3::Zero();
	info.normal = Vector3::Zero();
	info.tex_Color = Color3::Black();
	info.ambient = Color3::Black();
	info.diffuse = Color3::Black();
	info.specular = Color3::Black();
	info.shininess = 0;
	info.refractive_index = 0;
	return info;
}

//represents any additional intersection information
    
struct HitRecord{
    bool is_hit;
    //the hitted triangle index of mesh
    size_t triangle_index;
};


class Geometry
{
public:
    Geometry();
    virtual ~Geometry();
    /*
       World transformation are applied in the following order:
       1. Scale
       2. Orientation
       3. Position
    */

    // The world position of the object.
    Vector3 position;

    // The world orientation of the object.
    // Use Quaternion::to_matrix to get the rotation matrix.
    Quaternion orientation;

    // The world scale of the object.
    Vector3 scale;

    // Forward transformation matrix
    Matrix4 mat;

    // Inverse transformation matrix
    Matrix4 invMat;
    // Normal transformation matrix
    Matrix3 normMat;
	// Bounding box of the geometry
	Bound box;

    bool isBig;
    /**
     * Renders this geometry using OpenGL in the local coordinate space.
     */
    virtual void render() const = 0;

    virtual bool initialize();
	//intersection test function
	virtual bool intersect_test(const Ray& r, real_t& t, Intersection& rec) = 0;
	//shadow_test function
	virtual bool shadow_test(const Ray &r, real_t dis) = 0;
	//change to ray to it's local coordinate
	Ray to_local(const Ray& r);

};


struct SphereLight
{
    struct Attenuation
    {
        real_t constant;
        real_t linear;
        real_t quadratic;
    };

    SphereLight();

    bool intersect(const Ray& r, real_t& t);

    // The position of the light, relative to world origin.
    Vector3 position;
    // The color of the light (both diffuse and specular)
    Color3 color;
    // attenuation
    Attenuation attenuation;
    real_t radius;
};

/**
 * The container class for information used to render a scene composed of
 * Geometries.
 */
class Scene
{
public:

    /// the camera
    Camera camera;
    /// the background color
    Color3 background_color;
    /// the amibient light of the scene
    Color3 ambient_light;
    /// the refraction index of air
    real_t refractive_index;

	/// the environment cubemap of the scene
	Cubemap* skybox;

    /// Creates a new empty scene.
    Scene();

    /// Destroys this scene. Invokes delete on everything in geometries.
    ~Scene();

    bool initialize();

	BvhNode* gen_bvh_tree();

    // accessor functions
    Geometry* const* get_geometries() const;
    size_t num_geometries() const;
    const SphereLight* get_lights() const;
    size_t num_lights() const;
    Material* const* get_materials() const;
    size_t num_materials() const;
    Mesh* const* get_meshes() const;
    size_t num_meshes() const;

    /// Clears the scene, and invokes delete on everything in geometries.
    void reset();

    // functions to add things to the scene
    // all pointers are deleted by the scene upon scene deconstruction.
    void add_geometry( Geometry* g );
    void add_material( Material* m );
    void add_mesh( Mesh* m );
    void add_light( const SphereLight& l );
	
	// load environment map
	
    
private:

    typedef std::vector< SphereLight > SphereLightList;
    typedef std::vector< Material* > MaterialList;
    typedef std::vector< Mesh* > MeshList;
    typedef std::vector< Geometry* > GeometryList;

    // list of all lights in the scene
    SphereLightList point_lights;
    // all materials used by geometries
    MaterialList materials;
    // all meshes used by models
    MeshList meshes;
    // list of all geometries. deleted in dctor, so should be allocated on heap.
    GeometryList geometries;
private:

    // no meaningful assignment or copy
    Scene(const Scene&);
    Scene& operator=(const Scene&);

};
} /* _462 */

#endif /* _462_SCENE_SCENE_HPP_ */
