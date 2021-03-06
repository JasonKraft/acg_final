#include "bsptree.h"
#include "triangle.h"
#include "utils.h"

// COPY CONSTRUCTOR
BSPTree::BSPTree(const BSPTree &tree) : myMesh(tree.myMesh) {
	normal = tree.normal;
	offset = tree.offset;
	args = tree.args;
	depth = tree.depth;
	grade = tree.grade;

	// printf("COPY CONSTRUCTOR\n");
	if (!tree.isLeaf()) {
		leftChild = new BSPTree(*(tree.leftChild));
		rightChild = new BSPTree(*(tree.rightChild));
	} else {
		leftChild = NULL;
		rightChild = NULL;
	}
}

// ASSIGNMENT OPERATOR
BSPTree& BSPTree::operator= (const BSPTree& tree) {
	normal = tree.normal;
	offset = tree.offset;
	args = tree.args;
	depth = tree.depth;
	grade = tree.grade;
	myMesh = tree.myMesh;

	// printf("ASSIGNMENT OP\n");
	if (!tree.isLeaf()) {
		leftChild = new BSPTree(*(tree.leftChild));
		rightChild = new BSPTree(*(tree.rightChild));
	} else {
		leftChild = NULL;
		rightChild = NULL;
	}

	return *this;
}

// DESTRUCTOR
BSPTree::~BSPTree() {
	// printf("DESTRUCTOR\n");
	// if (leftChild == NULL) {
	// 	printf("left null\n");
	// }
	// if (rightChild == NULL) {
	// 	printf("right null\n");
	// }
	if (!isLeaf()) {
		// printf("deleting children\n");
		delete leftChild;
		delete rightChild;

		leftChild = NULL;
		rightChild = NULL;
	}

	// printf("deleting bsp tree\n");
	// myMesh.clear();
	// printf("cleared mesh\n");
}

float BSPTree::CastRay(const glm::vec3& dir, const glm::vec3& origin, const glm::vec3& normal, float offset) const {
	// equation for a plane
	// ax + by + cz = d;
	// normal . p + direction = 0
	// plug in ray
	// origin + direction * t = p(t)
	// origin . normal + t * direction . normal = d;
	// t = d - origin.normal / direction.normal;

	return (offset - glm::dot(origin, normal)) / glm::dot(dir, normal);
}

// function that adds a triangle to the left/right child mesh
// denoted by the side parameter; 0=right, 1=left for indexing in childVertices
// needs to check whether that vertex has already been added to the mesh
// if triangle being added is intersecting the plane, don't want its vertices to impact the bounding box
Triangle* BSPTree::addTriangle(Vertex* a, Vertex* b, Vertex* c, int side, std::vector<std::vector<Vertex*> >& childVertices,
																bool addToBoundingBox) {
	Vertex* newA;
	Vertex* newB;
	Vertex* newC;

	// checking if the vertices have already been added to the child mesh and
	// uses its pointer if it has
	// if not, creates a vertex with that position and updates childVertices with that pointer
	if (childVertices[a->getIndex()][side] != NULL) {
		newA = childVertices[a->getIndex()][side];
	} else {
		newA = myMesh.addVertex(a->getPos(), addToBoundingBox);
		childVertices[a->getIndex()][side] = newA;
	}

	if (childVertices[b->getIndex()][side] != NULL) {
		newB = childVertices[b->getIndex()][side];
	} else {
		newB = myMesh.addVertex(b->getPos(), addToBoundingBox);
		childVertices[b->getIndex()][side] = newB;
	}

	if (childVertices[c->getIndex()][side] != NULL) {
		newC = childVertices[c->getIndex()][side];
	} else {
		newC = myMesh.addVertex(c->getPos(), addToBoundingBox);
		childVertices[c->getIndex()][side] = newC;
	}

	// add triangle
	return myMesh.addTriangle(newA, newB, newC);
}

