#ifndef _SCENE_H_
#define _SCENE_H_

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>
#include "Primitives.h"

class Shape;
class Light;

class Camera {
public:
	glm::vec3 from, at, up;
	float fov;
	float aspect;
	float width, height;
	float atDistance;
	glm::vec3 xe, ye, ze;

	Camera(glm::vec3 from, glm::vec3 at, glm::vec3 up, float fov, float aspect);

	//move
	void update(glm::vec3 from, float fov);

	//reshape
	void update(float aspect);

private:
	void computeFrame();
	void computeHitherDimensions();

};


class Scene {
	Camera *_camera;
	glm::vec3 _backGroung;
	Material _material;
	std::vector<Shape *> _shapes;
	std::vector<Shape *> _planes;
	std::vector<Light *> _lights;
	glm::vec3 _cmin, _cmax;

public:
	Scene();
	~Scene();
	Camera* getCamera();
	std::vector<Shape *> &getShapes();
	std::vector<Shape *> &getPlanes();
	std::vector<Light *> &getLights();
	glm::vec3 getBackColor();
	glm::vec3 getCmin();
	glm::vec3 getCmax();
	bool loadNff(std::string filePath, float *initRadius, float *initVerticalAngle,
		float *initHorizontalAngle, float *initFov);
	void addView(glm::vec3 from, glm::vec3 at, glm::vec3 up, float fov, float aspect);
	void setBackColor(glm::vec3 color);
	void addLight(glm::vec3 pos);
	void addLight(glm::vec3 pos, glm::vec3 color);
	void setMaterial(glm::vec3 color, float diffuse, float specular, float shine, float transparency, float ior);
	void addCylinder(glm::vec3 base, glm::vec3 top, float radius);
	void addSphere(glm::vec3 center, float radius);
	void addPoly(int numVerts, std::vector<glm::vec3> verts);
	void addPolyPatch(int numVerts, std::vector<glm::vec3> verts, std::vector<glm::vec3> normals);
	void addPlane(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
};



#endif
