/**
* @file bvhnode.cpp
* @brief bounding volume tree class
*
* A bounding volume tree class with improved intersection test and shadow test.
*
* @author Yiling Chen(yilingc)
*/

#ifndef _462_SCENE_BVHNODE_HPP_
#define _462_SCENE_BVHNODE_HPP_
#include "scene/bound.hpp"
#include "scene/scene.hpp"

namespace _462 {
	class BvhNode : public Geometry{
	public:
		BvhNode(std::vector<Geometry* >& geo_list, int axis, size_t start, size_t end);
		virtual void render() const;

		virtual bool intersect_test(const Ray& r, real_t& t, Intersection& info);
		virtual bool shadow_test(const Ray &r, real_t dis);
	private:
		Geometry* left;
		Geometry* right;
	};

} /* 462 */

#endif