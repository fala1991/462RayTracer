/**
 * @file triangle.cpp
 * @brief Function definitions for the Triangle class.
 *
 * @author Eric Butler (edbutler)
 */

#include "scene/triangle.hpp"
#include "application/opengl.hpp"
#include "math/math.hpp"

namespace _462 {

Triangle::Triangle()
{
    vertices[0].material = 0;
    vertices[1].material = 0;
    vertices[2].material = 0;
    isBig=true;
}

Triangle::~Triangle() { }

bool Triangle::initialize(){
	Geometry::initialize();

	//set bound box;
	gen_bound_box();
	return true;
}

void Triangle::gen_bound_box(){
	//set bound box;
	Vector3 upper = mat.transform_point(vertices[0].position);
	Vector3 lower = upper;
	for (size_t i = 1; i < 3; ++i){
		Vector3 pos = mat.transform_point(vertices[i].position);
		if (pos.x < lower.x) lower.x = pos.x;
		if (pos.y < lower.y) lower.y = pos.y;
		if (pos.z < lower.z) lower.z = pos.z;

		if (pos.x > upper.x) upper.x = pos.x;
		if (pos.y > upper.y) upper.y = pos.y;
		if (pos.z > upper.z) upper.z = pos.z;
	}

	// incase the lower and upper is same, because bound box cound not be a plane or a line, 
	// otherwise there will be some bug in intersect test of bound box
	if (lower.x == upper.x) upper.x += EPS;

	if (lower.y == upper.y) upper.y += EPS;

	if (lower.z == upper.z) upper.z += EPS;

	box = Bound(lower, upper);
}

void Triangle::render() const
{
    bool materials_nonnull = true;
    for ( int i = 0; i < 3; ++i )
        materials_nonnull = materials_nonnull && vertices[i].material;

    // this doesn't interpolate materials. Ah well.
    if ( materials_nonnull )
        vertices[0].material->set_gl_state();

    glBegin(GL_TRIANGLES);

#if REAL_FLOAT
    glNormal3fv( &vertices[0].normal.x );
    glTexCoord2fv( &vertices[0].tex_coord.x );
    glVertex3fv( &vertices[0].position.x );

    glNormal3fv( &vertices[1].normal.x );
    glTexCoord2fv( &vertices[1].tex_coord.x );
    glVertex3fv( &vertices[1].position.x);

    glNormal3fv( &vertices[2].normal.x );
    glTexCoord2fv( &vertices[2].tex_coord.x );
    glVertex3fv( &vertices[2].position.x);
#else
    glNormal3dv( &vertices[0].normal.x );
    glTexCoord2dv( &vertices[0].tex_coord.x );
    glVertex3dv( &vertices[0].position.x );

    glNormal3dv( &vertices[1].normal.x );
    glTexCoord2dv( &vertices[1].tex_coord.x );
    glVertex3dv( &vertices[1].position.x);

    glNormal3dv( &vertices[2].normal.x );
    glTexCoord2dv( &vertices[2].tex_coord.x );
    glVertex3dv( &vertices[2].position.x);
#endif

    glEnd();

    if ( materials_nonnull )
        vertices[0].material->reset_gl_state();
}


bool Triangle::intersect_test(const Ray& r, real_t& t, Intersection& info){

	Ray local_r = to_local(r);
    t = -1;
	if (solve_raytri(local_r, vertices[0].position, vertices[1].position, vertices[2].position, t)){
		Vector3 local_pos = local_r.atTime(t);

		real_t a, b, c;

		barycentric_coor(local_pos, vertices[0].position, vertices[1].position, vertices[2].position, a, b, c);
		// compute intersection material info based on barycentric coordinate
		info.ambient = vertices[0].material->ambient * a + vertices[1].material->ambient * b + vertices[1].material->ambient * c;
		info.diffuse = vertices[0].material->diffuse * a + vertices[1].material->diffuse * b + vertices[1].material->diffuse * c;
		info.specular = vertices[0].material->specular * a + vertices[1].material->specular * b + vertices[1].material->specular * c;
		info.shininess = vertices[0].material->shininess * a + vertices[1].material->shininess * b + vertices[1].material->shininess * c;
		info.position = mat.transform_point(local_pos);
		info.normal = vertices[0].normal * a + vertices[1].normal * b + vertices[2].normal * c;
		info.normal = normalize(normMat * info.normal);
		info.refractive_index = vertices[0].material->refractive_index * a
			+ vertices[1].material->refractive_index * b
			+ vertices[2].material->refractive_index * c;
		//get the texture coordinate
		Vector2 tex_coord = vertices[0].tex_coord * a + vertices[1].tex_coord * b + vertices[2].tex_coord * c;
		//interpolate texture color
		Color3 c0 = vertices[0].material->texture.sample(tex_coord);
		Color3 c1 = vertices[1].material->texture.sample(tex_coord);
		Color3 c2 = vertices[2].material->texture.sample(tex_coord);
		info.tex_Color = c0 * a + c1 * b + c2 * c;
		return true;
	}
	return false;
}

bool Triangle::shadow_test(const Ray& r,real_t dis){
	if (!box.intersects(r)){
		return false;
	}

	Ray local_r = to_local(r);
	real_t t = -1;
	return solve_raytri(local_r, vertices[0].position, vertices[1].position, vertices[2].position, t) && t < dis;
}


bool solve_raytri(const Ray& r, const Vector3& v1, const Vector3& v2, const Vector3& v3, real_t& t){
	//initial linear solution matrix variable
	real_t a = v1.x - v2.x;
	real_t b = v1.y - v2.y;
	real_t c = v1.z - v2.z;
	real_t d = v1.x - v3.x;
	real_t e = v1.y - v3.y;
	real_t f = v1.z - v3.z;
	real_t g = r.d.x;
	real_t h = r.d.y;
	real_t i = r.d.z;
	real_t j = v1.x - r.e.x;
	real_t k = v1.y - r.e.y;
	real_t l = v1.z - r.e.z;

	real_t M = a * (e * i - h * f) + b * (g * f - d * i) + c * (d * h - e * g);
    if(M >=0 && M < EPS) return false;
	M = ec_reciprocal(M);
	//compute t
	t = (f * (a * k - j * b) + e * (j * c - a * l) + d * (b * l - k * c)) * M * -1;
	if (t < EPS) return false;

	//compute gamma
	real_t _gamma = (i * (a * k - j * b) + h * (j * c - a * l) + g * (b * l - k * c)) * M;
	if (_gamma < 0 || _gamma > 1) return false;

	//compute beta
	real_t _beta = (j * (e * i - h * f) + k *(g * f - d * i) + l * (d * h - e * g)) * M;
	if (_beta < 0 || _beta > 1 - _gamma) return false;

	return true;
}

//get the barycentric coordinate
void barycentric_coor(const Vector3& p, const Vector3& a, const Vector3& b, const Vector3& c, real_t &u, real_t &v, real_t &w)
{
	Vector3 v0 = b - a, v1 = c - a, v2 = p - a;
	real_t d00 = v0 * v0;
	real_t d01 = v0 * v1;
	real_t d11 = v1 * v1;
	real_t d20 = v2 * v0;
	real_t d21 = v2 * v1;
	real_t denom = d00 * d11 - d01 * d01;
	v = (d11 * d20 - d01 * d21) / denom;
	w = (d00 * d21 - d01 * d20) / denom;
	u = real_t(1) - v - w;
}

} /* _462 */
