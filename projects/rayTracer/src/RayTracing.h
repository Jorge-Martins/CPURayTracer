#ifndef _RAY_TRACING_
#define _RAY_TRACING_

#include <limits>
#include "AccelerationStructures.h"


glm::vec3 rayTracing(AccelerationStructure *sceneAS, Ray ray, int depth);
bool nearestIntersection(AccelerationStructure *sceneAS, Ray ray, RayIntersection *out);
glm::vec3 computeTransmissionDir(glm::vec3 inDir, glm::vec3 normal, float beforeIOR, float afterIOR);



#endif
