#include "RayTracing.h"


glm::vec3 rayTracing(Scene *scene, Ray ray, int depth) {

	RayIntersection intersect;
	glm::vec3 c = scene->getBackColor();

	bool foundIntersect = nearestIntersect(scene, ray, &intersect);

	if(!foundIntersect)
		return c;

	Material mat = intersect.shape->material();

	// local illumination
	glm::vec3 local(0.0f);
	for(Light *l : scene->getLights()) {
		glm::vec3 feelerDir = glm::normalize(l->position() - intersect.point);
		Ray feeler(intersect.point, feelerDir);

		bool inShadow = false;
		for(Shape *s : scene->getShapes()) {
			if(s->intersection(feeler, nullptr)) {
				inShadow = true;
				break;
			}
		}
		if(!inShadow) {
			float diff = std::fmax(glm::dot(feelerDir, intersect.normal), 0.0f);
			glm::vec3 reflectDir = glm::reflect(-feelerDir, intersect.normal);
			float spec = std::pow(std::fmax(glm::dot(reflectDir, -ray.direction), 0.0f), mat.shininess());

			glm::vec3 seenColor = mat.color() * l->color();
			local += seenColor * (diff * mat.diffuse() + spec * mat.specular());
		}
	}

	// reflection
	glm::vec3 reflectionCol(0.0f);
	if(mat.specular() > 0.0f && depth > 0) {
		Ray reflectedRay = Ray(intersect.point, glm::reflect(ray.direction, intersect.normal));
		reflectionCol = rayTracing(scene, reflectedRay, depth - 1) * mat.color() * mat.specular();
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
			refractionCol = rayTracing(scene, refractedRay, depth - 1) * mat.color() * mat.transparency();
		}
	}

	return local + reflectionCol + refractionCol;
}

bool nearestIntersect(Scene *scene, Ray ray, RayIntersection *out) {
	RayIntersection minIntersect(std::numeric_limits<float>::infinity(), glm::vec3(0.0f), glm::vec3(0.0f));
	bool intersectionFound = false;

	RayIntersection curr = minIntersect;
	for(Shape *s : scene->getShapes()) {
		if(s->intersection(ray, &curr)) {
			if(curr.distance < minIntersect.distance) {
				intersectionFound = true;
				minIntersect = curr;
			}
		}
	}

	if(intersectionFound) {
		*out = minIntersect;
	}
	return intersectionFound;
}

glm::vec3 computeTransmissionDir(glm::vec3 inDir, glm::vec3 normal, float beforeIOR, float afterIOR) {
	return glm::refract(inDir, normal, beforeIOR / afterIOR);
}