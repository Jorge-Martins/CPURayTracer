#include "Primitives.h"

RayIntersection::RayIntersection()
	: distance(0), point(glm::vec3(0)), normal(glm::vec3(0)), shape(nullptr), isEntering(true) {
}

RayIntersection::RayIntersection(float dist, glm::vec3 point, glm::vec3 normal)
	: distance(dist), point(point), normal(normal), shape(nullptr), isEntering(true) {
}

RayIntersection::RayIntersection(float dist, glm::vec3 point, glm::vec3 normal, bool isEntering, Shape *shape)
	: distance(dist), point(point), normal(normal), shape(shape), isEntering(isEntering) {
}


void Ray::computeSlopes() {
	invDirection = 1.0f / direction;
	x_y = direction.x * invDirection.y;
	y_x = direction.y * invDirection.x;
	y_z = direction.y * invDirection.z;
	z_y = direction.z * invDirection.y;
	x_z = direction.x * invDirection.z;
	z_x = direction.z * invDirection.x;

	c_xy = origin.y - y_x * origin.x;
	c_xz = origin.z - z_x * origin.x;
	c_yx = origin.x - x_y * origin.y;
	c_yz = origin.z - z_y * origin.y;
	c_zx = origin.x - x_z * origin.z;
	c_zy = origin.y - y_z * origin.z;


	if(direction.x < 0) {
		classification = NNN;

	}
	else if(direction.x > 0) {
		classification = PNN;

	}
	else {
		classification = ONN;
	}

	if(direction.y < 0) {
		//ignore
	}
	else if(direction.y > 0) {
		classification += 6;

	}
	else {
		classification += 3;
	}

	if(direction.z < 0) {
		//ignore
	}
	else if(direction.z > 0) {
		classification += 2;

	}
	else {
		classification += 1;
	}
}

Ray::Ray() {
	origin = glm::vec3(0.0f);
	direction = glm::vec3(0.0f, 0.0f, 1.0f);

	computeSlopes();
}


Ray::Ray(glm::vec3 origin, glm::vec3 direction) {
	this->origin = origin;
	this->direction = direction;

	computeSlopes();
}

void Ray::update(glm::vec3 origin, glm::vec3 direction) {
	this->origin = origin;
	this->direction = direction;

	computeSlopes();
}


Material::Material() {
	_color = glm::vec3(0.0f);
	_diffuse = _specular = _shininess = _transparency = _ior = 0.0f;
}

Material::Material(glm::vec3 color, float diffuse, float specular, float shininess, float transparency, float ior) {
	_color = color;
	_diffuse = diffuse;
	_specular = specular;
	_shininess = shininess;
	_transparency = transparency;
	_ior = ior;
}

glm::vec3 Material::color() {
	return _color;
}

float Material::diffuse() {
	return _diffuse;
}

float Material::specular() {
	return _specular;
}

float Material::shininess() {
	return _shininess;
}

float Material::transparency() {
	return _transparency;
}

float Material::ior() {
	return _ior;
}

Extent::Extent(glm::vec3 min, glm::vec3 max) : min(min), max(max) {}

Material &Shape::material() {
	return _material;
}


void Shape::setMaterial(Material mat) {
	_material = mat;
}

Sphere::Sphere(glm::vec3 center, float radius) {
	this->center = center;
	this->radius = radius;
}

Extent Sphere::getAAExtent() {
	glm::vec3 min = center - radius;
	glm::vec3 max = center + radius;

	return Extent(min, max);
}

bool Sphere::intersection(Ray ray, RayIntersection *out) {
	float d_2, r_2, b, root, t;

	glm::vec3 s_r = center - ray.origin;

	r_2 = radius * radius;
	d_2 = glm::dot(s_r, s_r);

	if(equal(d_2, r_2)) {
		return false;
	}
	b = glm::dot(ray.direction, s_r);

	if(d_2 > r_2 && b < 0.0f) {
		return false;
	}

	root = b*b - d_2 + r_2;
	if(root < 0.0f) {
		return false;
	}

	float sRoot = sqrtf(root);
	t = fminf(b - sRoot, b + sRoot);

	if(out != nullptr) {
		out->point = ray.origin + ray.direction * t;
		out->normal = glm::normalize((out->point - center) / radius);

		bool entering = true;
		if(d_2 < r_2) {
			out->normal *= -1.0f;
			entering = false;
		}

		out->point += out->normal * EPSILON;
		out->shape = this;
		out->distance = t;
		out->isEntering = entering;
	}

	return true;
}

