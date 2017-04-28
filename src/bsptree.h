#ifndef _BSPTREE_H_
#define _BSPTREE_H_

#include <cstdlib>
#include <vector>
#include "glCanvas.h"
#include "mesh.h"

// A hierarchical spatial data structure to store partitions of our mesh.

class BSPTree {
public:
	BSPTree() {
		args = NULL;
		depth = 0;
		leftChild = NULL;
		rightChild = NULL;
	}
	BSPTree(ArgParser *_args, unsigned int _depth = 0) : myMesh(_args) {
		args = _args;
		depth = _depth;
		leftChild = NULL;
		rightChild = NULL;
	}

	// copy constructor
	BSPTree(const BSPTree &tree);

	~BSPTree();

	// assignment operator
	BSPTree& operator= (const BSPTree& tree);

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
	int numVertices() const { return myMesh.numVertices(); }

	// MODIFIERS
	void setNormal(const glm::vec3& n) { normal = n; }
	void setOffset(float o) { offset = o; }
	void setGrade(float g) { grade = g; }

	// SPECIAL FUNCTIONS
	void Load() { myMesh.Load(); }
	const BoundingBox& getBoundingBox() const { return myMesh.getBoundingBox(); }
	void initializeVBOs() {
		printf("INIALIZE VBOS\n");
		if (isLeaf()) {
			myMesh.initializeVBOs();
		}
		if (leftChild != NULL) { leftChild->initializeVBOs(); }
		if (rightChild != NULL) { rightChild->initializeVBOs(); }
	}
	void setupVBOs() {
		printf("SETUP VBOS\n");
		if (isLeaf()) {
			myMesh.setupVBOs();
		}
		if (leftChild != NULL) { leftChild->setupVBOs(); }
		if (rightChild != NULL) { rightChild->setupVBOs(); }
	}
	void drawVBOs() {
		// printf("DRAW VBOS\n");
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
		// printf("FITS IN VOLUME\n");
		// checks if this mesh can fit in the printing volume
		if (isLeaf()) {
			return myMesh.fitsInVolume(width, height, length);
		}

		return leftChild->fitsInVolume(width, height, length) && rightChild->fitsInVolume(width, height, length);
	}
	int largestPart(float width, float height, float length, BSPTree* &lp);
	void getMinMaxOffsetsAlongNorm(const glm::vec3 &normal, float &minOffset, float &maxOffset);
	int getTotalPrintVolumes() {
		// printf("GET TOTAL PRINT VOLUMES");
		// gets the total number of print volumes that can fit in all the partitions seperately
		if (isLeaf()) {
			return myMesh.numPrintVolumes(args->printing_width, args->printing_height, args->printing_length);
		}

		return leftChild->getTotalPrintVolumes() + rightChild->getTotalPrintVolumes();
	}

	// ===============
	// OBJECTIVE functions
	float fPart() {
		// calculate number of printing volumes needed to do the initial mesh
		// sum up all the printing volumes of the leaf nodes/partitions
		// this tries to minimize the number of partitions needed
		int myPrintVolumes = myMesh.numPrintVolumes(args->printing_width, args->printing_height, args->printing_length);
		int totalPrintVolumes = getTotalPrintVolumes();

		return (1.0f / myPrintVolumes) * totalPrintVolumes;
	}
	float fUtil() {
		// finds the max of ( 1 - partBBoxVolume/(numPrintVolumes * printingVolume) ) of all partitions
		// this ensures that a partition is not too small
		if (isLeaf()) {
			float width = args->printing_width;
			float height = args->printing_height;
			float length = args->printing_length;
			float printingVolume = width * height * length;
			return 1 - myMesh.getBBVolume() / (myMesh.numPrintVolumes(width, height, length)  * printingVolume);
		}

		return std::max( leftChild->fUtil(), rightChild->fUtil() );
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
