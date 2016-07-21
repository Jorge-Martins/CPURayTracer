#include <cctype>
#include <fstream>
#include <cassert>

#include "mc_driver.hpp"
#include "../Scene.h"

MC::MC_Driver::~MC_Driver() {
	delete(scanner);
	delete(parser);
}

MC::MC_Driver::MC_Driver() : parser(nullptr),
scanner(nullptr),
_scene(nullptr),
initRadius(nullptr),
initVerticalAngle(nullptr),
initHorizontalAngle(nullptr),
initFov(nullptr) {
}

MC::MC_Driver::MC_Driver(Scene *scene, float *initRadius, float *initVerticalAngle,
	float *initHorizontalAngle, float *initFov) :
	parser(nullptr),
	scanner(nullptr),
	_scene(scene),
	initRadius(initRadius),
	initVerticalAngle(initVerticalAngle),
	initHorizontalAngle(initHorizontalAngle),
	initFov(initFov) {
}

void MC::MC_Driver::parse(const char *filename) {
	assert(filename != nullptr);
	std::ifstream in_file(filename);
	if(!in_file.good()) exit(EXIT_FAILURE);

	delete(scanner);
	scanner = nullptr;
	scanner = new MC::MC_Scanner(&in_file);
	/* check to see if its initialized */
	assert(scanner != nullptr);

	delete(parser);
	parser = nullptr;
	parser = new MC::MC_Parser((*scanner) /* scanner */,
		(*this) /* driver */);
	assert(parser != nullptr);

	//parser->set_debug_level(1);

	const int accept(0);
	if(parser->parse() != accept) {
		std::cerr << "Parse failed!!\n";
	}
}

void MC::MC_Driver::add_view(vec3 _from, vec3 _at, vec3 _up, float fov, float hither, ivec2 res) {
	//print("Saw a view");
	glm::vec3 from(_from.x, _from.y, _from.z);
	glm::vec3 at(_at.x, _at.y, _at.z);
	glm::vec3 up(_up.x, _up.y, _up.z);

	float ha, va;

	if(initRadius != nullptr) {
		*initRadius = glm::length(at - from);
		*initFov = fov;


		if(up.x > 0) {
			ha = RAD2DEG * atanf(from.z / from.y);
			va = RAD2DEG * acosf(from.x / *initRadius);

		}
		else if(up.y > 0) {
			ha = RAD2DEG * atanf(from.x / from.z);
			va = RAD2DEG * acosf(from.y / *initRadius);

		}
		else {
			ha = RAD2DEG * atanf(from.y / from.x);
			va = RAD2DEG * acosf(from.z / *initRadius);
		}

		if(ha > 180) {
			ha -= 180;
		}
		else if(ha < 0) {
			ha += 180;
		}

		if(va > 360) {
			va -= 360;
		}
		else if(va < 0) {
			va += 360;
		}

		*initHorizontalAngle = ha;
		*initVerticalAngle = va;
	}

	_scene->addView(from, at, up, fov, float(res.x / (float)res.y));
}

void MC::MC_Driver::add_back_col(vec3 col) {
	//print("Saw background");
	glm::vec3 color(col.x, col.y, col.z);
	_scene->setBackColor(color);
}

void MC::MC_Driver::add_light(vec3 lPos) {
	//print("Saw light");
	glm::vec3 position(lPos.x, lPos.y, lPos.z);
	_scene->addLight(position);
}

void MC::MC_Driver::add_light(vec3 lPos, vec3 lCol) {
	//print("Saw ligh + col");
	glm::vec3 position(lPos.x, lPos.y, lPos.z), color(lCol.x, lCol.y, lCol.z);
	_scene->addLight(position, color);
}

void MC::MC_Driver::add_material(vec3 col, float diff, float spec, float shine, float t, float ior) {
	//print("Saw material");
	glm::vec3 color(col.x, col.y, col.z);
	_scene->setMaterial(color, diff, spec, shine, t, ior);
}

void MC::MC_Driver::add_cylinder(vec3 bCenter, float bRad, vec3 tCenter, float tRad) {
	//print("Saw cylinder");
	glm::vec3 base(bCenter.x, bCenter.y, bCenter.z), top(tCenter.x, tCenter.y, tCenter.z);
	_scene->addCylinder(base, top, tRad);
}

void MC::MC_Driver::add_sphere(vec3 center, float radius) {
	//print("Saw sphere");
	glm::vec3 middle(center.x, center.y, center.z);
	_scene->addSphere(middle, radius);
}

void MC::MC_Driver::add_poly(int nVerts, std::vector<vec3>* points) {
	//print("Saw poly");
	if(nVerts == points->size()) {
		std::vector<glm::vec3> verts;
		for(vec3 &v : *points)
			verts.push_back(glm::vec3(v.x, v.y, v.z));
		_scene->addPoly(nVerts, verts);
	}
	else {
		print("Num verts did not match... :(");
	}
	delete points;
}

void MC::MC_Driver::add_poly_patch(int nVerts, std::vector<vec3>*pointsAndNormals) {
	//print("Saw poly patch");
	if(2 * nVerts == pointsAndNormals->size()) {
		std::vector<glm::vec3> verts(pointsAndNormals->size() / 2);
		std::vector<glm::vec3> norms(pointsAndNormals->size() / 2);

		for(int vert = 0, size = (int) pointsAndNormals->size() / 2; vert < size; ++vert) {
			vec3 &p = (*pointsAndNormals)[vert * 2], &n = (*pointsAndNormals)[vert * 2 + 1];
			glm::vec3 pos(p.x, p.y, p.z), normal(n.x, n.y, n.z);
			verts.push_back(pos);
			norms.push_back(normal);
		}
		_scene->addPolyPatch(nVerts, verts, norms);
	}
	else {
		print("Num verts did not match... :(");
	}
	delete pointsAndNormals;
}

void MC::MC_Driver::add_plane(vec3 p1, vec3 p2, vec3 p3) {
	//print("Saw plane");
	glm::vec3 v1(p1.x, p1.y, p1.z), v2(p2.x, p2.y, p2.z), v3(p3.x, p3.y, p3.z);
	_scene->addPlane(v1, v2, v3);
}

void MC::MC_Driver::print(std::string msg) {
	std::cout << msg << std::endl;
}