#ifndef __MCDRIVER_HPP__
#define __MCDRIVER_HPP__ 

#include <string>
#include <vector>
#include "driver_structs.h"
#include "mc_scanner.hpp"
#include "mc_parser.tab.hh"

#include <glm.hpp>

class Scene;

namespace MC {

	class MC_Driver {
		Scene *_scene;
		float *initRadius, *initVerticalAngle, *initHorizontalAngle, *initFov;

	public:
		MC_Driver();
		MC_Driver(Scene *scene, float *initRadius, float *initVerticalAngle,
			float *initHorizontalAngle, float *initFov);

		virtual ~MC_Driver();

		void parse(const char *filename);


		void add_view(vec3, vec3, vec3, float, float, ivec2);
		void add_back_col(vec3);
		void add_light(vec3);
		void add_light(vec3, vec3);
		void add_material(vec3, float, float, float, float, float);
		void add_cylinder(vec3, float, vec3, float);
		void add_sphere(vec3, float);
		void add_poly(int, std::vector<vec3>*);
		void add_poly_patch(int, std::vector<vec3>*);
		void add_plane(vec3, vec3, vec3);

	private:
		void print(std::string msg);

		MC::MC_Parser *parser;
		MC::MC_Scanner *scanner;
	};

} /* end namespace MC */
#endif /* END __MCDRIVER_HPP__ */
