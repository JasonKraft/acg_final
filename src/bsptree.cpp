#include "bsptree.h"
#include "triangle.h"

// DESTRUCTOR
BSPTree::~BSPTree() {
	if (!isLeaf()) {
		delete leftChild;
		delete rightChild;
	}
}

float BSPTree::CastRay(const glm::vec3& dir, const glm::vec3& origin, const glm::vec3& normal, float offset) const {
	// equation for a plane
	// ax + by + cz = d;
	// normal . p + direction = 0
	// plug in ray
	// origin + direction * t = p(t)
	// origin . normal + t * direction . normal = d;
	// t = d - origin.normal / direction.normal;

	return offset - glm::dot(origin, normal) / glm::dot(dir, normal);
}

// function that adds a triangle to the left/right child mesh
// denoted by the side parameter; 0=right, 1=left for indexing in childVertices
// needs to check whether that vertex has already been added to the mesh
void BSPTree::addTriangle(Vertex* a, Vertex* b, Vertex* c, int side, std::vector<std::vector<Vertex*> >& childVertices) {
	Vertex* newA;
	Vertex* newB;
	Vertex* newC;

	// checking if the vertices have already been added to the child mesh and
	// uses its pointer if it has
	// if not, creates a vertex with that position and updates childVertices with that pointer
	if (childVertices[a->getIndex()][side] != NULL) {
		newA = childVertices[a->getIndex()][side];
	} else {
		newA = myMesh.addVertex(a->getPos());
		childVertices[a->getIndex()][side] = newA;
	}

	if (childVertices[b->getIndex()][side] != NULL) {
		newB = childVertices[b->getIndex()][side];
	} else {
		newB = myMesh.addVertex(b->getPos());
		childVertices[b->getIndex()][side] = newB;
	}

	if (childVertices[c->getIndex()][side] != NULL) {
		newC = childVertices[c->getIndex()][side];
	} else {
		newC = myMesh.addVertex(c->getPos());
		childVertices[c->getIndex()][side] = newC;
	}

	// add triangle
	myMesh.addTriangle(newA, newB, newC);
}

void BSPTree::pruneChildMesh(BSPTree* child, const glm::vec3& normal, float offset) {

}

void BSPTree::chop(const glm::vec3& normal, float offset) {
	assert(isLeaf());
	leftChild = new BSPTree(args);
	rightChild = new BSPTree(args);

	glm::vec3 pointOnPlane = normal * offset;

	// 2D vector with pointers to the new Vertex in each child mesh
	// index of the inner vector is corresponds to the index of the vertex from the parent BSPTree mesh
	std::vector<std::vector<Vertex*> > childVertices(myMesh.numVertices(), std::vector<Vertex*>(2));
	// vector<vector<int> > A(dimension, vector<int>(dimension));

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
			rightChild->addTriangle(avert,bvert,cvert,0,childVertices);
			continue;
		}

		// triangle to the left of the plane, add to the left child
		if (distA <= 0 && distB <= 0 && distC <= 0) {
			leftChild->addTriangle(avert,bvert,cvert,1,childVertices);
			continue;
		}

		// triangle intersecting the plane, add it to both children
		rightChild->addTriangle(avert,bvert,cvert,0,childVertices);
		leftChild->addTriangle(avert,bvert,cvert,1,childVertices);


		// need to figure out which vertices are on which side of the plane to know
		// what the new triangles look like to have a clean boundary
		// glm::vec3 Vab = glm::normalize(b - a), Vbc = glm::normalize(c - b), Vca = glm::normalize(a - c);
		// glm::vec3 aint = a + Vab * CastRay(Vab, a, normal, offset);
		// glm::vec3 bint = b + Vbc * CastRay(Vbc, b, normal, offset);
		// glm::vec3 cint = c + Vca * CastRay(Vca, c, normal, offset);
		// glm::vec3 p1, p2;
		//
		// // a is left, others are right
		// if (distA < 0 && distB > 0 && distC > 0) {
		// 	p1 = aint;
		// 	p2 = cint;
		//
		// 	rightChild->addTriangle(b,p2,p1);
		// 	rightChild->addTriangle(c,p2,b);
		// 	leftChild->addTriangle(a,p1,p2);
		// 	continue;
		// }
		//
		// if (distB < 0 && distA > 0 && distC > 0) {
		// 	p1 = aint;
		// 	p2 = bint;
		//
		// 	rightChild->addTriangle(a,p1,p2);
		// 	rightChild->addTriangle(c,a,p2);
		// 	leftChild->addTriangle(b,p2,p1);
		// 	continue;void addTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c);
		// }
		//
		// if (distC < 0 && distA > 0 && distB > 0) {
		// 	p1 = bint;
		// 	p2 = cint;
		//
		// 	rightChild->addTriangle(b,p1,p2);
		// 	rightChild->addTriangle(a,b,p2);
		// 	leftChild->addTriangle(c,p2,p1);
		// 	continue;
		// }
		//
		// if (distA > 0 && distB < 0 && distC < 0) {
		// 	p1 = aint;
		// 	p2 = cint;
		//
		// 	rightChild->addTriangle(a,p1,p2);
		// 	leftChild->addTriangle(b,p2,p1);
		// 	leftChild->addTriangle(c,p2,b);
		// 	continue;
		// }
		//
		// if (distB > 0 && distA < 0 && distC < 0) {
		// 	p1 = bint;
		// 	p2 = aint;
		//
		// 	rightChild->addTriangle(b,p1,p2);
		// 	leftChild->addTriangle(c,p2,p1);
		// 	leftChild->addTriangle(a,p2,c);
		// 	continue;
		// }
		//
		// if (distC > 0 && distA < 0 && distB < 0) {
		// 	p1 = cint;
		// 	p2 = bint;
		//
		// 	rightChild->addTriangle(c,p1,p2);
		// 	leftChild->addTriangle(a,p2,p1);
		// 	leftChild->addTriangle(b,p2,a);
		// 	continue;
		// }
	}

	// cut the triangles in each child mesh that cross the plane
	pruneChildMesh(rightChild, normal, offset);
	pruneChildMesh(leftChild, -normal, offset);

}