Cylinder::Cylinder(glm::vec3 base, glm::vec3 top, float radius) : base(base), top(top), radius(radius) {}

Extent Cylinder::getAAExtent() {
	glm::vec3 min = glm::min(base, top) - radius;
	glm::vec3 max = glm::max(base, top) + radius;

	return Extent(min, max);
}

bool Cylinder::infiniteCylinderIntersection(Ray ray, RayIntersection *out, glm::vec3 axis, float *inD, float *outD) {
	glm::vec3 r_c = ray.origin - base;
	float r_2 = radius * radius;
	glm::vec3 n = glm::cross(ray.direction, axis);

	float ln = glm::length(n);

	// check if is parallel
	if(equal(ln, 0.0f)) {
		*inD = -1.0e21f;
		*outD = 1.0e21f;
		return glm::length(r_c - glm::dot(r_c, axis) * axis) <= radius;
	}
	n = glm::normalize(n);

	float d = fabsf(glm::dot(r_c, n));

	if(d <= radius) {
		glm::vec3 O = glm::cross(r_c, axis);

		float t = -glm::dot(O, n) / ln;

		O = glm::normalize(glm::cross(n, axis));

		float s = fabsf(sqrtf(r_2 - d*d) / glm::dot(ray.direction, O));

		*inD = t - s;
		*outD = t + s;

		return true;
	}

	return false;
}

bool Cylinder::intersection(Ray ray, RayIntersection *out) {
	glm::vec3 axis = glm::normalize(top - base);
	glm::vec3 normal, point;

	float baseDistance = -glm::dot(-axis, base);
	float topDistance = -glm::dot(axis, top);

	float dc, dw, t;
	float inD, outD;		/* Object  intersection dists.	*/
							//0 top, 1 side, 2 base
	unsigned char sideIn;
	unsigned char sideOut;

	if(!infiniteCylinderIntersection(ray, out, axis, &inD, &outD)) {
		return false;
	}

	sideIn = sideOut = 1;

	/*	Intersect the ray with the bottom end-cap plane.		*/

	dc = glm::dot(-axis, ray.direction);
	dw = glm::dot(-axis, ray.origin) + baseDistance;

	if(dc == 0.0f) {		/* If parallel to bottom plane	*/
		if(dw >= 0.0f) {
			return false;
		}
	}
	else {
		t = -dw / dc;
		if(dc >= 0.0f) {			    /* If far plane	*/
			if(t > inD && t < outD) {
				outD = t;
				sideOut = 2;
			}
			if(t < inD) {
				return false;
			}
		}
		else {				    /* If near plane	*/
			if(t > inD && t < outD) {
				inD = t;
				sideIn = 2;

			}
			if(t > outD) {
				return false;
			}
		}
	}

	/*	Intersect the ray with the top end-cap plane.			*/

	dc = glm::dot(axis, ray.direction);
	dw = glm::dot(axis, ray.origin) + topDistance;

	if(dc == 0.0f) {		/* If parallel to top plane	*/
		if(dw >= 0.0f) {
			return false;
		}
	}
	else {
		t = -dw / dc;
		if(dc >= 0.0f) {			    /* If far plane	*/
			if(t > inD && t < outD) {
				outD = t;
				sideOut = 0;
			}
			if(t < inD) {
				return false;
			}
		}
		else {				    /* If near plane	*/
			if(t > inD && t < outD) {
				inD = t;
				sideIn = 0;

			}
			if(t > outD) {
				return false;
			}
		}
	}

	if(inD < 0 && outD < 0) {
		return false;
	}

	bool entering = true;
	unsigned char side = sideIn;
	if(inD < outD && inD > 0) {
		t = inD;
		point = ray.origin + t * ray.direction;

	}
	else if(outD > 0) {
		t = outD;

		point = ray.origin + t * ray.direction;

		side = sideOut;
		entering = false;

	}
	else {
		return false;
	}

	if(side == 0) {
		normal = axis;
	}
	else if(side == 1) {
		glm::vec3 v1 = point - base;
		normal = glm::normalize(v1 - projectVector(v1, axis));
	}
	else {
		normal = -axis;
	}

	if(out != nullptr) {
		if(!entering) {
			normal *= -1.0f;
		}

		out->isEntering = entering;
		out->shape = this;
		out->distance = t;
		out->point = point + normal * EPSILON;
		out->normal = normal;

	}

	return true;
}

