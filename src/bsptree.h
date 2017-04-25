#ifndef _BSPTREE_H_
#define _BSPTREE_H_

#include <cstdlib>
#include <vector>
#include "glCanvas.h"

// A hierarchical spatial data structure to store partitions of our mesh.

class BSPTree {
public:
	BSPTree(glm::vec3 _normal, float _offset, unsigned int _leftIndex,
			unsigned int _rightIndex, unsigned int _depth = 0, BSPTree* _leftChild = NULL,
			BSPTree* _rightChild = NULL) {
		depth = _depth;
		normal = _normal;
		offset = _offset;
		leftIndex = _leftIndex;
		rightIndex = _rightIndex;
		leftChild = _leftChild;
		rightChild = _rightChild;
	}
	~BSPTree();

	// ACCESSORS
	const glm::vec3& getNormal() const { return normal; }
	float getOffset() const { return offset; }
	unsigned int getDepth() const { return depth; }
	bool isLeaf() const {
		if (leftChild == NULL && rightChild == NULL) return true;
		assert(leftChild != NULL && rightChild != NULL);
		return false;
	}
	unsigned int getLeftIndex() const { return leftIndex; }
	unsigned int getRightIndex() const { return rightIndex; }

	BSPTree* leftChild;
	BSPTree* rightChild;

private:
	unsigned int depth;

	// these indices correspond to indices in our mesh vector
	unsigned int leftIndex;
	unsigned int rightIndex;

	glm::vec3 normal;
	float offset;
};

#endif
