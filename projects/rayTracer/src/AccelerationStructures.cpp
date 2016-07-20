#include "AccelerationStructures.h"

BVHNode::BVHNode() : shape(nullptr), parent(nullptr), leftChild(nullptr), rightChild(nullptr),
min(glm::vec3(FLT_MAX)), max(glm::vec3(-FLT_MAX)) {}

AccelerationStructure::AccelerationStructure(Scene *scene) : scene(scene) {}

LBVH::LBVH(Scene *scene) : AccelerationStructure::AccelerationStructure(scene), nodes(nullptr), bvhSize(0) {}

void LBVH::build() {
	std::vector<Shape *> shapes = scene->getShapes();
	unsigned int nObjects = shapes.size();
	unsigned int size = 2 * nObjects - 1;

	unsigned int *mortonCodes = new unsigned int[nObjects];

	//Index where the leaves start on the BVH
	unsigned int leafOffset = nObjects - 1;

	//Alocate space for the BVH
	if(bvhSize > 0 && bvhSize != size) {
		delete[] nodes;
		nodes = new BVHNode[size];
		bvhSize = size;

	}
	else if(bvhSize == 0) {
		nodes = new BVHNode[size];
		bvhSize = size;
	}
 
	BVHNode *temp = new BVHNode[nObjects];
	
	int index = 0;
	Extent *e;
	for(Shape *s : shapes) {
		e = &s->getAAExtent();

		temp[index].min = e->min;
		temp[index].max = e->max;
		temp[index++].shape = s;
	}

	std::cout << " morton codes sort: " << std::endl;
	unsigned int code;
	std::map <unsigned int, unsigned int> mcodes;
	//sort morton codes
	clock_t start = clock();
	for(unsigned int i = 0; i < nObjects; i++) {
		code = morton3D(computeCenter(scene->getCmin(), scene->getCmax(), temp[i].min, temp[i].max));
		mcodes.insert(std::pair<unsigned int, unsigned int>(code, i));
	}

	clock_t end = clock();
	std::cout << "time: " << (float)(end - start) / CLOCKS_PER_SEC << "s" << std::endl << std::endl;

	index = 0;
	unsigned int value;
	for(auto entry : mcodes) {
		mortonCodes[index] = entry.first;
		value = entry.second;

		nodes[leafOffset + index++] = temp[value];
	}

	delete[] temp;
	mcodes.clear();

	

}

bool LBVH::findNearestIntersection(Ray ray, RayIntersection *out) {
	return false;
}