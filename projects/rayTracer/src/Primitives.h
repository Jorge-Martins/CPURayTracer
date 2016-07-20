#ifndef _PRIMITIVES_
#define _PRIMITIVES_

#include <glm.hpp>
#include <vector>
#include "MathUtil.h"

class Shape;

enum RayClassification {
	NNN, NNO, NNP, NON, NOO, NOP, NPN, NPO, NPP,
	ONN, ONO, ONP, OON, OOO, OOP, OPN, OPO, OPP,
	PNN, PNO, PNP, PON, POO, POP, PPN, PPO, PPP
};

struct RayIntersection {
	float distance;
	glm::vec3 point;
	glm::vec3 normal;
	Shape *shape;
	bool isEntering;

	RayIntersection();
	RayIntersection(float dist, glm::vec3 point, glm::vec3 normal);
	RayIntersection(float dist, glm::vec3 point, glm::vec3 normal, bool isEntering, Shape *shape);
};

struct Ray {
	glm::vec3 origin;
	glm::vec3 direction;
	glm::vec3 invDirection;

	//slope
	short classification;
	float x_y, y_x, y_z, z_y, x_z, z_x;
	float c_xy, c_xz, c_yx, c_yz, c_zx, c_zy;

	Ray();
	Ray(glm::vec3 origin, glm::vec3 direction);
	void computeSlopes();
	void update(glm::vec3 origin, glm::vec3 direction);
};

class Material {
	glm::vec3 _color;
	float _diffuse;
	float _specular;
	float _shininess;
	float _transparency;
	float _ior;

public:
	Material();
	Material(glm::vec3 color, float diffuse, float specular, float shininess, float transparency, float refraction);
	glm::vec3 color();
	float diffuse();
	float specular();
	float shininess();
	float transparency();
	float ior();
};

struct Extent {
	glm::vec3 min;
	glm::vec3 max;

	Extent(glm::vec3 min, glm::vec3 max);
};

class Shape {
private:
	Material _material;

public:
	Material &material();
	void setMaterial(Material mat);
	virtual bool intersection(Ray ray, RayIntersection *out) = 0;
	virtual Extent getAAExtent() = 0;
};

class Sphere : public Shape {
private:
	float radius;
	glm::vec3 center;

public:
	Sphere(glm::vec3 center, float radius);
	bool intersection(Ray ray, RayIntersection *out);
	Extent getAAExtent();
};

class Cylinder : public Shape {
private:
	glm::vec3 base, top;
	float radius;

	bool infiniteCylinderIntersection(Ray ray, RayIntersection *out, glm::vec3 axis, float *inD, float *outD);

public:
	Cylinder(glm::vec3 base, glm::vec3 top, float radius);
	bool intersection(Ray ray, RayIntersection *out);
	Extent getAAExtent();
};

class Plane : public Shape {
	glm::vec3 normal;
	float distance;

public:
	Plane(glm::vec3 n, float d);
	Plane(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3);
	bool intersection(Ray ray, RayIntersection *out);
	Extent getAAExtent();
};

class Triangle : public Shape {
private:
	std::vector<glm::vec3> vertices;
	glm::vec3 e1, e2, normal;

public:
	Triangle(std::vector<glm::vec3> vertices);
	bool intersection(Ray ray, RayIntersection *out);
	Extent getAAExtent();
};

class Light {
private:
	glm::vec3 _position;
	glm::vec3 _color;

public:
	Light();
	Light(glm::vec3 position, glm::vec3 color);
	glm::vec3 position();
	glm::vec3 color();
};

struct BVHNode {
	glm::vec3 min, max;
	Shape *shape;
	BVHNode *parent;
	BVHNode *leftChild;
	BVHNode *rightChild;

	BVHNode();
	bool intersection(Ray ray);
	bool intersection(Ray ray, float &distance);
};

#endif 

