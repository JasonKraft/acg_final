#include "bsptree.h"

// DESTRUCTOR
BSPTree::~BSPTree() {
	if (!isLeaf()) {
		delete leftChild;
		delete rightChild;
	}
}
