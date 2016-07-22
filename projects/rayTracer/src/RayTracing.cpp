#include "RayTracing.h"


glm::vec3 rayTracing(AccelerationStructure *sceneAS, Ray ray, int depth) {
	Scene *scene = sceneAS->getScene();
	RayIntersection intersect;
	glm::vec3 c = scene->getBackColor();

	bool foundIntersect = nearestIntersection(sceneAS, ray, &intersect);

	if(!foundIntersect)
		return c;

	Material mat = intersect.shape->material();

	// local illumination
	glm::vec3 local(0.0f);
	for(Light *l : scene->getLights()) {
		#ifndef SOFT_SHADOWS
		local += computeShadows(sceneAS, ray, intersect, 
			glm::normalize(l->position() - intersect.point), l);
		#else
		local += computeSoftShadows(sceneAS, ray, intersect,
			glm::normalize(l->position() - intersect.point), l);
		#endif
	}

	// reflection
	glm::vec3 reflectionCol(0.0f);
	if(mat.specular() > 0.0f && depth > 0) {
		Ray reflectedRay = Ray(intersect.point, glm::reflect(ray.direction, intersect.normal));
		reflectionCol = rayTracing(sceneAS, reflectedRay, depth - 1) * mat.color() * mat.specular();
	}

	// transmission
	glm::vec3 refractionCol(0.0f);
	if(mat.transparency() > 0.0f && depth > 0) {
		float ior1, ior2;
		if(intersect.isEntering) {
			ior1 = 1.0f;
			ior2 = mat.ior();
		}
		else {
			ior1 = mat.ior();
			ior2 = 1.0f;
		}
		glm::vec3 refractionDir = computeTransmissionDir(ray.direction, intersect.normal, ior1, ior2);
		if(!equal(glm::length(refractionDir), 0.0f)) {
			Ray refractedRay(intersect.point, refractionDir);
			refractionCol = rayTracing(sceneAS, refractedRay, depth - 1) * mat.color() * mat.transparency();
		}
	}

	return local + reflectionCol + refractionCol;
}

bool nearestIntersection(AccelerationStructure *sceneAS, Ray ray, RayIntersection *out) {
	RayIntersection minIntersect(std::numeric_limits<float>::infinity(), glm::vec3(0.0f), glm::vec3(0.0f));
	bool intersectionFound = false, minIntersection = false;

	minIntersection = sceneAS->findNearestIntersection(ray, &minIntersect);

	RayIntersection curr = minIntersect;

	for(Shape *s : sceneAS->getScene()->getPlanes()) {
		intersectionFound = s->intersection(ray, &curr);

		if(intersectionFound && curr.distance < minIntersect.distance) {
			minIntersect = curr;
			minIntersection = true;
		}
	}

	if(minIntersection) {
		*out = minIntersect;
	}

	return minIntersection;
}

glm::vec3 estimateShadowTransmittance(AccelerationStructure *sceneAS, Ray feeler, glm::vec3 lightColor) {
	float transmittance = 1.0f;
	sceneAS->estimateShadowTransmittance(feeler, lightColor, transmittance);

	bool result = false;
	for(Shape *s : sceneAS->getScene()->getPlanes()) {
		RayIntersection curr = RayIntersection();
		result = s->intersection(feeler, &curr);

		if(result) {
			transmittance *= curr.shape->material().transparency();
			lightColor *= curr.shape->material().color();
		}
	}

	return lightColor * transmittance;

}


bool findIntersection(AccelerationStructure *sceneAS, Ray feeler) {
	bool result = sceneAS->findIntersection(feeler);

	if(result) {
		return result;
	}

	for(Shape *s : sceneAS->getScene()->getPlanes()) {
		result = s->intersection(feeler, nullptr);

		if(result) {
			return result;
		}
	}

	return false;
}

glm::vec3 computeTransmissionDir(glm::vec3 inDir, glm::vec3 normal, float beforeIOR, float afterIOR) {
	return glm::refract(inDir, normal, beforeIOR / afterIOR);
}

glm::vec3 computeShadows(AccelerationStructure *sceneAS, Ray ray, RayIntersection intersect, 
	glm::vec3 feelerDir, Light* light) {
	Ray feeler = Ray(intersect.point, feelerDir);
	bool result = false;

	#ifndef SHADOW_TRANSMITTANCE
	bool inShadow = findIntersection(sceneAS, feeler);
	result = !inShadow;

	#else
	glm::vec3 transmittance = estimateShadowTransmittance(sceneAS, feeler, light->color());
	result = glm::length(transmittance) > 0.01f;

	#endif

	if(result) {
		Material mat = intersect.shape->material();
		glm::vec3 reflectDir = glm::reflect(-feelerDir, intersect.normal);
		float Lspec = powf(fmaxf(glm::dot(reflectDir, -ray.direction), 0.0f), mat.shininess());
		float Ldiff = fmaxf(glm::dot(feelerDir, intersect.normal), 0.0f);

		#ifndef SHADOW_TRANSMITTANCE
		return (Ldiff * mat.color() * mat.diffuse() + Lspec * mat.specular()) * light->color();

		#else
		return (Ldiff * mat.color() * mat.diffuse() + Lspec * mat.specular()) * transmittance;

		#endif
	}

	return glm::vec3(0.0f);
}

glm::vec3 computeSoftShadows(AccelerationStructure *sceneAS, Ray ray, RayIntersection intersect, 
	glm::vec3 feelerDir, Light* light) {
	glm::vec3 u, v;
	const glm::vec3 xAxis = glm::vec3(1, 0, 0);
	const glm::vec3 yAxis = glm::vec3(0, 1, 0);

	if(equal(glm::dot(xAxis, feelerDir), 1.0f)) {
		u = glm::cross(feelerDir, yAxis);
	}
	else {
		u = glm::cross(feelerDir, xAxis);
	}
	v = glm::cross(feelerDir, u);

	glm::vec3 localColor = glm::vec3(0.0f);
	for(int x = 0; x < LIGHT_SAMPLE_RADIUS; x++) {
		for(int y = 0; y < LIGHT_SAMPLE_RADIUS; y++) {
			float xCoord = LIGHT_SOURCE_SIZE * ((y + 0.5f) * LIGHT_SAMPLE_RADIUS_F - 0.5f);
			float yCoord = LIGHT_SOURCE_SIZE * ((x + 0.5f) * LIGHT_SAMPLE_RADIUS_F - 0.5f);

			feelerDir = glm::normalize((light->position() + xCoord*u + yCoord*v) - intersect.point);

			localColor += computeShadows(sceneAS, ray, intersect, feelerDir, light);
		}
	}

	return SUM_FACTOR * localColor;
}