// cuts off the excess parts of triangles crossing the cutting plane
// assumes child always in direction of "right" of plane to reduce having to check which side its on
void BSPTree::pruneChildMesh(const glm::vec3& normal, float offset, std::vector<Triangle*>& trianglesToRemove) {
	// clear out any previous relationships between vertices
  myMesh.vertex_parents.clear();

	glm::vec3 pointOnPlane = normal * offset;

	// loop through all the triangles to find the triangles that cross the boundary
	for (unsigned int i=0; i<trianglesToRemove.size(); i++) {

		Triangle *t = trianglesToRemove[i];
		Vertex *avert = (*t)[0];
		Vertex *bvert = (*t)[1];
		Vertex *cvert = (*t)[2];
		glm::vec3 a = avert->getPos();
		glm::vec3 b = bvert->getPos();
		glm::vec3 c = cvert->getPos();

		float distA = glm::dot(normal, a-pointOnPlane);
		float distB = glm::dot(normal, b-pointOnPlane);
		float distC = glm::dot(normal, c-pointOnPlane);

		glm::vec3 Vab = glm::normalize(b - a), Vbc = glm::normalize(c - b), Vca = glm::normalize(a - c);
		glm::vec3 aint = a + Vab * CastRay(Vab, a, normal, offset);
		glm::vec3 bint = b + Vbc * CastRay(Vbc, b, normal, offset);
		glm::vec3 cint = c + Vca * CastRay(Vca, c, normal, offset);
		glm::vec3 p1, p2;
		Vertex *newPoint1;
		Vertex *newPoint2;

		// a is left, others are right
		if (distA < 0 && distB > 0 && distC > 0) {
			p1 = cint;
			p2 = aint;

			// check if a new point along the edge exists already between edges that intersect the plane
			// if there isn't one already, create that vertex and set its parent
			newPoint1 = myMesh.getChildVertex(cvert, avert);
			if (newPoint1 == NULL) {
				newPoint1 = myMesh.addVertex(p1);
				myMesh.setParentsChild(cvert, avert, newPoint1);
			}
			newPoint2 = myMesh.getChildVertex(avert, bvert);
			if (newPoint2 == NULL) {
				newPoint2 = myMesh.addVertex(p2);
				myMesh.setParentsChild(avert, bvert, newPoint2);
			}

			// remove the intersecting triangle first!
			myMesh.removeTriangle(t);

			// add these new triangles to the mesh and delete the old one
			myMesh.addTriangle(cvert,newPoint1,newPoint2);
			myMesh.addTriangle(bvert,cvert,newPoint2);
			continue;
		}

		// b is left, others are right
		if (distB < 0 && distA > 0 && distC > 0) {
			p1 = aint;
			p2 = bint;

			// check if a new point along the edge exists already between edges that intersect the plane
			// if there isn't one already, create that vertex
			newPoint1 = myMesh.getChildVertex(avert, bvert);
			if (newPoint1 == NULL) {
				newPoint1 = myMesh.addVertex(p1);
				myMesh.setParentsChild(avert, bvert, newPoint1);
			}
			newPoint2 = myMesh.getChildVertex(bvert, cvert);
			if (newPoint2 == NULL) {
				newPoint2 = myMesh.addVertex(p2);
				myMesh.setParentsChild(bvert, cvert, newPoint2);
			}

			// remove the intersecting triangle first!
			myMesh.removeTriangle(t);

			// add these new triangles to the mesh and delete the old one
			myMesh.addTriangle(avert,newPoint1,newPoint2);
			myMesh.addTriangle(cvert,avert,newPoint2);
			continue;
		}

		// c is left, others are right
		if (distC < 0 && distA > 0 && distB > 0) {
			p1 = bint;
			p2 = cint;

			// check if a new point along the edge exists already between edges that intersect the plane
			// if there isn't one already, create that vertex
			newPoint1 = myMesh.getChildVertex(bvert, cvert);
			if (newPoint1 == NULL) {
				newPoint1 = myMesh.addVertex(p1);
				myMesh.setParentsChild(bvert, cvert, newPoint1);
			}
			newPoint2 = myMesh.getChildVertex(cvert, avert);
			if (newPoint2 == NULL) {
				newPoint2 = myMesh.addVertex(p2);
				myMesh.setParentsChild(cvert, avert, newPoint2);
			}

			// remove the intersecting triangle first!
			myMesh.removeTriangle(t);

			// add these new triangles to the mesh and delete the old one
			myMesh.addTriangle(bvert,newPoint1,newPoint2);
			myMesh.addTriangle(avert,bvert,newPoint2);
			continue;
		}

		// a is right, others are left
		if (distA > 0 && distB < 0 && distC < 0) {
			p1 = aint;
			p2 = cint;

			// check if a new point along the edge exists already between edges that intersect the plane
			// if there isn't one already, create that vertex
			newPoint1 = myMesh.getChildVertex(avert, bvert);
			if (newPoint1 == NULL) {
				newPoint1 = myMesh.addVertex(p1);
				myMesh.setParentsChild(avert, bvert, newPoint1);
			}
			newPoint2 = myMesh.getChildVertex(cvert, avert);
			if (newPoint2 == NULL) {
				newPoint2 = myMesh.addVertex(p2);
				myMesh.setParentsChild(cvert, avert, newPoint2);
			}

			// remove the intersecting triangle first!
			myMesh.removeTriangle(t);

			// add these new triangles to the mesh and delete the old one
			myMesh.addTriangle(avert,newPoint1,newPoint2);
			continue;
		}

		// b is right, others are left
		if (distB > 0 && distA < 0 && distC < 0) {
			p1 = bint;
			p2 = aint;

			// check if a new point along the edge exists already between edges that intersect the plane
			// if there isn't one already, create that vertex
			newPoint1 = myMesh.getChildVertex(bvert, cvert);
			if (newPoint1 == NULL) {
				newPoint1 = myMesh.addVertex(p1);
				myMesh.setParentsChild(bvert, cvert, newPoint1);
			}
			newPoint2 = myMesh.getChildVertex(avert, bvert);
			if (newPoint2 == NULL) {
				newPoint2 = myMesh.addVertex(p2);
				myMesh.setParentsChild(avert, bvert, newPoint2);
			}

			// remove the intersecting triangle first!
			myMesh.removeTriangle(t);

			// add these new triangles to the mesh and delete the old one
			myMesh.addTriangle(bvert,newPoint1,newPoint2);
			continue;
		}

		// c is right, others are left
		if (distC > 0 && distA < 0 && distB < 0) {
			p1 = cint;
			p2 = bint;

			// check if a new point along the edge exists already between edges that intersect the plane
			// if there isn't one already, create that vertex
			newPoint1 = myMesh.getChildVertex(cvert, avert);
			if (newPoint1 == NULL) {
				newPoint1 = myMesh.addVertex(p1);
				myMesh.setParentsChild(cvert, avert, newPoint1);
			}
			newPoint2 = myMesh.getChildVertex(bvert, cvert);
			if (newPoint2 == NULL) {
				newPoint2 = myMesh.addVertex(p2);
				myMesh.setParentsChild(bvert, cvert, newPoint2);
			}

			// remove the intersecting triangle first!
			myMesh.removeTriangle(t);

			// add these new triangles to the mesh and delete the old one
			myMesh.addTriangle(cvert,newPoint1,newPoint2);
			continue;
		}

		// a is on the plane
		if (distA == 0) {
			p1 = bint;

			// check if a new point along the edge exists already between edges that intersect the plane
			// if there isn't one already, create that vertex
			newPoint1 = myMesh.getChildVertex(bvert, cvert);
			if (newPoint1 == NULL) {
				newPoint1 = myMesh.addVertex(p1);
				myMesh.setParentsChild(bvert, cvert, newPoint1);
			}

			// remove the intersecting triangle first!
			myMesh.removeTriangle(t);

			// add these new triangles to the mesh and delete the old one
			myMesh.addTriangle(bvert,newPoint1,avert);
			continue;
		}

		// b is on the plane
		if (distB == 0) {
			p1 = cint;

			// check if a new point along the edge exists already between edges that intersect the plane
			// if there isn't one already, create that vertex
			newPoint1 = myMesh.getChildVertex(cvert, avert);
			if (newPoint1 == NULL) {
				newPoint1 = myMesh.addVertex(p1);
				myMesh.setParentsChild(cvert, avert, newPoint1);
			}

			// remove the intersecting triangle first!
			myMesh.removeTriangle(t);

			// add these new triangles to the mesh and delete the old one
			myMesh.addTriangle(cvert,newPoint1,bvert);
			continue;
		}

		// c is on the plane
		if (distC == 0) {
			p1 = aint;

			// check if a new point along the edge exists already between edges that intersect the plane
			// if there isn't one already, create that vertex
			newPoint1 = myMesh.getChildVertex(avert, bvert);
			if (newPoint1 == NULL) {
				newPoint1 = myMesh.addVertex(p1);
				myMesh.setParentsChild(avert, bvert, newPoint1);
			}

			// remove the intersecting triangle first!
			myMesh.removeTriangle(t);

			// add these new triangles to the mesh and delete the old one
			myMesh.addTriangle(avert,newPoint1,cvert);
			continue;
		}
	}
}

