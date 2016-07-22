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
	virtual bool estimateShadowTransmittance(Ray ray, glm::vec3 &color, float &transmittance) = 0;
	virtual bool findIntersection(Ray ray) = 0;
	Scene* getScene();
};

class LBVH : public AccelerationStructure {
	BVHNode *nodes;
	unsigned int bvhSize;

public:
	LBVH(Scene *scene);
	~LBVH();
	void build();
	bool findNearestIntersection(Ray ray, RayIntersection *out);
	bool estimateShadowTransmittance(Ray ray, glm::vec3 &color, float &transmittance);
	bool findIntersection(Ray ray);
};


#endif