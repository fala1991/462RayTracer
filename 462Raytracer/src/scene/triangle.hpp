/**
 * @file triangle.hpp
 * @brief Class definition for Triangle.
 *
 * @author Eric Butler (edbutler)
 */

#ifndef _462_SCENE_TRIANGLE_HPP_
#define _462_SCENE_TRIANGLE_HPP_

#include "scene/scene.hpp"

namespace _462 {

    
    //get the barycentric coordinate
void barycentric_coor(const Vector3& p, const Vector3& a, const Vector3& b, const Vector3& c, real_t &u, real_t &v, real_t &w);
    //solve the ray triangle linear function
bool solve_raytri(const Ray& r, const Vector3& at, const Vector3& bt, const Vector3& ct, real_t& t);
    
/**
 * a triangle geometry.
 * Triangles consist of 3 vertices. Each vertex has its own position, normal,
 * texture coordinate, and material. These should all be interpolated to get
 * values in the middle of the triangle.
 * These values are all in local space, so it must still be transformed by
 * the Geometry's position, orientation, and scale.
 */
class Triangle : public Geometry
{
public:

    struct Vertex
    {
        // note that position and normal are in local space
        Vector3 position;
        Vector3 normal;
        Vector2 tex_coord;
        const Material* material;
    };

    // the triangle's vertices, in CCW order
    Vertex vertices[3];

    Triangle();
    virtual ~Triangle();
	virtual bool initialize();
    virtual void render() const;
	virtual bool intersect_test(const Ray& r, real_t& t, Intersection& info);
	virtual bool shadow_test(const Ray &r, real_t dis);
	void gen_bound_box();
};


} /* _462 */

#endif /* _462_SCENE_TRIANGLE_HPP_ */