Plane::Plane(glm::vec3 n, float d) : normal(n), distance(d) {}

Plane::Plane(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
	glm::vec3 p12 = p2 - p1, p23 = p3 - p2;
	normal = glm::normalize(glm::cross(p12, p23));
	float nDOTapoint = glm::dot(normal, p1);
	distance = -nDOTapoint;
}

//Planes need to be in a separate list
Extent Plane::getAAExtent() {
	return Extent(glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX));
}

bool Plane::intersection(Ray ray, RayIntersection *out) {
	float nDOTrdir = glm::dot(normal, ray.direction);

	if(equal(nDOTrdir, 0.0f)) {
		return false;
	}

	float nDOTr0 = glm::dot(normal, ray.origin);
	float t = -((nDOTr0 + distance) / nDOTrdir);

	if(t < 0.0f) {
		return false;
	}

	if(out != nullptr) {
		out->shape = this;
		out->distance = t;
		out->normal = normal;
		out->point = ray.origin + t*ray.direction;
		out->isEntering = nDOTrdir < 0.0f;

		out->point += out->normal * EPSILON;
	}
	return true;
}


Triangle::Triangle(std::vector<glm::vec3> vertices) {
	this->vertices = vertices;
	e1 = vertices[1] - vertices[0];
	e2 = vertices[2] - vertices[0];
	normal = glm::normalize(glm::cross(e1, e2));
}

Extent Triangle::getAAExtent() {
	glm::vec3 min = glm::min(glm::min(vertices[0], vertices[1]), vertices[2]);
	glm::vec3 max = glm::max(glm::max(vertices[0], vertices[1]), vertices[2]);

	return Extent(min, max);
}

bool Triangle::intersection(Ray ray, RayIntersection *out) {
	float normalDOTray = glm::dot(normal, ray.direction);

	glm::vec3 h = glm::cross(ray.direction, e2);
	float a = glm::dot(e1, h);

	if(a > -EPSILON && a < EPSILON) {
		return false;
	}
	float f = 1.0f / a;
	glm::vec3 s = ray.origin - vertices[0];
	float u = f * glm::dot(s, h);

	if(u < 0.0 || u > 1.0) {
		return false;
	}

	glm::vec3 q = glm::cross(s, e1);
	float v = f * glm::dot(ray.direction, q);

	if(v < 0.0 || u + v > 1.0) {
		return false;
	}

	float t = f * glm::dot(e2, q);

	if(t < 0) {
		return false;
	}

	if(out != nullptr) {
		out->distance = t;
		out->normal = normal;
		out->point = (ray.origin + t * ray.direction) + out->normal * EPSILON;
		out->shape = this;
		out->isEntering = normalDOTray < 0.0f;
	}

	return true;
}

Light::Light() {
	_position = glm::vec3(0.0f);
	_color = glm::vec3(1.0f);
}

Light::Light(glm::vec3 position, glm::vec3 color) {
	_position = position;
	_color = color;
}

glm::vec3 Light::position() {
	return _position;
}

glm::vec3 Light::color() {
	return _color;
}

BoundingBox::BoundingBox() {
	min = glm::vec3(FLT_MAX);
	max = glm::vec3(-FLT_MAX);
}

