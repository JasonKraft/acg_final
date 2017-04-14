#ifndef _BSPTREE_H_
#define _BSPTREE_H_

#include <cstdlib>
#include <vector>
#include "glCanvas.h"

// A hierarchical spatial data structure to store partitions of our mesh.

class BSPTree {
public:
	BSPTree(glm::vec3 _normal, glm::vec3 _position, unsigned int _leftIndex,
			unsigned int _rightIndex, unsigned int _depth = 0, BSPTree* _leftChild = NULL,
			BSPTree* _rightChild = NULL) {
		depth = _depth;
		children = std::vector<BSPTree*>();
		normal = _normal;
		position = _position;
		leftIndex = _leftIndex;
		rightIndex = _rightIndex;
		leftChild = _leftChild;
		rightChild = _rightChild;
	}
	~BSPTree();

	// ACCESSORS
	const glm::vec3& getNormal() const { return normal; }
	const glm::vec3& getPosition() const { return position; }
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
};

#endif
