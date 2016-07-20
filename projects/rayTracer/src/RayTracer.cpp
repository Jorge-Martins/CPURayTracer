#include <stdlib.h> 
#include <stdio.h>
#include <GL/glew.h>
#include <GL/glut.h> 
#include <gtc/type_ptr.hpp>
#include "parsing/mc_driver.hpp"
#include "RayTracing.h"


int RES_X = 512;
int RES_Y = 512;

Scene *scene(new Scene);
glm::vec3 *colors = new glm::vec3[RES_X * RES_Y];
Camera *camera;

bool renderFrame = true;
bool videoMode = false;

float horizontalAngle, verticalAngle, radius;

float initHorizontalAngle;
float initVerticalAngle;
float initRadius;
float initFov;

int xDragStart, yDragStart, dragging, zooming;
float fov;

AccelerationStructure *accelerationStructure;

glm::vec3 computeFromCoordinates(glm::vec3 up) {
	float ha, va;

	ha = (float)(horizontalAngle * DEG2RAD);
	va = (float)(verticalAngle * DEG2RAD);

	if(up.x > 0) {
		return glm::vec3(radius * cos(va), radius * sin(va) * cos(ha), radius * sin(va) * sin(ha));

	}
	else if(up.y > 0) {
		return glm::vec3(radius * sin(va) * sin(ha), radius * cos(va), radius * sin(va) * cos(ha));
	}

	return glm::vec3(radius * sin(va) * cos(ha), radius * sin(va) * sin(ha), radius * cos(va));
}

void cleanup() {
	delete scene;
	delete[] colors;

	std::cout << "Cleanup done!" << std::endl;
}

