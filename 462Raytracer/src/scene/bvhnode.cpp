/**
* @file bvhnode.cpp
* @brief bounding volume tree class
*
* A bounding volume tree class with improved intersection test and shadow test.
*
* @author Yiling Chen(yilingc)
*/

#include "bvhnode.hpp"

namespace _462{
	/**
	* Build a bounding volume hierarchy tree by giving geometries 
	* @param geo_list The whole list of geometries we want to add to tree.
	* @param axis The x, y or z axis that current node use to divide child node.
	* @param start Start index of belonging geometries of current node. Inclusive.
	* @param end End index of belonging geometries of current node. Inclusive.
	* @return the bvh node.
	*/
	BvhNode::BvhNode(std::vector< Geometry* >& geo_list, int axis, size_t start, size_t end){
		size_t n = end - start + 1;
		if (n <= 0){
			left = NULL;
			right = NULL;
			return;
		}
		if (n == 1){
			left = geo_list[start];
			right = NULL;
			box = geo_list[start]->box;
		}
		else if (n == 2){
			left = geo_list[start];
			right = geo_list[end];
			box = Bound(left->box, right->box);
		}
		else {
			//find the midpoint of the box 
			Bound limit = geo_list[start]->box;
			for (size_t i = start + 1; i <= end; ++i){
				limit = Bound(limit, geo_list[i]->box);
			}
			Vector3 midpoint = limit.get_center();

			// partition geo_list into left and right by the midpoint
			size_t i = start, j = end;
			// quick partition
			while (i < j){
				switch (axis)
				{
				// use x axis to divide
				case 0:
					while (i < j && geo_list[i]->box.get_center().x <= midpoint.x)
					{
						i++;
					}
					while (i < j && geo_list[j]->box.get_center().x > midpoint.x){
						j--;
					}
					break;
				// use y axis to divide
				case 1:
					while (i < j && geo_list[i]->box.get_center().y <= midpoint.y)
					{
						i++;
					}
					while (i < j && geo_list[j]->box.get_center().y > midpoint.y){
						j--;
					}
					break;
				// use z axis to divide
				case 2:
					while (i < j && geo_list[i]->box.get_center().z <= midpoint.z)
					{
						i++;
					}
					while (i < j && geo_list[j]->box.get_center().z > midpoint.z){
						j--;
					}
					break;
				default:
					//should not go to here!!!
					assert(false);
					break;
				} /* end switch */
				if (i < j){
					// swap i, j 
					Geometry* g = geo_list[i];
					geo_list[i] = geo_list[j];
					geo_list[j] = g;
				}
			}/* end while*/

			// get new left node and right node

			left = new BvhNode(geo_list, (axis + 1) % 3, start, i - 1);
			right = new BvhNode(geo_list, (axis + 1) % 3, j, end);

			box = Bound(left->box,right->box);
		}
	}


	/**
	* Performs a recursively intersection test of giving ray
	* @param r The ray used to do intersection test
	* @param t Output intersection time t.
	* @param info Output intersection information.
	* @return True if find a intersection otherwise return False.
	*/
	bool BvhNode::intersect_test(const Ray& r, real_t& t, Intersection& info){
		if (box.intersects(r)){
			Intersection lrec, rrec;
			real_t lt, rt;
			bool left_hit = left != NULL && left->intersect_test(r, lt, lrec);
			bool right_hit = right != NULL && right->intersect_test(r, rt, rrec);
			if (left_hit && right_hit){
				if (lt < rt){
					t = lt;
					info = lrec;
				}
				else{
					t = rt;
					info = rrec;
				}
				return true;
			}
			else if (left_hit){
				t = lt;
				info = lrec;
				return true;
			}else if (right_hit){
				t = rt;
				info = rrec;
				return true;
			}
			else return false;
		}
		return false;
	}

	/**
	* Performs a recursively shadow test of giving ray
	* @param r The ray used to do intersection test
	* @param t Output intersection time t.
	* @return True if the start point of the ray is in the shadow.
	*/
	bool BvhNode::shadow_test(const Ray &r, real_t t){
		if (box.intersects(r)){

			bool left_hit = left != NULL && left->shadow_test(r, t);
			bool right_hit = right != NULL && right->shadow_test(r, t);
			if (left_hit || right_hit){
				return true;
			}
		}
		return false;
	}

	/**
	* Render function since it inherited from Geometry class which is abstract.
	*/
	void BvhNode::render()const { }

} /* 462 */