#include "cubemap.hpp"

namespace _462 {

	Cubemap::Cubemap(std::string file)
	{
		filename = file;
	}

	const std::string Cubemap::tex_name[6] = { "posx", "negx", "posy", "negy", "posz", "negz" };

	// load cube map texture
	bool Cubemap::load(){
		// if no texture, nothing to do
		if (filename.empty())
			return true;
		std::cout << "Load cubemap " << filename << "...\n";
		std::string pre_path = "cubemaps/" + filename + "/";
		
		bool res = true;

		for (int i = 0; i < 6; i++){
			texture[i].filename = pre_path + tex_name[i] + ".png";
			res = res && texture[i].load();
		}
		return res;
	}

	//get the index of texture we need to sample, and it's coordinate
	void Cubemap::index_cubemap(Vector3 d, size_t& face, real_t& s, real_t& t){
		Vector3 absd;
		float sc = 0.0f, tc = 0.0f, ma = 1.0f;

		absd.x = fabs(d.x);
		absd.y = fabs(d.y);
		absd.z = fabs(d.z);
		face = 0;
		if ((absd.x >= absd.y) && (absd.x >= absd.z)) {
			if (d.x > 0.0f) {
				face = 0;
				sc = -d.z; tc = d.y; ma = absd.x;
			}
			else {
				face = 1;
				sc = d.z; tc = d.y; ma = absd.x;
			}
		}

		if ((absd.y >= absd.x) && (absd.y >= absd.z)) {
			if (d.y > 0.0f) {
				face = 2;
				sc = d.x; tc = -d.z; ma = absd.y;
			}
			else {
				face = 3;
				sc = d.x; tc = d.z; ma = absd.y;
			}
		}

		if ((absd.z >= absd.x) && (absd.z >= absd.y)) {
			if (d.z > 0.0f) {
				face = 4;
				sc = d.x; tc = d.y; ma = absd.z;

			}
			else {
				face = 5;
				sc = -d.x; tc = d.y; ma = absd.z;
			}
		}

		if (ma == 0.0f) {
			s = 0.0f;
			t = 0.0f;
		}
		else {
			s = ((sc / ma) + 1.0f) * 0.5f;
			t = ((tc / ma) + 1.0f) * 0.5f;
		}
	}

	//
	Color3 Cubemap::texCube(Vector3 d){
		size_t face;
		real_t s, t;
		index_cubemap(d, face, s, t);
		assert(face < 6);
		return texture[face].sample(s, t);
	}

}
