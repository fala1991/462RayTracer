/**
 * @file texture.hpp
 * @brief Represents a matrix of rgb values
 *
 * @author Nathan Dobson (ndobson)
 */

#ifndef _462_SCENE_TEXTURE_HPP_
#define _462_SCENE_TEXTURE_HPP_

#include "math/color.hpp"
#include "math/vector.hpp"
#include "application/opengl.hpp"
#include <string>
#include "application/imageio.hpp"

namespace _462 {
    class Texture{
        public:
        Texture(){
            width=0;height=0;data=NULL;
        }
        std::string filename;
        int width;
        int height;
        unsigned char * data;
        /// returns the raw texture data
        const unsigned char* get_texture_data() const;
        
        /// puts the dimensions into width and height
        void get_texture_size( int* width, int* height ) const;
        
        /**
         * returns the color of the (x,y) pixel, where
         * x ranges from [0, width-1] and y ranges from [0, height-1].
         * Returns white if there is no texture.
         */
        Color3 get_texture_pixel( int x, int y ) const;
        Color3 sample(Vector2 coord) const;
		Color3 sample(real_t x, real_t y) const;
        bool load();
    };
}

#endif /*_462_SCENE_TEXTURE_HPP_*/