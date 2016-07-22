#ifndef _RAY_TRACING_
#define _RAY_TRACING_

#include <limits>
#include "AccelerationStructures.h"


glm::vec3 rayTracing(AccelerationStructure *sceneAS, Ray ray, int depth);

bool nearestIntersection(AccelerationStructure *sceneAS, Ray ray, RayIntersection *out);

glm::vec3 computeTransmissionDir(glm::vec3 inDir, glm::vec3 normal, float beforeIOR, float afterIOR);

glm::vec3 estimateShadowTransmittance(AccelerationStructure *sceneAS, Ray feeler, glm::vec3 color);

glm::vec3 computeShadows(AccelerationStructure *sceneAS, Ray ray, RayIntersection intersect,
	glm::vec3 feelerDir, Light* light);

glm::vec3 computeSoftShadows(AccelerationStructure *sceneAS, Ray ray, RayIntersection intersect,
	glm::vec3 feelerDir, Light* light);

bool findIntersection(AccelerationStructure *sceneAS, Ray feeler);

#endif
