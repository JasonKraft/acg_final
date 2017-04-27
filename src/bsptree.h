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

	// copy constructor
	BSPTree(const BSPTree &tree);

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
	float getGrade() const { return grade; }

	// MODIFIERS
	void setNormal(const glm::vec3& n) { normal = n; }

	// SPECIAL FUNCTIONS
	void Load() { myMesh.Load(); }
	const BoundingBox& getBoundingBox() const { return myMesh.getBoundingBox(); }
	void initializeVBOs() {
		if (isLeaf()) {
			myMesh.initializeVBOs();
		}
		if (leftChild != NULL) { leftChild->initializeVBOs(); }
		if (rightChild != NULL) { rightChild->initializeVBOs(); }
	}
	void setupVBOs() {
		if (isLeaf()) {
			myMesh.setupVBOs();
		}
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

	// ===============
	// CUTTING MESH FUNCTIONS
	void chop(const glm::vec3& normal, float offset);

	// ===============
	// VOLUME FUNCTIONS (dealing with printing volume)
	bool fitsInVolume(float width, float height, float length) {
		// checks if this mesh can fit in the printing volume
		if (isLeaf()) {
			return myMesh.fitsInVolume(width, height, length);
		}

		return leftChild->fitsInVolume(width, height, length) && rightChild->fitsInVolume(width, height, length);
	}
	int largestPart(float width, float height, float length, BSPTree* &lp);
	void getMinMaxOffsetsAlongNorm(const glm::vec3 &normal, float &minOffset, float &maxOffset);
	int getTotalPrintVolumes() {
		// gets the total number of print volumes that can fit in all the partitions seperately
		if (isLeaf()) {
			return myMesh.numPrintVolumes(args->printing_width, args->printing_height, args->printing_length);
		}

		return leftChild->getTotalPrintVolumes() + rightChild->getTotalPrintVolumes();
	}

	// ===============
	// OBJECTIVE functions
	float fPart() {
		//calculate number of printing volumes needed to do the initial mesh
		//sum up all the printing volumes of the leaf nodes/partitions
		int myPrintVolumes = myMesh.numPrintVolumes(args->printing_width, args->printing_height, args->printing_length);
		int totalPrintVolumes = getTotalPrintVolumes();

		return (1.0f / myPrintVolumes) * totalPrintVolumes;
	}
	float fUtil() {
		// find the max of ( 1 - partBBoxVolume/(numPrintingVolume * printingVolume) )
		return 0;
	}
	float fConnector() { return 0; }

	BSPTree* leftChild;
	BSPTree* rightChild;

private:
	float CastRay(const glm::vec3& dir, const glm::vec3& origin, const glm::vec3& normal, float offset) const;
	Triangle* addTriangle(Vertex* a, Vertex* b, Vertex* c, int side, std::vector<std::vector<Vertex*> >& childVertices);
	void pruneChildMesh(const glm::vec3& normal, float offset, std::vector<Triangle*>& trianglesToRemove);

	ArgParser *args;
	unsigned int depth;

	// the mesh that's split by our BSP node
	Mesh myMesh;

	// plane describing our BSP cut
	glm::vec3 normal;
	float offset;

	// objective function grade
	// the lower the grade, the better
	float grade;
};

// class to compare BSPTrees based on their objective function grade
class BSPTreeGreaterThan {
public:
	bool operator() (BSPTree* lhs, BSPTree* rhs) const {
		return lhs->getGrade() > rhs->getGrade();
	}
};

#endif
