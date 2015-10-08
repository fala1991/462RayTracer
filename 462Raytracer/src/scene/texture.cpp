//
//  texture.cpp
//  
//
//  Created by Nathan A. Dobson on 2/1/15.
//
//

#include "scene/texture.hpp"
namespace _462{
const unsigned char* Texture::get_texture_data() const
{
    return data;
}

void Texture::get_texture_size( int* w, int* h ) const
{
    assert( w && h );
    *w = this->width;
    *h = this->height;
}

Color3 Texture::get_texture_pixel( int x, int y ) const
{
    if(width<=0 || height<=0){
        return Color3::White();
    }
    x=x%width;
    while(x<0){
        x+=width;
    }
    y=y%height;
    while(y<0){
        y+=height;
    }
    assert(x>=0 && y>=0 && x<width && y<height);
    return data ? Color3( data + 4 * (x + y * width) ) : Color3::White();
}

bool Texture::load(){
    // if no texture, nothing to do
    if ( filename.empty() )
        return true;
    
    std::cout << "Loading texture " << filename << "...\n";
    
    // allocates data with malloc
    data = imageio_load_image( filename.c_str(), &width, &height );
    
    if ( !data ) {
        std::cerr << "Cannot load texture file " << filename << std::endl;
        return false;
    }
    std::cout << "Finished loading texture" << std::endl;
    return true;
}

Color3 Texture::sample(Vector2 coord) const{
	real_t u0 = fmod(coord.x, 1);
	real_t v0 = fmod(coord.y, 1);
	size_t i = std::floor(u0 * width);
	size_t j = std::floor(v0 * height);
	real_t u1 = width * u0 - i;
	real_t v1 = height * v0 - j;
	real_t u2 = 3 * std::pow(u1,2) - 2 * std::pow(u1,3);
	real_t v2 = 3 * std::pow(v1, 2) - 2 * std::pow(v1, 3);
	return (1 - u2) * (1 - v2) * get_texture_pixel(i, j) 
		+ u2 * (1 - v2) * get_texture_pixel(i + 1, j)
		+ (1 - u2) * v2 * get_texture_pixel(i, j + 1)
		 + u2 * v2 * get_texture_pixel(i + 1, j + 1);

}

Color3 Texture::sample(real_t x, real_t y) const{
	return sample(Vector2(x,y));
}
}