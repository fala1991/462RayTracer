/**
 * @file model.hpp
 * @brief Model class
 *
 * @author Eric Butler (edbutler)
 */

#ifndef _462_SCENE_MODEL_HPP_
#define _462_SCENE_MODEL_HPP_

#include "scene/scene.hpp"
#include "scene/mesh.hpp"
#include "scene/meshtree.hpp"

namespace _462 {

/**
 * A mesh of triangles.
 */
class Model : public Geometry
{
public:

    const Mesh* mesh;
    const MeshTree *tree;
    const Material* material;

    Model();
    virtual ~Model();

    virtual void render() const;
    virtual bool initialize();
	virtual bool intersect_test(const Ray& r, real_t& t, Intersection& rec);
	virtual bool shadow_test(const Ray &r, real_t dis);
private:
	BvhNode* bvh_root;
};


} /* _462 */

#endif /* _462_SCENE_MODEL_HPP_ */

