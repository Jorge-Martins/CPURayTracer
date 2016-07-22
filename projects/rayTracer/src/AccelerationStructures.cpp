#include "AccelerationStructures.h"

BVHNode::BVHNode() : shape(nullptr), parent(nullptr), leftChild(nullptr), rightChild(nullptr),
min(glm::vec3(FLT_MAX)), max(glm::vec3(-FLT_MAX)) {}

AccelerationStructure::AccelerationStructure(Scene *scene) : scene(scene) {}

Scene* AccelerationStructure::getScene() {
	return scene;
}

LBVH::LBVH(Scene *scene) : AccelerationStructure::AccelerationStructure(scene), nodes(nullptr), bvhSize(0) {}

LBVH::~LBVH() {
	if(nodes != nullptr) {
		delete[] nodes;
	}
}

void LBVH::build() {
	long time1 = glutGet(GLUT_ELAPSED_TIME);
	std::vector<Shape *> shapes = scene->getShapes();
	unsigned int nObjects = shapes.size();
	unsigned int size = 2 * nObjects - 1;

	unsigned int *mortonCodes = new unsigned int[nObjects];

	//Index where the leaves start on the BVH
	unsigned int leafOffset = nObjects - 1;

	//Alocate space for the BVH
	if(bvhSize > 0) {
		delete[] nodes;
		nodes = new BVHNode[size];
		bvhSize = size;

	}
	else if(bvhSize == 0) {
		nodes = new BVHNode[size];
		bvhSize = size;
	}
 
	BVHNode *temp = new BVHNode[nObjects];

	for(unsigned int i = 0; i < shapes.size(); i++) {
		Extent e = shapes[i]->getAAExtent();

		temp[i].min = e.min;
		temp[i].max = e.max;
		temp[i].shape = shapes[i];
	}

	unsigned int code;
	std::multimap <unsigned int, unsigned int> mcodes;
	//sort morton codes
	
	for(unsigned int i = 0; i < nObjects; i++) {
		code = morton3D(computeCenter(scene->getCmin(), scene->getCmax(), temp[i].min, temp[i].max));
		mcodes.insert(std::pair<unsigned int, unsigned int>(code, i));
	}

	int aux = 0;
	unsigned int value;
	for(auto entry : mcodes) {
		mortonCodes[aux] = entry.first;
		value = entry.second;

		nodes[leafOffset + aux] = temp[value];
		aux++;
	}

	delete[] temp;
	mcodes.clear();

	BVHNode *bvhLeaves = &nodes[nObjects - 1];

	//build BVH
	#pragma omp parallel for 
	for(int i = 0; i < nObjects - 1; i++) {
		// Determine direction of the range (+1 or -1)
		int sign = longestCommonPrefix(i, i + 1, nObjects, mortonCodes) - 
			longestCommonPrefix(i, i - 1, nObjects, mortonCodes);

		int d = sign > 0 ? 1 : -1;

		// Compute upper bound for the length of the range
		int sigMin = longestCommonPrefix(i, i - d, nObjects, mortonCodes);
		int lmax = 2;

		while(longestCommonPrefix(i, i + lmax * d, nObjects, mortonCodes) > sigMin) {
			lmax *= 2;
		}

		// Find the other end using binary search
		int l = 0;
		float divider = 2.0f;
		for(int t = lmax / divider; t >= 1.0f; divider *= 2.0f) {
			if(longestCommonPrefix(i, i + (l + t) * d, nObjects, mortonCodes) > sigMin) {
				l += t;
			}
			t = lmax / divider;
		}

		int j = i + l * d;

		// Find the split position using binary search
		int sigNode = longestCommonPrefix(i, j, nObjects, mortonCodes);
		int s = 0;

		divider = 2.0f;
		for(int t = ceilf(l / divider); t >= 1.0f; divider *= 2.0f) {
			if(longestCommonPrefix(i, i + (s + t) * d, nObjects, mortonCodes) > sigNode) {
				s += t;
			}
			t = ceilf(l / divider);
		}

		int gamma = i + s * d + glm::min(d, 0);

		// Output child pointers
		BVHNode *current = &nodes[i];

		if(glm::min(i, j) == gamma) {
			current->leftChild = &bvhLeaves[gamma];
		}
		else {
			current->leftChild = &nodes[gamma];
		}

		if(glm::max(i, j) == gamma + 1) {
			current->rightChild = &bvhLeaves[gamma + 1];
		}
		else {
			current->rightChild = &nodes[gamma + 1];
		}

		current->leftChild->parent = current;
		current->rightChild->parent = current;
	}
	
	delete[] mortonCodes;
	
	int *lock = new int[nObjects];
	memset(lock, 0, nObjects * sizeof(int));
	
	//Update BB limits
	for(int i = 0; i < nObjects; i++) {
		BVHNode *node = &bvhLeaves[i];

		node = node->parent;
		int index = node - nodes;
		int oldLock = lock[index];
		lock[index] += 1;

		while(1) {
			if(oldLock == 0) {
				break;
			}

			node->min = glm::min(node->leftChild->min, node->rightChild->min);
			node->max = glm::max(node->leftChild->max, node->rightChild->max);

			//if root
			if(node->parent == nullptr) {
				break;
			}

			node = node->parent;

			index = node - nodes;
			oldLock = lock[index];
			lock[index] += 1;
		}
	}

	delete[] lock;

	long time2 = glutGet(GLUT_ELAPSED_TIME);
	std::cout << "BVH building time: " << (float)(time2 - time1) / 1000.0f << "s" << std::endl << std::endl;
}