BoundingBox::BoundingBox(glm::vec3 p1, glm::vec3 p2) {
	min = glm::min(p1, p2);
	max = glm::max(p1, p2);
}

BoundingBox::BoundingBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) {
	min = glm::vec3(minX, minY, minZ);
	max = glm::vec3(maxX, maxY, maxZ);
}

BVHBoundingBox::BVHBoundingBox(glm::vec3 min, glm::vec3 max) : BoundingBox(min, max), shape(nullptr) {}

BVHBoundingBox::BVHBoundingBox(Shape *shape) {
	Extent e = shape->getAAExtent();

	min = e.min;
	max = e.max;
	this->shape = shape;
}

BVHBoundingBox::BVHBoundingBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) :
	BoundingBox(minX, minY, minZ, maxX, maxY, maxZ), shape(nullptr) {
}

Shape* BVHBoundingBox::getShape() {
	return shape;
}


bool nnn(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.x < min.x) || (ray.origin.y < min.y) || (ray.origin.z < min.z)
		|| (ray.y_x * min.x - max.y + ray.c_xy > 0)
		|| (ray.x_y * min.y - max.x + ray.c_yx > 0)
		|| (ray.y_z * min.z - max.y + ray.c_zy > 0)
		|| (ray.z_y * min.y - max.z + ray.c_yz > 0)
		|| (ray.z_x * min.x - max.z + ray.c_xz > 0)
		|| (ray.x_z * min.z - max.x + ray.c_zx > 0));
}


bool nnp(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.x < min.x) || (ray.origin.y < min.y) || (ray.origin.z > max.z)
		|| (ray.y_x * min.x - max.y + ray.c_xy > 0)
		|| (ray.x_y * min.y - max.x + ray.c_yx > 0)
		|| (ray.y_z * max.z - max.y + ray.c_zy > 0)
		|| (ray.z_y * min.y - min.z + ray.c_yz < 0)
		|| (ray.z_x * min.x - min.z + ray.c_xz < 0)
		|| (ray.x_z * max.z - max.x + ray.c_zx > 0));
}


bool npn(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.x < min.x) || (ray.origin.y > max.y) || (ray.origin.z < min.z)
		|| (ray.y_x * min.x - min.y + ray.c_xy < 0)
		|| (ray.x_y * max.y - max.x + ray.c_yx > 0)
		|| (ray.y_z * min.z - min.y + ray.c_zy < 0)
		|| (ray.z_y * max.y - max.z + ray.c_yz > 0)
		|| (ray.z_x * min.x - max.z + ray.c_xz > 0)
		|| (ray.x_z * min.z - max.x + ray.c_zx > 0));
}


bool npp(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.x < min.x) || (ray.origin.y > max.y) || (ray.origin.z > max.z)
		|| (ray.y_x * min.x - min.y + ray.c_xy < 0)
		|| (ray.x_y * max.y - max.x + ray.c_yx > 0)
		|| (ray.y_z * max.z - min.y + ray.c_zy < 0)
		|| (ray.z_y * max.y - min.z + ray.c_yz < 0)
		|| (ray.z_x * min.x - min.z + ray.c_xz < 0)
		|| (ray.x_z * max.z - max.x + ray.c_zx > 0));
}


bool pnn(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.x > max.x) || (ray.origin.y < min.y) || (ray.origin.z < min.z)
		|| (ray.y_x * max.x - max.y + ray.c_xy > 0)
		|| (ray.x_y * min.y - min.x + ray.c_yx < 0)
		|| (ray.y_z * min.z - max.y + ray.c_zy > 0)
		|| (ray.z_y * min.y - max.z + ray.c_yz > 0)
		|| (ray.z_x * max.x - max.z + ray.c_xz > 0)
		|| (ray.x_z * min.z - min.x + ray.c_zx < 0));
}


