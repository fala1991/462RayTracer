/**
 * @file sphere.cpp
 * @brief Function defnitions for the Sphere class.
 *
 * @author Kristin Siu (kasiu)
 * @author Eric Butler (edbutler)
 */

#include "scene/sphere.hpp"
#include "application/opengl.hpp"
#include <algorithm>

namespace _462 {

#define SPHERE_NUM_LAT 80
#define SPHERE_NUM_LON 100

#define SPHERE_NUM_VERTICES ( ( SPHERE_NUM_LAT + 1 ) * ( SPHERE_NUM_LON + 1 ) )
#define SPHERE_NUM_INDICES ( 6 * SPHERE_NUM_LAT * SPHERE_NUM_LON )
// index of the x,y sphere where x is lat and y is lon
#define SINDEX(x,y) ((x) * (SPHERE_NUM_LON + 1) + (y))
#define VERTEX_SIZE 8
#define TCOORD_OFFSET 0
#define NORMAL_OFFSET 2
#define VERTEX_OFFSET 5
#define BUMP_FACTOR 1

static unsigned int Indices[SPHERE_NUM_INDICES];
static float Vertices[VERTEX_SIZE * SPHERE_NUM_VERTICES];

static void init_sphere()
{
    static bool initialized = false;
    if ( initialized )
        return;

    for ( int i = 0; i <= SPHERE_NUM_LAT; i++ ) {
        for ( int j = 0; j <= SPHERE_NUM_LON; j++ ) {
            real_t lat = real_t( i ) / SPHERE_NUM_LAT;
            real_t lon = real_t( j ) / SPHERE_NUM_LON;
            float* vptr = &Vertices[VERTEX_SIZE * SINDEX(i,j)];

            vptr[TCOORD_OFFSET + 0] = lon;
            vptr[TCOORD_OFFSET + 1] = 1-lat;

            lat *= PI;
            lon *= 2 * PI;
            real_t sinlat = sin( lat );

            vptr[NORMAL_OFFSET + 0] = vptr[VERTEX_OFFSET + 0] = sinlat * sin( lon );
            vptr[NORMAL_OFFSET + 1] = vptr[VERTEX_OFFSET + 1] = cos( lat ),
            vptr[NORMAL_OFFSET + 2] = vptr[VERTEX_OFFSET + 2] = sinlat * cos( lon );
        }
    }

    for ( int i = 0; i < SPHERE_NUM_LAT; i++ ) {
        for ( int j = 0; j < SPHERE_NUM_LON; j++ ) {
            unsigned int* iptr = &Indices[6 * ( SPHERE_NUM_LON * i + j )];

            unsigned int i00 = SINDEX(i,  j  );
            unsigned int i10 = SINDEX(i+1,j  );
            unsigned int i11 = SINDEX(i+1,j+1);
            unsigned int i01 = SINDEX(i,  j+1);

            iptr[0] = i00;
            iptr[1] = i10;
            iptr[2] = i11;
            iptr[3] = i11;
            iptr[4] = i01;
            iptr[5] = i00;
        }
    }

    initialized = true;
}

Sphere::Sphere()
    : radius(0), material(0) {}

Sphere::~Sphere() {}

bool Sphere::initialize(){
	Geometry::initialize();

	//set the bounding box
	int a[8][3] = { { 1, 1, 1 }, { -1, -1, -1 },
					{ -1, 1, 1 }, { 1, -1, 1 }, { 1, 1, -1 }, 
					{ -1, -1, 1 }, { -1, 1, -1 }, { 1, -1, -1 } 
					};
	Vector3 lower = mat.transform_point(Vector3(radius,radius,radius));
	Vector3 upper = lower;
	for (size_t i = 1; i < 8; i++){
		real_t x = radius * a[i][0];
		real_t y = radius * a[i][1];
		real_t z = radius * a[i][2];
		Vector3 pos = mat.transform_point(Vector3(x, y, z));
		if (pos.x > upper.x) upper.x = pos.x;
		if (pos.y > upper.y) upper.y = pos.y;
		if (pos.z > upper.z) upper.z = pos.z;
		
		if (pos.x < lower.x) lower.x = pos.x;
		if (pos.y < lower.y) lower.y = pos.y;
		if (pos.z < lower.z) lower.z = pos.z;
	}
	box = Bound(lower,upper);

	return true;
}

void Sphere::render() const
{
    // create geometry if we haven't already
    init_sphere();

    if ( material )
        material->set_gl_state();

    // just scale by radius and draw unit sphere
    glPushMatrix();
    glScaled( radius, radius, radius );
    glInterleavedArrays( GL_T2F_N3F_V3F, VERTEX_SIZE * sizeof Vertices[0], Vertices );
    glDrawElements( GL_TRIANGLES, SPHERE_NUM_INDICES, GL_UNSIGNED_INT, Indices );
    glPopMatrix();

    if ( material )
        material->reset_gl_state();
}


//quadratic formula
//If a solution exists, returns answers in x1 and x2, and returns true.
//Otherwise, returns false
bool solve_quadratic(real_t *x1,real_t *x2, real_t a, real_t b, real_t c){
    real_t b24ac = b*b-4*a*c;
    if(b24ac<0){
        return false;
    }else{
        real_t sb24ac=sqrt(b24ac);
        *x1=(-b+sb24ac)/(2*a);
        *x2=(-b-sb24ac)/(2*a);
        return true;
    }
}


//solve a quadratic equation, and then return the smallest solution larger than EPS
//if there is no solution, return -1
real_t solve_time(real_t a,real_t b,real_t c){
    real_t x1;
    real_t x2;
    if(solve_quadratic(&x1,&x2,a,b,c)){
        if(x1>EPS && x2>EPS){
            return std::min(x1,x2);
        }else if(x1>EPS){
            return x1;
        }else if(x2>EPS){
            return x2;
        }
    }
    return -1;
}


bool Sphere::intersect_test(const Ray& r, real_t& t, Intersection& info){
	Ray local_r = to_local(r);
	t = solve_time(local_r.d * local_r.d, 2.0f * local_r.d * local_r.e, local_r.e * local_r.e - radius * radius);
    if(t > EPS){

		info.ambient = material->ambient;
		info.diffuse = material->diffuse;
		info.specular = material->specular;
		info.shininess = material->shininess;
		Vector3 local_pos = local_r.atTime(t);
		info.position = mat.transform_point(local_pos);
		//for sphere, the local_pos is it's normal vector;
		info.normal = normalize(normMat * local_pos);
		info.refractive_index = material->refractive_index;

		//caculate the uv coordinate of hit point

		real_t theta = std::acos(local_pos.y / radius);
		real_t phi = std::atan2(-local_pos.x, -local_pos.z) + PI;
		Vector2 tex_coor = Vector2(phi / (2 * PI),  (PI - theta) / PI);
		info.tex_Color = material->texture.sample(tex_coor);

		return true;
    }
    return false;
}

bool Sphere::shadow_test(const Ray& r, real_t dis){
	if (!box.intersects(r)){
		return false;
	}

	Ray local_r = to_local(r);
	real_t t = solve_time(local_r.d * local_r.d, 2.0f * local_r.d * local_r.e, local_r.e * local_r.e - radius * radius);
	return  t > EPS && t < dis;
}

} /* _462 */

