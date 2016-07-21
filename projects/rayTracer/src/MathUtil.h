#ifndef _MATH_UTIL_
#define _MATH_UTIL_

#define MAX_DEPTH 3
#define SUPER_SAMPLING 3
#define SUPER_SAMPLING_F (1.0f / SUPER_SAMPLING)
#define SUPER_SAMPLING_2 (SUPER_SAMPLING * SUPER_SAMPLING)
#define SUPER_SAMPLING_2F (1.0f / SUPER_SAMPLING_2)

#define PI 3.14159265359f
#define EPSILON 1E-4f
#define DEG2RAD (PI/180.0f)
#define RAD2DEG (180.0f/PI)

#define StackSize 64
#define sizeMul 6

#define TRANSMITTANCE_LIMIT 0.05f

#include <intrin.h>

inline float haltonSequance(int index, int base) {
	float result = 0.0f;
	float f = 1.0f;

	for(int i = index; i > 0; i = (int)(i / (float)base)) {
		f = f / (float)base;
		result = result + f * (i % base);

	}

	return result;
}

inline glm::vec3 projectVector(glm::vec3 vector, glm::vec3 axis) {
	return axis * glm::dot(vector, axis);
}

inline glm::vec3 projectToPlane(glm::vec3 point, glm::vec3 planeNormal, float planeDistance) {
	float dist = glm::dot(planeNormal, point) + planeDistance;

	return point - dist * planeNormal;
}

inline bool equal(float f1, float f2) {
	float diffAbs = abs(f1 - f2);
	return diffAbs < EPSILON;
}

inline unsigned int expandBits(unsigned int v) {
	v = (v * 0x00010001u) & 0xFF0000FFu;
	v = (v * 0x00000101u) & 0x0F00F00Fu;
	v = (v * 0x00000011u) & 0xC30C30C3u;
	v = (v * 0x00000005u) & 0x49249249u;
	return v;
}

// Calculates a 30-bit Morton code for the
// given 3D point located within the unit cube [0,1].
inline unsigned int morton3D(glm::vec3 center) {
	center.x = fminf(fmaxf(center.x * 1024.0f, 0.0f), 1023.0f);
	center.y = fminf(fmaxf(center.y * 1024.0f, 0.0f), 1023.0f);
	center.z = fminf(fmaxf(center.z * 1024.0f, 0.0f), 1023.0f);

	unsigned int xx = expandBits((unsigned int)center.x);
	unsigned int yy = expandBits((unsigned int)center.y);
	unsigned int zz = expandBits((unsigned int)center.z);
	return (xx << 2) + (yy << 1) + zz;
}

//receives bvh top level min and max and the min and max of a BB
inline glm::vec3 computeCenter(glm::vec3 cmin, glm::vec3 cmax, glm::vec3 min, glm::vec3 max) {
	glm::vec3 tmpMin, tmpMax;

	glm::vec3 len = cmax - cmin;

	tmpMin = (min - cmin) / len;
	tmpMax = (max - cmin) / len;

	glm::vec3 axis = tmpMax - tmpMin;
	float d = glm::length(axis) * 0.5f;

	axis = glm::normalize(axis);

	return tmpMin + d * axis;
}

int const mul[] = { 10, 100, 1000, 10000, 100000, 1000000 };

inline int longestCommonPrefix(int i, int j, int nObjects, unsigned int *mortonCodes) {
	if(j >= 0 && j < nObjects) {
		size_t mci = mortonCodes[i];
		size_t mcj = mortonCodes[j];

		//Concatenate index to key k' = k | index
		if(mci == mcj) {
			int maxIndex = sizeMul - 1;
			int exp = 0;
			if(i > 10) {
				exp = (int) log10(i);
			}

			if(exp < sizeMul) {
				mci = mci * mul[exp] + i;

			}
			else {
				mci *= mul[maxIndex];

				exp -= sizeMul;
				while(exp >= 0) {
					if(exp < sizeMul) {
						mci *= mul[exp];
						break;
					}
					else {
						mci *= mul[maxIndex];
						exp -= sizeMul;
					}
				}

				mci += i;
			}

			exp = 0;
			if(j > 10) {
				exp = (int)log10(j);
			}

			if(exp < sizeMul) {
				mcj = mcj * mul[exp] + j;

			}
			else {
				mcj *= mul[maxIndex];

				exp -= sizeMul;
				while(exp >= 0) {
					if(exp < sizeMul) {
						mcj *= mul[exp];
						break;
					}
					else {
						mcj *= mul[maxIndex];
						exp -= sizeMul;
					}
				}

				mcj += j;
			}

			return (unsigned int) __lzcnt64(mci ^ mcj);
		}

		return __lzcnt((unsigned int) (mci ^ mcj));

	}
	else {
		return -1;
	}
}

#endif