void reshape(int w, int h) {
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D(0, RES_X, 0, RES_Y);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


glm::vec3 naiveSuperSampling(glm::vec3 xe, glm::vec3 ye, glm::vec3 ze, int sx, int sy, int i) {
	int x = i % RES_X;
	int y = i / RES_Y;

	xe *= ((x + (sx + 0.5f) * SUPER_SAMPLING_F) / (float)RES_X - 0.5f);
	ye *= ((y + (sy + 0.5f) * SUPER_SAMPLING_F) / (float)RES_Y - 0.5f);

	glm::vec3 direction = glm::normalize(xe + ye + ze);

	return direction;
}

glm::vec3 stochasticSuperSampling(glm::vec3 xe, glm::vec3 ye, glm::vec3 ze, int ss, int i) {
	int x = i % RES_X;
	int y = i / RES_Y;
	int seed = (int)clock();

	xe *= ((x + haltonSequance(i + seed + ss, 2)) / (float)RES_X - 0.5f);
	ye *= ((y + haltonSequance(i + seed + ss, 3)) / (float)RES_Y - 0.5f);


	glm::vec3 direction = glm::normalize(xe + ye + ze);

	return direction;
}

// Draw function by primary ray casting from the eye towards the scene's objects 
void drawScene() {
	int res = RES_X * RES_Y;

	long time1 = glutGet(GLUT_ELAPSED_TIME);


	if(renderFrame || videoMode) {
		renderFrame = false;
		omp_set_num_threads(res);

		glm::vec3 zeFactor = -camera->ze * camera->atDistance;
		glm::vec3 xe = camera->width * camera->xe;
		glm::vec3 ye = camera->height * camera->ye;

		#pragma omp parallel for
		for(int i = 0; i < res; i++) {
			glm::vec3 direction;
			glm::vec3 color = glm::vec3(0.0f);

			for(int sx = 0; sx < SUPER_SAMPLING; sx++) {
				for(int sy = 0; sy < SUPER_SAMPLING; sy++) {
					direction = naiveSuperSampling(xe, ye, zeFactor, sx, sy, i);
					//direction = stochasticSuperSampling(xe, ye, zeFactor, sx + SUPER_SAMPLING * sy, i);


					Ray ray(camera->from, direction);
					color += rayTracing(scene, ray, MAX_DEPTH);
				}
			}

			colors[i] = SUPER_SAMPLING_2F * color;

		}

		glBegin(GL_POINTS);
		for(int i = 0; i < res; i++) {
			int x = i % RES_X;
			int y = i / RES_Y;

			glColor3fv(glm::value_ptr(colors[i]));
			glVertex2i(i % RES_X, i / RES_X);
		}
		glEnd();
		glFlush();

		long time2 = glutGet(GLUT_ELAPSED_TIME);
		if(!videoMode) {
			std::cout << std::endl << "Elapsed time: " << (time2 - time1) / 1000.0f << " s" << std::endl;
		}

	}
	else {
		glBegin(GL_POINTS);
		for(int i = 0; i < res; i++) {
			glColor3fv(glm::value_ptr(colors[i]));
			glVertex2i(i % RES_X, i / RES_X);
		}
		glEnd();
		glFlush();
	}
}

void mouseMove(int x, int y) {
	float ystep = 0.01f;
	float xstep = 0.01f;
	float zstep = 0.01f;

	if(dragging == 1) {
		horizontalAngle += (-x + xDragStart) * xstep;

		if(horizontalAngle > 360) {
			horizontalAngle -= 360;

		}
		else if(horizontalAngle < 0) {
			horizontalAngle += 360;
		}

		verticalAngle += (-y + yDragStart) * ystep;

		if(verticalAngle > 179) {
			verticalAngle = 179;

		}
		else if(verticalAngle < 1) {
			verticalAngle = 1;
		}

		camera->update(computeFromCoordinates(camera->up), fov);
	}

	if(zooming == 1) {
		fov += (y - yDragStart) * zstep;

		if(fov > 179) {
			fov = 179;
		}
		else if(fov < 1) {
			fov = 1;
		}

		camera->update(computeFromCoordinates(camera->up), fov);
	}
}

void mousePressed(int button, int state, int x, int y) {
	if(button == GLUT_LEFT_BUTTON) {
		if(state == GLUT_DOWN) {
			dragging = 1;
			xDragStart = x;
			yDragStart = y;

		}
		else {
			dragging = 0;
		}
	}

	if(button == GLUT_RIGHT_BUTTON) {
		if(state == GLUT_DOWN) {
			zooming = 1;
			yDragStart = y;

		}
		else {
			zooming = 0;
		}
	}
}

void initPosition() {
	radius = initRadius;
	verticalAngle = initVerticalAngle;
	horizontalAngle = initHorizontalAngle;
	fov = initFov;
}

void keyboardKey(unsigned char key, int x, int y) {
	if(key == 'c') {
		initPosition();
		camera->update(computeFromCoordinates(camera->up), fov);
	}

	if(key == 'f') {
		renderFrame = true;
	}

	if(key == 'm') {
		videoMode = !videoMode;
	}
}

void idle() {
	glutPostRedisplay();
}

int main(int argc, char *argv[]) {
	std::string path = "../../resources/nffFiles/";
	std::string file = "balls_low.nff";

	//file = "balls_low.nff";
	//file = "balls_medium.nff";
	//file = "balls_high.nff";
	file = "mount_low.nff";
	//file = "mount_high.nff";
	//file = "mount_very_high.nff";
	//file = "cyl.nff";

	if(!scene->loadNff(path + file, &initRadius, &initVerticalAngle, &initHorizontalAngle, &initFov)) {
		std::cout << "Could not find scene files." << std::endl;
		cleanup();

		getchar();
		return -1;
	}

	initPosition();

	std::cout << "ResX = " << RES_X << std::endl << "ResY = " << RES_Y << std::endl;
	camera = scene->getCamera();

	accelerationStructure = new LBVH(scene);
	accelerationStructure->build();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);

	glutInitWindowSize(RES_X, RES_Y);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("CPU Ray Tracing");
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST);
	glutReshapeFunc(reshape);
	glutDisplayFunc(drawScene);
	glutMouseFunc(mousePressed);
	glutMotionFunc(mouseMove);
	glutKeyboardFunc(keyboardKey);
	glutIdleFunc(idle);
	glutCloseFunc(cleanup);

	std::cout << std::endl << "CONTEXT: OpenGL v" << glGetString(GL_VERSION) << std::endl;
	glutMainLoop();
	return 0;
}