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

void BSPTree::addTriangle(Vertex* a, Vertex* b, Vertex* c) {
	Vertex* newA = myMesh.addVertex(a);
	Vertex* newB = myMesh.addVertex(b);
	Vertex* newC = myMesh.addVertex(c);



	myMesh.addTriangle(newA, newB, newC);
}

void BSPTree::chop(const glm::vec3& normal, float offset) {
	assert(isLeaf());
	leftChild = new BSPTree(args);
	rightChild = new BSPTree(args);

	glm::vec3 pointOnPlane = normal * offset;

	for (triangleshashtype::iterator iter = myMesh.triangles.begin();
       iter != myMesh.triangles.end(); iter++) {
		Triangle *t = iter->second;
		Vertex *avert = (*t)[0];
		Vertex *bvert = (*t)[0];
		Vertex *cvert = (*t)[0];
		glm::vec3 a = avert->getPos();
		glm::vec3 b = bvert->getPos();
		glm::vec3 c = cvert->getPos();

		float distA = glm::dot(normal, a-pointOnPlane);
		float distB = glm::dot(normal, b-pointOnPlane);
		float distC = glm::dot(normal, c-pointOnPlane);

		if (distA >= 0 && distB >= 0 && distC >= 0) {
			Edge* ab = rightChild->myMesh.getEdge(avert,bvert);
			Edge* bc = rightChild->myMesh.getEdge(bvert,cvert);
			Edge* ca = rightChild->myMesh.getEdge(cvert,avert);
			rightChild->addTriangle(a,b,c);
			continue;
		}

		if (distA <= 0 && distB <= 0 && distC <= 0) {
			leftChild->addTriangle(a,b,c);
			continue;
		}

		glm::vec3 Vab = glm::normalize(b - a), Vbc = glm::normalize(c - b), Vca = glm::normalize(a - c);
		glm::vec3 aint = a + Vab * CastRay(Vab, a, normal, offset);
		glm::vec3 bint = b + Vbc * CastRay(Vbc, b, normal, offset);
		glm::vec3 cint = c + Vca * CastRay(Vca, c, normal, offset);
		glm::vec3 p1, p2;

		// a is left, others are right
		if (distA < 0 && distB > 0 && distC > 0) {
			p1 = aint;
			p2 = cint;

			rightChild->addTriangle(b,p2,p1);
			rightChild->addTriangle(c,p2,b);
			leftChild->addTriangle(a,p1,p2);
			continue;
		}

		if (distB < 0 && distA > 0 && distC > 0) {
			p1 = aint;
			p2 = bint;

			rightChild->addTriangle(a,p1,p2);
			rightChild->addTriangle(c,a,p2);
			leftChild->addTriangle(b,p2,p1);
			continue;
		}

		if (distC < 0 && distA > 0 && distB > 0) {
			p1 = bint;
			p2 = cint;

			rightChild->addTriangle(b,p1,p2);
			rightChild->addTriangle(a,b,p2);
			leftChild->addTriangle(c,p2,p1);
			continue;
		}

		if (distA > 0 && distB < 0 && distC < 0) {
			p1 = aint;
			p2 = cint;

			rightChild->addTriangle(a,p1,p2);
			leftChild->addTriangle(b,p2,p1);
			leftChild->addTriangle(c,p2,b);
			continue;
		}

		if (distB > 0 && distA < 0 && distC < 0) {
			p1 = bint;
			p2 = aint;

			rightChild->addTriangle(b,p1,p2);
			leftChild->addTriangle(c,p2,p1);
			leftChild->addTriangle(a,p2,c);
			continue;
		}

		if (distC > 0 && distA < 0 && distB < 0) {
			p1 = cint;
			p2 = bint;

			rightChild->addTriangle(c,p1,p2);
			leftChild->addTriangle(a,p2,p1);
			leftChild->addTriangle(b,p2,a);
			continue;
		}
	}
}
