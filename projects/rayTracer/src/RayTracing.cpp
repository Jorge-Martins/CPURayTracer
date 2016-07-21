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
		glm::vec3 feelerDir = glm::normalize(l->position() - intersect.point);
		Ray feeler(intersect.point, feelerDir);

		glm::vec3 transmittance = estimateShadowTransmittance(sceneAS, feeler, l->color());

		bool result = glm::length(transmittance) > 0.01f;

		if(result) {
			float Ldiff = std::fmax(glm::dot(feelerDir, intersect.normal), 0.0f);
			glm::vec3 reflectDir = glm::reflect(-feelerDir, intersect.normal);
			float Lspec = std::pow(std::fmax(glm::dot(reflectDir, -ray.direction), 0.0f), mat.shininess());

			local += (Ldiff * mat.color() * mat.diffuse() + Lspec * mat.specular()) * transmittance;
		}
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

glm::vec3 estimateShadowTransmittance(AccelerationStructure *sceneAS, Ray feeler, glm::vec3 &color) {
	float transmittance = 1.0f;
	sceneAS->estimateShadowTransmittance(feeler, color, transmittance);

	bool result = false;
	for(Shape *s : sceneAS->getScene()->getPlanes()) {
		RayIntersection curr = RayIntersection();
		result = s->intersection(feeler, &curr);

		if(result) {
			transmittance *= curr.shape->material().transparency();
			color *= curr.shape->material().color();
		}
	}

	return color * transmittance;

}

glm::vec3 computeTransmissionDir(glm::vec3 inDir, glm::vec3 normal, float beforeIOR, float afterIOR) {
	return glm::refract(inDir, normal, beforeIOR / afterIOR);
}