// cuts the mesh along the plane
void BSPTree::chop(const glm::vec3& normal, float offset) {
	// printf("begin chop %f %f %f, %f\n", normal.x, normal.y, normal.z, offset);
	assert(isLeaf());
	assert(numVertices() > 0);
	leftChild = new BSPTree(args);
	rightChild = new BSPTree(args);
	this->normal = normal;
	this->offset = offset;

	glm::vec3 pointOnPlane = normal * offset;

	// 2D vector with pointers to the new Vertex in each child mesh
	// index of the inner vector is corresponds to the index of the vertex from the parent BSPTree mesh
	std::vector<std::vector<Vertex*> > childVertices(myMesh.numVertices(), std::vector<Vertex*>(2,NULL));

	// vector of triangles that intersect the plane, one for each child
	// holds the pointers to the triangles for that respective mesh
	std::vector<Triangle*> trianglesToRemoveL;
	std::vector<Triangle*> trianglesToRemoveR;

	// go through all the triangles and if they are on the right/left side of the cut
	// put them in the right/left child respectively
	// if the triangle is intersecting the cut plane, place them in both children
	for (triangleshashtype::iterator iter = myMesh.triangles.begin();
       iter != myMesh.triangles.end(); iter++) {
		Triangle *t = iter->second;
		Vertex *avert = (*t)[0];
		Vertex *bvert = (*t)[1];
		Vertex *cvert = (*t)[2];
		glm::vec3 a = avert->getPos();
		glm::vec3 b = bvert->getPos();
		glm::vec3 c = cvert->getPos();

		float distA = glm::dot(normal, a-pointOnPlane);
		float distB = glm::dot(normal, b-pointOnPlane);
		float distC = glm::dot(normal, c-pointOnPlane);

		// triangle to the right of the plane, add to the right child
		if (distA >= 0 && distB >= 0 && distC >= 0) {
			// printf("adding right\n");
			rightChild->addTriangle(avert,bvert,cvert,0,childVertices, true);
			continue;
		}

		// triangle to the left of the plane, add to the left child
		if (distA <= 0 && distB <= 0 && distC <= 0) {
			// printf("adding left\n");
			leftChild->addTriangle(avert,bvert,cvert,1,childVertices, true);
			continue;
		}

		// printf("adding to both\n");
		// triangle intersecting the plane, add it to both children and to vector
		Triangle *RT = rightChild->addTriangle(avert,bvert,cvert,0,childVertices, false);
		Triangle *LT = leftChild->addTriangle(avert,bvert,cvert,1,childVertices, false);

		trianglesToRemoveR.push_back(RT);
		trianglesToRemoveL.push_back(LT);
	}

	assert(rightChild->numVertices() > 0);
	assert(leftChild->numVertices() > 0);

	// cut the triangles in each child mesh that cross the plane
	rightChild->pruneChildMesh(normal, offset, trianglesToRemoveR);
	leftChild->pruneChildMesh(-normal, -offset, trianglesToRemoveL);

	assert(rightChild->numVertices() > 0);
	assert(leftChild->numVertices() > 0);
}

