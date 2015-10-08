/**
* @file cubemap.hpp
* @brief Represents six matrices of rgb values
*
* @author Yiling Chen (yilingc)
*/

#include "math/color.hpp"
#include "math/vector.hpp"
#include "scene/texture.hpp"
#include <string>

#ifndef _462_SCENE_CUBEMAP_HPP_
#define _462_SCENE_CUBEMAP_HPP_

namespace _462 {

	class Cubemap
	{
	public:
		std::string filename;
		Cubemap(std::string file);
		bool load();
		Color3 texCube(Vector3 d);
	private:
		//six faces' texture 
		Texture texture[6];
		const static std::string tex_name[6];
		void index_cubemap(Vector3 d, size_t& face, real_t& s, real_t& t);
	};
}/* _462 */

#endif