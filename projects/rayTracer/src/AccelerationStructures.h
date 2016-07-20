#ifndef _ACCELERATION_STRUCTURES_
#define _ACCELERATION_STRUCTURES_

#include <iostream> 
#include <omp.h>
#include <time.h>
#include <algorithm>
#include <map>
#include "Scene.h"

class AccelerationStructure {
protected:
	Scene *scene;
	
public:
	AccelerationStructure(Scene *scene);
	virtual void build() = 0;
	virtual bool findNearestIntersection(Ray ray, RayIntersection *out) = 0;
};

struct BVHNode {
	glm::vec3 min, max;
	Shape *shape;
	BVHNode *parent;
	BVHNode *leftChild;
	BVHNode *rightChild;

	BVHNode();
};

class LBVH : public AccelerationStructure {
	BVHNode *nodes;
	unsigned int bvhSize;

public:
	LBVH(Scene *scene);
	void build();
	bool findNearestIntersection(Ray ray, RayIntersection *out);
};


#endif