bool pnp(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.x > max.x) || (ray.origin.y < min.y) || (ray.origin.z > max.z)
		|| (ray.y_x * max.x - max.y + ray.c_xy > 0)
		|| (ray.x_y * min.y - min.x + ray.c_yx < 0)
		|| (ray.y_z * max.z - max.y + ray.c_zy > 0)
		|| (ray.z_y * min.y - min.z + ray.c_yz < 0)
		|| (ray.z_x * max.x - min.z + ray.c_xz < 0)
		|| (ray.x_z * max.z - min.x + ray.c_zx < 0));
}


bool ppn(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.x > max.x) || (ray.origin.y > max.y) || (ray.origin.z < min.z)
		|| (ray.y_x * max.x - min.y + ray.c_xy < 0)
		|| (ray.x_y * max.y - min.x + ray.c_yx < 0)
		|| (ray.y_z * min.z - min.y + ray.c_zy < 0)
		|| (ray.z_y * max.y - max.z + ray.c_yz > 0)
		|| (ray.z_x * max.x - max.z + ray.c_xz > 0)
		|| (ray.x_z * min.z - min.x + ray.c_zx < 0));
}


bool ppp(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.x > max.x) || (ray.origin.y > max.y) || (ray.origin.z > max.z)
		|| (ray.y_x * max.x - min.y + ray.c_xy < 0)
		|| (ray.x_y * max.y - min.x + ray.c_yx < 0)
		|| (ray.y_z * max.z - min.y + ray.c_zy < 0)
		|| (ray.z_y * max.y - min.z + ray.c_yz < 0)
		|| (ray.z_x * max.x - min.z + ray.c_xz < 0)
		|| (ray.x_z * max.z - min.x + ray.c_zx < 0));
}


bool onn(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.x < min.x) || (ray.origin.x > max.x)
		|| (ray.origin.y < min.y) || (ray.origin.z < min.z)
		|| (ray.y_z * min.z - max.y + ray.c_zy > 0)
		|| (ray.z_y * min.y - max.z + ray.c_yz > 0));
}


bool onp(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.x < min.x) || (ray.origin.x > max.x)
		|| (ray.origin.y < min.y) || (ray.origin.z > max.z)
		|| (ray.y_z * max.z - max.y + ray.c_zy > 0)
		|| (ray.z_y * min.y - min.z + ray.c_yz < 0));
}


bool opn(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.x < min.x) || (ray.origin.x > max.x)
		|| (ray.origin.y > max.y) || (ray.origin.z < min.z)
		|| (ray.y_z * min.z - min.y + ray.c_zy < 0)
		|| (ray.z_y * max.y - max.z + ray.c_yz > 0));
}


bool opp(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.x < min.x) || (ray.origin.x > max.x)
		|| (ray.origin.y > max.y) || (ray.origin.z > max.z)
		|| (ray.y_z * max.z - min.y + ray.c_zy < 0)
		|| (ray.z_y * max.y - min.z + ray.c_yz < 0));
}


bool non(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.y < min.y) || (ray.origin.y > max.y)
		|| (ray.origin.x < min.x) || (ray.origin.z < min.z)
		|| (ray.z_x * min.x - max.z + ray.c_xz > 0)
		|| (ray.x_z * min.z - max.x + ray.c_zx > 0));
}


bool nop(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.y < min.y) || (ray.origin.y > max.y)
		|| (ray.origin.x < min.x) || (ray.origin.z > max.z)
		|| (ray.z_x * min.x - min.z + ray.c_xz < 0)
		|| (ray.x_z * max.z - max.x + ray.c_zx > 0));
}


bool pon(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.y < min.y) || (ray.origin.y > max.y)
		|| (ray.origin.x > max.x) || (ray.origin.z < min.z)
		|| (ray.z_x * max.x - max.z + ray.c_xz > 0)
		|| (ray.x_z * min.z - min.x + ray.c_zx < 0));
}


bool pop(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.y < min.y) || (ray.origin.y > max.y)
		|| (ray.origin.x > max.x) || (ray.origin.z > max.z)
		|| (ray.z_x * max.x - min.z + ray.c_xz < 0)
		|| (ray.x_z * max.z - min.x + ray.c_zx < 0));
}