bool LBVH::findNearestIntersection(Ray ray, RayIntersection *minIntersect) {
	float distance;
	bool intersectionFound = false;

	RayIntersection curr = *minIntersect;

	BVHNode *stackNodes[StackSize];

	unsigned int stackIndex = 0;

	stackNodes[stackIndex++] = nullptr;

	BVHNode *childL, *childR, *node = &nodes[0];

	intersectionFound = node->intersection(ray);

	if(!intersectionFound) {
		return false;
	}

	bool result = false;
	bool lIntersection, rIntersection, traverseL, traverseR;
	while(node != nullptr) {
		lIntersection = rIntersection = traverseL = traverseR = false;

		childL = node->leftChild;
		if(childL != nullptr) {
			lIntersection = childL->intersection(ray, distance);

			if(lIntersection && distance < minIntersect->distance) {
				// Leaf node
				if(childL->shape != nullptr) {
					intersectionFound = childL->shape->intersection(ray, &curr);

					if(intersectionFound && (curr.distance < minIntersect->distance)) {
						result = true;
						*minIntersect = curr;
					}

				}
				else {
					traverseL = true;
				}
			}
		}

		childR = node->rightChild;
		if(childR != nullptr) {
			rIntersection = childR->intersection(ray, distance);

			if(rIntersection && distance < minIntersect->distance) {
				// Leaf node
				if(childR->shape != nullptr) {
					intersectionFound = childR->shape->intersection(ray, &curr);

					if(intersectionFound && (curr.distance < minIntersect->distance)) {
						result = true;
						*minIntersect = curr;
					}

				}
				else {
					traverseR = true;
				}
			}
		}


		if(!traverseL && !traverseR) {
			node = stackNodes[--stackIndex]; // pop

		}
		else {
			node = (traverseL) ? childL : childR;
			if(traverseL && traverseR) {
				stackNodes[stackIndex++] = childR; // push
			}
		}
	}

	return result;
}

bool LBVH::estimateShadowTransmittance(Ray ray, glm::vec3 &color, float &transmittance) {
	bool intersectionFound = false;
	RayIntersection curr = RayIntersection();

	BVHNode *stackNodes[StackSize];

	unsigned int stackIndex = 0;

	stackNodes[stackIndex++] = nullptr;

	BVHNode *childL, *childR, *node = &nodes[0];

	intersectionFound = node->intersection(ray);

	if(!intersectionFound) {
		return false;
	}


	bool result = false;
	bool lIntersection, rIntersection, traverseL, traverseR;

	while(node != nullptr) {
		lIntersection = rIntersection = traverseL = traverseR = false;

		childL = node->leftChild;
		if(childL != nullptr) {
			lIntersection = childL->intersection(ray);

			if(lIntersection) {
				// Leaf node
				if(childL->shape != nullptr) {
					if(transmittance > TRANSMITTANCE_LIMIT) {
						intersectionFound = childL->shape->intersection(ray, &curr);

						if(intersectionFound) {
							transmittance *= curr.shape->material().transparency();
							color *= curr.shape->material().color();
							result = true;

						}

					}
					else {
						return result;
					}

				}
				else {
					traverseL = true;
				}
			}
		}

		childR = node->rightChild;
		if(childR != nullptr) {
			rIntersection = childR->intersection(ray);

			if(rIntersection) {
				// Leaf node
				if(childR->shape != nullptr) {
					if(transmittance > TRANSMITTANCE_LIMIT) {
						intersectionFound = childR->shape->intersection(ray, &curr);

						if(intersectionFound) {
							transmittance *= curr.shape->material().transparency();
							color *= curr.shape->material().color();
							result = true;

						}

					}
					else {
						return result;
					}

				}
				else {
					traverseR = true;
				}
			}
		}


		if(!traverseL && !traverseR) {
			node = stackNodes[--stackIndex]; // pop

		}
		else {
			node = (traverseL) ? childL : childR;
			if(traverseL && traverseR) {
				stackNodes[stackIndex++] = childR; // push
			}
		}
	}

	return result;
}

bool LBVH::findIntersection(Ray ray) {
	bool intersectionFound = false;

	BVHNode *stackNodes[StackSize];

	unsigned int stackIndex = 0;

	stackNodes[stackIndex++] = nullptr;

	BVHNode *childL, *childR, *node = &nodes[0];

	intersectionFound = node->intersection(ray);

	if(!intersectionFound) {
		return false;
	}

	bool lIntersection, rIntersection, traverseL, traverseR;
	while(node != nullptr) {
		lIntersection = rIntersection = traverseL = traverseR = false;

		childL = node->leftChild;
		if(childL != nullptr) {
			lIntersection = childL->intersection(ray);

			if(lIntersection) {
				// Leaf node
				if(childL->shape != nullptr) {
					intersectionFound = childL->shape->intersection(ray, nullptr);

					if(intersectionFound) {
						return true;
					}

				}
				else {
					traverseL = true;
				}
			}
		}

		childR = node->rightChild;
		if(childR != nullptr) {
			rIntersection = childR->intersection(ray);

			if(rIntersection) {
				// Leaf node
				if(childR->shape != nullptr) {
					intersectionFound = childR->shape->intersection(ray, nullptr);

					if(intersectionFound) {
						return true;
					}

				}
				else {
					traverseR = true;
				}
			}
		}


		if(!traverseL && !traverseR) {
			node = stackNodes[--stackIndex]; // pop

		}
		else {
			node = (traverseL) ? childL : childR;
			if(traverseL && traverseR) {
				stackNodes[stackIndex++] = childR; // push
			}
		}
	}

	return false;
}

