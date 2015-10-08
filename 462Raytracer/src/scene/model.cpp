/**
 * @file model.cpp
 * @brief Model class
 *
 * @author Eric Butler (edbutler)
 * @author Zeyang Li (zeyangl)
 */

#include "scene/model.hpp"
#include "scene/material.hpp"
#include "application/opengl.hpp"
#include "scene/triangle.hpp"
#include "scene/bvhnode.hpp"
#include <iostream>
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>


namespace _462 {

Model::Model() : mesh( 0 ), material( 0 ), bvh_root(NULL) { }
Model::~Model() { }

void Model::render() const
{
    if ( !mesh )
        return;
    if ( material )
        material->set_gl_state();
    mesh->render();
    if ( material )
        material->reset_gl_state();
}


bool Model::initialize(){
	Geometry::initialize();

	//copy mesh triangles to a geometry triangles
	size_t tri_num = mesh->num_triangles();
	std::vector<Geometry* > triangle_list;
	triangle_list.reserve(tri_num);

	const MeshTriangle* triangles = mesh->get_triangles();
	const MeshVertex* vertices = mesh->get_vertices();

	for (size_t i = 0; i < tri_num; i++){
		Triangle* tri = new Triangle();
		for (size_t j = 0; j < 3; j++){
			tri->vertices[j].position = vertices[triangles[i].vertices[j]].position;
			tri->vertices[j].normal = vertices[triangles[i].vertices[j]].normal;
			tri->vertices[j].tex_coord = vertices[triangles[i].vertices[j]].tex_coord;
			tri->vertices[j].material = material;
		}

		tri->mat = mat;
		tri->invMat = invMat;
		tri->normMat = normMat;
		tri->gen_bound_box();
		triangle_list.push_back(tri);
	}

	//set internal bvh tree
	bvh_root = new BvhNode(triangle_list,0 , 0, tri_num - 1);
	box = bvh_root->box;
    return true;
}

bool Model::intersect_test(const Ray& r, real_t& t, Intersection& info){
	return bvh_root->intersect_test(r,t,info);
}

bool Model::shadow_test(const Ray& r,real_t dis){
	return bvh_root->shadow_test(r,dis);
}


} /* _462 */
