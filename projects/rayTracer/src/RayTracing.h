#ifndef _RAY_TRACING_
#define _RAY_TRACING_

#include <limits>
#include "AccelerationStructures.h"


glm::vec3 rayTracing(Scene *scene, Ray ray, int depth);
bool nearestIntersect(Scene *scene, Ray ray, RayIntersection *out);
glm::vec3 computeTransmissionDir(glm::vec3 inDir, glm::vec3 normal, float beforeIOR, float afterIOR);



#endif