// finds the largest partition and returns the number of printing volumes
// needed for it and the pointer to it via pass by reference
int BSPTree::largestPart(float width, float height, float length, BSPTree* &lp) {
	if (isLeaf()) {
		lp = this;
		return myMesh.numPrintVolumes(width, height, length);
	}

	BSPTree* lpl;
	BSPTree* lpr;
	int numPartsLeft = leftChild->largestPart(width, height, length, lpl);
	int numPartsRight = rightChild->largestPart(width, height, length, lpr);

	if (numPartsLeft > numPartsRight) {
		lp = lpl;
		return numPartsLeft;
	}

	lp = lpr;
	return numPartsRight;
}

	void BSPTree::getMinMaxOffsetsAlongNorm(const glm::vec3 &normal, float &minOffset, float &maxOffset) {
		assert(myMesh.numVertices() > 0);

		edgeshashtype::iterator iter = myMesh.edges.begin();

		minOffset = glm::dot(normal, (*iter).second->getStartVertex()->getPos());
		maxOffset = glm::dot(normal, (*iter).second->getStartVertex()->getPos());
		iter++;

		for (; iter != myMesh.edges.end(); ++iter) {
			float off = glm::dot(normal, (*iter).second->getStartVertex()->getPos());
			if (off < minOffset) {
				minOffset = off;
			}

			if (off > maxOffset) {
				maxOffset = off;
			}
		}
	}