bool nno(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.z < min.z) || (ray.origin.z > max.z)
		|| (ray.origin.x < min.x) || (ray.origin.y < min.y)
		|| (ray.y_x * min.x - max.y + ray.c_xy > 0)
		|| (ray.x_y * min.y - max.x + ray.c_yx > 0));
}


bool npo(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.z < min.z) || (ray.origin.z > max.z)
		|| (ray.origin.x < min.x) || (ray.origin.y > max.y)
		|| (ray.y_x * min.x - min.y + ray.c_xy < 0)
		|| (ray.x_y * max.y - max.x + ray.c_yx > 0));
}


bool pno(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.z < min.z) || (ray.origin.z > max.z)
		|| (ray.origin.x > max.x) || (ray.origin.y < min.y)
		|| (ray.y_x * max.x - max.y + ray.c_xy > 0)
		|| (ray.x_y * min.y - min.x + ray.c_yx < 0));
}


bool ppo(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.z < min.z) || (ray.origin.z > max.z)
		|| (ray.origin.x > max.x) || (ray.origin.y > max.y)
		|| (ray.y_x * max.x - min.y + ray.c_xy < 0)
		|| (ray.x_y * max.y - min.x + ray.c_yx < 0));
}


bool noo(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.x < min.x)
		|| (ray.origin.y < min.y) || (ray.origin.y > max.y)
		|| (ray.origin.z < min.z) || (ray.origin.z > max.z));
}


bool poo(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.x > max.x)
		|| (ray.origin.y < min.y) || (ray.origin.y > max.y)
		|| (ray.origin.z < min.z) || (ray.origin.z > max.z));
}


bool ono(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.y < min.y)
		|| (ray.origin.x < min.x) || (ray.origin.x > max.x)
		|| (ray.origin.z < min.z) || (ray.origin.z > max.z));
}


bool opo(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.y > max.y)
		|| (ray.origin.x < min.x) || (ray.origin.x > max.x)
		|| (ray.origin.z < min.z) || (ray.origin.z > max.z));
}


bool oon(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.z < min.z)
		|| (ray.origin.x < min.x) || (ray.origin.x > max.x)
		|| (ray.origin.y < min.y) || (ray.origin.y > max.y));
}


bool oop(Ray ray, glm::vec3 min, glm::vec3 max) {
	return !((ray.origin.z > max.z)
		|| (ray.origin.x < min.x) || (ray.origin.x > max.x)
		|| (ray.origin.y < min.y) || (ray.origin.y > max.y));
}

/* AABB intersection with ray slopes */
bool BoundingBox::intersection(Ray ray) {
	switch(ray.classification) {
	case NNN:
		return nnn(ray, min, max);

	case NNP:
		return nnp(ray, min, max);

	case NPN:
		return npn(ray, min, max);

	case NPP:
		return npp(ray, min, max);

	case PNN:
		return pnn(ray, min, max);

	case PNP:
		return pnp(ray, min, max);

	case PPN:
		return ppn(ray, min, max);

	case PPP:
		return ppp(ray, min, max);

	case ONN:
		return onn(ray, min, max);

	case ONP:
		return onp(ray, min, max);

	case OPN:
		return opn(ray, min, max);

	case OPP:
		return opp(ray, min, max);

	case NON:
		return non(ray, min, max);

	case NOP:
		return nop(ray, min, max);

	case PON:
		return pon(ray, min, max);

	case POP:
		return pop(ray, min, max);

	case NNO:
		return nno(ray, min, max);

	case NPO:
		return npo(ray, min, max);

	case PNO:
		return pno(ray, min, max);

	case PPO:
		return ppo(ray, min, max);

	case NOO:
		return noo(ray, min, max);

	case POO:
		return poo(ray, min, max);

	case ONO:
		return ono(ray, min, max);

	case OPO:
		return opo(ray, min, max);

	case OON:
		return oon(ray, min, max);

	case OOP:
		return oop(ray, min, max);

	}

	return false;
}


