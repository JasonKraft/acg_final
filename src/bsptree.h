#ifndef _BSPTREE_H_
#define _BSPTREE_H_

#include <cstdlib>
#include <vector>
#include "glCanvas.h"
#include "mesh.h"

// A hierarchical spatial data structure to store partitions of our mesh.

class BSPTree {
public:
	BSPTree(ArgParser *_args, unsigned int _depth = 0) : myMesh(_args) {
		args = _args;
		depth = _depth;
		leftChild = rightChild = NULL;
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

	// MODIFIERS
	void setNormal(const glm::vec3& n) { normal = n; }

	// SPECIAL FUNCTIONS
	void Load() {
		printf("hello, world!\n");
		myMesh.Load(); }
	const BoundingBox& getBoundingBox() const { return myMesh.getBoundingBox(); }
	void initializeVBOs() {
		myMesh.initializeVBOs();
		if (leftChild != NULL) { leftChild->initializeVBOs(); }
		if (rightChild != NULL) { rightChild->initializeVBOs(); }
	}
	void setupVBOs() {
		myMesh.setupVBOs();
		if (leftChild != NULL) { leftChild->setupVBOs(); }
		if (rightChild != NULL) { rightChild->setupVBOs(); }
	}
	void drawVBOs() {
		if (isLeaf()) {
			myMesh.drawVBOs();
			return;
		}

		leftChild->drawVBOs();
		rightChild->drawVBOs();
	}
	void cleanupVBOs() {
		myMesh.cleanupVBOs();
		if (leftChild != NULL) { leftChild->cleanupVBOs(); }
		if (rightChild != NULL) { rightChild->cleanupVBOs(); }
	}
	glm::vec3 LightPosition() const { return myMesh.LightPosition(); }


	BSPTree* leftChild;
	BSPTree* rightChild;

private:
	ArgParser *args;
	unsigned int depth;

	// the mesh that's split by our BSP node
	Mesh myMesh;

	// plane describing our BSP cut
	glm::vec3 normal;
	float offset;
};

#endif
