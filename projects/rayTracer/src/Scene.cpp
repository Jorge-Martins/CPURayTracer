#include "Scene.h"
#include "parsing/mc_driver.hpp"

Scene::Scene() : _camera(nullptr), _backGroung(), _material(), _shapes(), _lights(),
_cmin(glm::vec3(FLT_MAX)), _cmax(glm::vec3(-FLT_MAX)) {}

Scene::~Scene() {
	for(Shape *s : _shapes)
		delete s;
	for(Light *l : _lights)
		delete l;
	delete _camera;
}

Camera* Scene::getCamera() {
	return _camera;
}

std::vector<Shape *> &Scene::getShapes() {
	return _shapes;
}

std::vector<Shape *> &Scene::getPlanes() {
	return _planes;
}

std::vector<Light *> &Scene::getLights() {
	return _lights;
}

glm::vec3 Scene::getCmin() {
	return _cmin;
}

glm::vec3 Scene::getCmax() {
	return _cmax;
}

bool Scene::loadNff(std::string filePath, float *initRadius, float *initVerticalAngle,
	float *initHorizontalAngle, float *initFov) {

	std::cout << "Loading: " << filePath << std::endl;
	MC::MC_Driver driver(this, initRadius, initVerticalAngle, initHorizontalAngle, initFov);
	driver.parse(filePath.c_str());
	std::cout << std::endl;
	return true;
}

void Scene::addView(glm::vec3 from, glm::vec3 at, glm::vec3 up, float fov, float aspect) {
	if(_camera != nullptr) {
		std::cerr << "Scene: Camera already defined." << std::endl;
		return;
	}
	_camera = new Camera(from, at, up, fov, aspect);
}

void Scene::setBackColor(glm::vec3 color) {
	_backGroung = color;
}

glm::vec3 Scene::getBackColor() {
	return _backGroung;
}

void Scene::addLight(glm::vec3 pos) {
	_lights.push_back(new Light(pos, glm::vec3(1.0f)));
}

void Scene::addLight(glm::vec3 pos, glm::vec3 color) {
	_lights.push_back(new Light(pos, color));
}

void Scene::setMaterial(glm::vec3 color, float diffuse, float specular, float shine, float trans, float ior) {
	_material = Material(color, diffuse, specular, shine, trans, ior);
}

void Scene::addCylinder(glm::vec3 base, glm::vec3 top, float radius) {
	Cylinder *cyl = new Cylinder(base, top, radius);
	cyl->setMaterial(_material);
	_shapes.push_back(cyl);

	Extent e = cyl->getAAExtent();
	_cmin = glm::min(_cmin, e.min);
	_cmax = glm::max(_cmax, e.max);
}

void Scene::addSphere(glm::vec3 center, float radius) {
	Sphere *sp = new Sphere(center, radius);
	sp->setMaterial(_material);
	_shapes.push_back(sp);

	Extent e = sp->getAAExtent();
	_cmin = glm::min(_cmin, e.min);
	_cmax = glm::max(_cmax, e.max);
}

void Scene::addPoly(int numVerts, std::vector<glm::vec3> verts) {

	if(numVerts = 3) {
		Triangle *tri = new Triangle(verts);
		tri->setMaterial(_material);
		_shapes.push_back(tri);

		Extent e = tri->getAAExtent();
		_cmin = glm::min(_cmin, e.min);
		_cmax = glm::max(_cmax, e.max);
	}
	//TODO
}
void Scene::addPolyPatch(int numVerts, std::vector<glm::vec3> verts, std::vector<glm::vec3> normals) {
	//TODO
}

void Scene::addPlane(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
	Plane *pl = new Plane(p1, p2, p3);
	pl->setMaterial(_material);
	_planes.push_back(pl);
}


Camera::Camera(glm::vec3 from, glm::vec3 at, glm::vec3 up, float fov, float aspect) {
	this->from = from;
	this->at = at;
	this->up = up;
	this->fov = fov;
	this->aspect = aspect;
	computeFrame();
	computeHitherDimensions();
}

void Camera::update(glm::vec3 from, float fov) {
	this->fov = fov;
	this->from = from;
	computeFrame();
	computeHitherDimensions();
}

void Camera::update(float aspect) {
	this->aspect = aspect;
	computeHitherDimensions();
}

void Camera::computeFrame() {
	ze = glm::normalize(from - at);
	xe = glm::normalize(glm::cross(up, ze));
	ye = glm::cross(ze, xe);
}

void Camera::computeHitherDimensions() {
	atDistance = glm::length(at - from);
	height = 2.0f * tanf(fov * DEG2RAD * 0.5f) * atDistance;
	width = aspect * height;
}