bool BoundingBox::intersection(Ray ray, float &distance) {
	float t1, t2;
	switch(ray.classification) {
	case NNN:
		if(nnn(ray, min, max)) {
			distance = (max.x - ray.origin.x) * ray.invDirection.x;
			t1 = (max.y - ray.origin.y) * ray.invDirection.y;
			if(t1 > distance) {
				distance = t1;
			}
			t2 = (max.z - ray.origin.z) * ray.invDirection.z;
			if(t2 > distance) {
				distance = t2;
			}

			return true;
		}
		break;

	case NNP:
		if(nnp(ray, min, max)) {
			distance = (max.x - ray.origin.x) * ray.invDirection.x;
			t1 = (max.y - ray.origin.y) * ray.invDirection.y;
			if(t1 > distance) {
				distance = t1;
			}
			t2 = (min.z - ray.origin.z) * ray.invDirection.z;
			if(t2 > distance) {
				distance = t2;
			}

			return true;
		}
		break;

	case NPN:
		if(npn(ray, min, max)) {
			distance = (max.x - ray.origin.x) * ray.invDirection.x;
			t1 = (min.y - ray.origin.y) * ray.invDirection.y;
			if(t1 > distance) {
				distance = t1;
			}
			t2 = (max.z - ray.origin.z) * ray.invDirection.z;
			if(t2 > distance) {
				distance = t2;
			}

			return true;
		}
		break;

	case NPP:
		if(npp(ray, min, max)) {
			distance = (max.x - ray.origin.x) * ray.invDirection.x;
			t1 = (min.y - ray.origin.y) * ray.invDirection.y;
			if(t1 > distance) {
				distance = t1;
			}
			t2 = (min.z - ray.origin.z) * ray.invDirection.z;
			if(t2 > distance) {
				distance = t2;
			}

			return true;
		}
		break;

	case PNN:
		if(pnn(ray, min, max)) {
			distance = (min.x - ray.origin.x) * ray.invDirection.x;
			t1 = (max.y - ray.origin.y) * ray.invDirection.y;
			if(t1 > distance) {
				distance = t1;
			}
			t2 = (max.z - ray.origin.z) * ray.invDirection.z;
			if(t2 > distance) {
				distance = t2;
			}

			return true;
		}
		break;

	case PNP:
		if(pnp(ray, min, max)) {
			distance = (min.x - ray.origin.x) * ray.invDirection.x;
			t1 = (max.y - ray.origin.y) * ray.invDirection.y;
			if(t1 > distance) {
				distance = t1;
			}
			t2 = (min.z - ray.origin.z) * ray.invDirection.z;
			if(t2 > distance) {
				distance = t2;
			}

			return true;
		}
		break;

	case PPN:
		if(ppn(ray, min, max)) {
			distance = (min.x - ray.origin.x) * ray.invDirection.x;
			t1 = (min.y - ray.origin.y) * ray.invDirection.y;
			if(t1 > distance) {
				distance = t1;
			}
			t2 = (max.z - ray.origin.z) * ray.invDirection.z;
			if(t2 > distance) {
				distance = t2;
			}

			return true;
		}
		break;

	case PPP:
		if(ppp(ray, min, max)) {
			distance = (min.x - ray.origin.x) * ray.invDirection.x;
			t1 = (min.y - ray.origin.y) * ray.invDirection.y;
			if(t1 > distance) {
				distance = t1;
			}
			t2 = (min.z - ray.origin.z) * ray.invDirection.z;
			if(t2 > distance) {
				distance = t2;
			}

			return true;
		}
		break;

	case ONN:
		if(onn(ray, min, max)) {
			distance = (max.y - ray.origin.y) * ray.invDirection.y;
			t2 = (max.z - ray.origin.z) * ray.invDirection.z;
			if(t2 > distance) {
				distance = t2;
			}

			return true;

		}
		break;

	case ONP:
		if(onp(ray, min, max)) {
			distance = (max.y - ray.origin.y) * ray.invDirection.y;
			t2 = (min.z - ray.origin.z) * ray.invDirection.z;
			if(t2 > distance) {
				distance = t2;
			}

			return true;

		}
		break;

	case OPN:
		if(opn(ray, min, max)) {
			distance = (min.y - ray.origin.y) * ray.invDirection.y;
			t2 = (max.z - ray.origin.z) * ray.invDirection.z;
			if(t2 > distance) {
				distance = t2;
			}

			return true;
		}
		break;

	case OPP:
		if(opp(ray, min, max)) {
			distance = (min.y - ray.origin.y) * ray.invDirection.y;
			t2 = (min.z - ray.origin.z) * ray.invDirection.z;
			if(t2 > distance) {
				distance = t2;
			}

			return true;
		}
		break;

	case NON:
		if(non(ray, min, max)) {
			distance = (max.x - ray.origin.x) * ray.invDirection.x;
			t2 = (max.z - ray.origin.z) * ray.invDirection.z;
			if(t2 > distance) {
				distance = t2;
			}

			return true;
		}
		break;

	case NOP:
		if(nop(ray, min, max)) {
			distance = (max.x - ray.origin.x) * ray.invDirection.x;
			t2 = (min.z - ray.origin.z) * ray.invDirection.z;
			if(t2 > distance) {
				distance = t2;
			}

			return true;
		}
		break;

	case PON:
		if(pon(ray, min, max)) {
			distance = (min.x - ray.origin.x) * ray.invDirection.x;
			t2 = (max.z - ray.origin.z) * ray.invDirection.z;
			if(t2 > distance) {
				distance = t2;
			}

			return true;
		}
		break;

	case POP:
		if(pop(ray, min, max)) {
			distance = (min.x - ray.origin.x) * ray.invDirection.x;
			t2 = (min.z - ray.origin.z) * ray.invDirection.z;
			if(t2 > distance) {
				distance = t2;
			}

			return true;
		}
		break;

	case NNO:
		if(nno(ray, min, max)) {
			distance = (max.x - ray.origin.x) * ray.invDirection.x;
			t1 = (max.y - ray.origin.y) * ray.invDirection.y;
			if(t1 > distance) {
				distance = t1;
			}

			return true;
		}
		break;

	case NPO:
		if(npo(ray, min, max)) {
			distance = (max.x - ray.origin.x) * ray.invDirection.x;
			t1 = (min.y - ray.origin.y) * ray.invDirection.y;
			if(t1 > distance) {
				distance = t1;
			}

			return true;
		}
		break;

	case PNO:
		if(pno(ray, min, max)) {
			distance = (min.x - ray.origin.x) * ray.invDirection.x;
			t1 = (max.y - ray.origin.y) * ray.invDirection.y;
			if(t1 > distance) {
				distance = t1;
			}

			return true;
		}
		break;

	case PPO:
		if(ppo(ray, min, max)) {
			distance = (min.x - ray.origin.x) * ray.invDirection.x;
			t1 = (min.y - ray.origin.y) * ray.invDirection.y;
			if(t1 > distance) {
				distance = t1;
			}

			return true;
		}

	case NOO:
		if(noo(ray, min, max)) {
			distance = (max.x - ray.origin.x) * ray.invDirection.x;
			return true;
		}
		break;

	case POO:
		if(poo(ray, min, max)) {
			distance = (min.x - ray.origin.x) * ray.invDirection.x;
			return true;
		}
		break;

	case ONO:
		if(ono(ray, min, max)) {
			distance = (max.y - ray.origin.y) * ray.invDirection.y;
			return true;
		}
		break;

	case OPO:
		if(opo(ray, min, max)) {
			distance = (min.y - ray.origin.y) * ray.invDirection.y;
			return true;
		}
		break;

	case OON:
		if(oon(ray, min, max)) {
			distance = (max.z - ray.origin.z) * ray.invDirection.z;
			return true;
		}
		break;

	case OOP:
		if(oop(ray, min, max)) {
			distance = (min.z - ray.origin.z) * ray.invDirection.z;
			return true;
		}
		break;

	}

	return false;
}