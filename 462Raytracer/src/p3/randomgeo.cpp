#include "randomgeo.hpp"
namespace _462{
//return a vector uniformily selected from the unit ball (not sphere)
Vector3 random_sphere(){
	real_t x, y, z;
	do{
		x = random_uniform() * real_t(2) - real_t(1);
		y = random_uniform() * real_t(2) - real_t(1);
		z = random_uniform() * real_t(2) - real_t(1);
	} while (x * x + y * y + z * z > real_t(1));
	return Vector3(x,y,z);
}

Vector3 random_hemisphere_indexed(real_t k,real_t n){
    real_t H = floor((-2+sqrt(4+32*n/PI))/(16/PI));
    real_t Y = floor((PI+4*H*(asin((1/(2*H))*(k-2*H)*sin(PI/(4*H)))))/(2*PI));
    real_t k0 = 2*H*(1 + (sin(PI*(-1+2*Y)/(4*H)))/sin(PI/(4*H)));
    real_t k1 = 2*H*(1 + (sin(PI*(1+2*Y)/(4*H)))/sin(PI/(4*H)));
    real_t X = (random_uniform()+floor(k-k0))/floor(k1-k0);
    real_t phi = (Y+random_uniform())/H*PI/2;
    real_t theta = X * 2 * PI;
    return Vector3(cos(phi)*cos(theta),cos(phi)*sin(theta),sin(phi));
}
    
/*
 * consider n vectors randomly distributed over the surface of a sphere
 * without clustering (clustering occurs with independently selected vectors).
 * For some fixed ordering, this function returns the kth vector out of the n.
 */
Vector3 random_sphere_indexed(int k,int n){
    if(k<n/2){
        return random_hemisphere_indexed(k,n/2);
    }else{
        return -random_hemisphere_indexed(k-n/2,n/2);
    }
}

//return a vector uniformily randomly selected from the volume of a hemisphere
Vector3 random_hemisphere(Vector3 d){
    assert(false);
	return Vector3::Zero();
}

Vector3 random_orthnormal_square(Vector3 d, real_t a){
	Vector3 w = normalize(d);
	real_t x, y, z, res;
	//find a not collinear vector t
	do{
		x = random_uniform() * real_t(2) - real_t(1);
		y = random_uniform() * real_t(2) - real_t(1);
		z = random_uniform() * real_t(2) - real_t(1);
		res = x * w.x + y * w.y + z * w.z;
	} while ((fabs(res) - real_t(1)) < EPS);
	Vector3 t = Vector3(x,y,z);
	Vector3 u = normalize(cross(t, w));
	Vector3 v = cross(w, u);
	x = a * (real_t)(-0.5f) + random_uniform() * a;
	y = a * (real_t)(-0.5f) + random_uniform() * a;
	return x * u + y * v;
}
}