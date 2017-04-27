#include <iostream>
#include <string.h>

#include "mesh.h"
#include "edge.h"
#include "utils.h"
#include "vertex.h"
#include "triangle.h"
#include "argparser.h"

int Triangle::next_triangle_id = 0;

// =======================================================================
// MESH COPY CONSTRUCTOR
// =======================================================================
Mesh::Mesh(const Mesh &oldMesh) {
  args = oldMesh.args;
  meshColor = oldMesh.meshColor;

  // copy all vertices
  // this has the added benefit of copying our bounding box
  for (unsigned int i = 0; i < oldMesh.vertices.size(); ++i) {
    addVertex(oldMesh.vertices[i]->getPos());
  }

  // copy all triangles and edges
  for (triangleshashtype::const_iterator iter = oldMesh.triangles.begin(); iter != oldMesh.triangles.end(); ++iter) {
    Triangle* t = iter->second;
    Vertex* a = vertices[(*t)[0]->getIndex()];
    Vertex* b = vertices[(*t)[1]->getIndex()];
    Vertex* c = vertices[(*t)[2]->getIndex()];
    addTriangle(a,b,c);
  }
}

// =======================================================================
// MESH DESTRUCTOR
// =======================================================================

Mesh::~Mesh() {
  clear();
}

void Mesh::clear() {
  // delete all the triangles
  std::vector<Triangle*> todo;
  for (triangleshashtype::iterator iter = triangles.begin();
       iter != triangles.end(); iter++) {
    Triangle *t = iter->second;
    todo.push_back(t);
  }
  int num_triangles = todo.size();
  for (int i = 0; i < num_triangles; i++) {
    removeTriangle(todo[i]);
  }
  // delete all the vertices
  int num_vertices = numVertices();
  for (int i = 0; i < num_vertices; i++) {
    delete vertices[i];
  }
  cleanupVBOs();
}

// =======================================================================
// MODIFIERS:   ADD & REMOVE
// =======================================================================

Vertex* Mesh::addVertex(const glm::vec3 &position) {
  int index = numVertices();
  Vertex *v = new Vertex(index, position);
  vertices.push_back(v);
  if (numVertices() == 1)
    bbox = BoundingBox(position,position);
  else
    bbox.Extend(position);
  return v;
}


Triangle* Mesh::addTriangle(Vertex *a, Vertex *b, Vertex *c) {
  // create the triangle
  Triangle *t = new Triangle();
  // create the edges
  Edge *ea = new Edge(a,b,t);
  Edge *eb = new Edge(b,c,t);
  Edge *ec = new Edge(c,a,t);
  // point the triangle to one of its edges
  t->setEdge(ea);
  // connect the edges to each other
  ea->setNext(eb);
  eb->setNext(ec);
  ec->setNext(ea);
  // verify these edges aren't already in the mesh
  // (which would be a bug, or a non-manifold mesh)
  assert (edges.find(std::make_pair(a,b)) == edges.end());
  assert (edges.find(std::make_pair(b,c)) == edges.end());
  assert (edges.find(std::make_pair(c,a)) == edges.end());
  // add the edges to the master list
  edges[std::make_pair(a,b)] = ea;
  edges[std::make_pair(b,c)] = eb;
  edges[std::make_pair(c,a)] = ec;
  // connect up with opposite edges (if they exist)
  edgeshashtype::iterator ea_op = edges.find(std::make_pair(b,a));
  edgeshashtype::iterator eb_op = edges.find(std::make_pair(c,b));
  edgeshashtype::iterator ec_op = edges.find(std::make_pair(a,c));
  if (ea_op != edges.end()) { ea_op->second->setOpposite(ea); }
  if (eb_op != edges.end()) { eb_op->second->setOpposite(eb); }
  if (ec_op != edges.end()) { ec_op->second->setOpposite(ec); }
  // add the triangle to the master list
  assert (triangles.find(t->getID()) == triangles.end());
  triangles[t->getID()] = t;

  return t;
}


void Mesh::removeTriangle(Triangle *t) {
  Edge *ea = t->getEdge();
  Edge *eb = ea->getNext();
  Edge *ec = eb->getNext();
  Vertex *a = ea->getStartVertex();
  Vertex *b = eb->getStartVertex();
  Vertex *c = ec->getStartVertex();
  // remove these elements from master lists
  edges.erase(std::make_pair(a,b));
  edges.erase(std::make_pair(b,c));
  edges.erase(std::make_pair(c,a));
  triangles.erase(t->getID());
  // clean up memory
  delete ea;
  delete eb;
  delete ec;
  delete t;
}


// =======================================================================
// Helper functions for accessing data in the hash table
// =======================================================================

Edge* Mesh::getMeshEdge(Vertex *a, Vertex *b) const {
  edgeshashtype::const_iterator iter = edges.find(std::make_pair(a,b));
  if (iter == edges.end()) return NULL;
  return iter->second;
}

Vertex* Mesh::getChildVertex(Vertex *p1, Vertex *p2) const {
  vphashtype::const_iterator iter = vertex_parents.find(std::make_pair(p1,p2));
  if (iter == vertex_parents.end()) return NULL;
  return iter->second;
}

void Mesh::setParentsChild(Vertex *p1, Vertex *p2, Vertex *child) {
  assert (vertex_parents.find(std::make_pair(p1,p2)) == vertex_parents.end());
  vertex_parents[std::make_pair(p1,p2)] = child;
}

// =======================================================================
// the load function parses very simple .obj files
// =======================================================================

void Mesh::Load() {
  std::string input_file = args->path + "/" + args->input_file;

  FILE *objfile = fopen(input_file.c_str(),"r");
  if (objfile == NULL) {
    std::cout << "ERROR! CANNOT OPEN '" << input_file << "'\n";
    return;
  }

  char line[200] = "";
  char token[100] = "";
  char atoken[100] = "";
  char btoken[100] = "";
  char ctoken[100] = "";
  float x,y,z;
  int a,b,c,d,e;

  int index = 0;
  int vert_count = 0;
  int vert_index = 1;

  while (fgets(line, 200, objfile)) {
    int token_count = sscanf (line, "%s\n",token);
    if (token_count == -1) continue;
    a = b = c = d = e = -1;
    if (!strcmp(token,"usemtl") ||
	!strcmp(token,"g")) {
      vert_index = 1;
      index++;
    } else if (!strcmp(token,"v")) {
      vert_count++;
      sscanf (line, "%s %f %f %f\n",token,&x,&y,&z);
      addVertex(glm::vec3(x,y,z));
    } else if (!strcmp(token,"f")) {
      int num = sscanf (line, "%s %s %s %s\n",token,
			atoken,btoken,ctoken);
      sscanf (atoken,"%d",&a);
      sscanf (btoken,"%d",&b);
      sscanf (ctoken,"%d",&c);
      assert (num == 4);
      a -= vert_index;
      b -= vert_index;
      c -= vert_index;
      assert (a >= 0 && a < numVertices());
      assert (b >= 0 && b < numVertices());
      assert (c >= 0 && c < numVertices());
      addTriangle(getVertex(a),getVertex(b),getVertex(c));
    } else if (!strcmp(token,"vt")) {
    } else if (!strcmp(token,"vn")) {
    } else if (token[0] == '#') {
    } else {
      printf ("LINE: '%s'",line);
    }
  }

  ComputeGouraudNormals();

  std::cout << "loaded " << numTriangles() << " triangles " << std::endl;
}

// =======================================================================
// this function outputs scene into a very simple .obj file
// =======================================================================

// void Mesh::OutputFile() {
//   std::string output_file = args->path + "/output.obj";
//
//   FILE *objfile = fopen(output_file.c_str(),"w");
//   if (objfile == NULL) {
//     std::cout << "ERROR! CANNOT OPEN '" << output_file << "'\n";
//     return;
//   }
//
//   //loop through all the vertices and write all vertices of one object
//   //after finishing the vertices of an object, create a group
//   //then loop through the indices of the faces of this object and print them
//
//   int indexStart = 0;  //index of the starting face of each object
//
//   for (unsigned int i=0; i<vertices.size(); i++) {
//     unsigned int j;
//
//     //writing the vertices in this object to the file
//     for (j=0; j<vertices[i].size(); j++) {
//       glm::vec3 vPos = vertices[i][j]->getPos();
//       fprintf(objfile, "v %.6f %.6f %.6f\n", vPos.x, vPos.y, vPos.z);
//     }
//
//     //create a group
//     //need to also check that this vector of verices isn't empty
//     if (vertices[i].size() > 0) {
//       fprintf(objfile, "g partition%d\n", i);
//     }
//
//     //writing the faces in this object to the file
//     for (j=indexStart; j<triangles.size(); j++) {
//       Triangle *t = triangles[j];
//       if (t->getObjectIndex() == (int)i) {
//         //this face is part of this object
//         //need to add 1 to index number because for vertices it starts at 1 not 0
//         fprintf(objfile, "f %d %d %d\n", (*t)[0]->getIndex()+1, (*t)[1]->getIndex()+1, (*t)[2]->getIndex()+1);
//       } else {
//         indexStart = j;
//         break;
//       }
//     }
//   }
// }

// =======================================================================

// compute the gouraud normals of all vertices of the mesh and store at each vertex
void Mesh::ComputeGouraudNormals() {
  int i;
  // clear the normals
  for (i = 0; i < numVertices(); i++) {
    getVertex(i)->clearGouraudNormal();
  }
  // loop through all the triangles incrementing the normal at each vertex
  for (triangleshashtype::iterator iter = triangles.begin();
       iter != triangles.end(); iter++) {
    Triangle *t = iter->second;
    glm::vec3 n = ComputeNormal((*t)[0]->getPos(),
                                (*t)[1]->getPos(),
                                (*t)[2]->getPos());
    (*t)[0]->incrGouraudNormal(n);
    (*t)[1]->incrGouraudNormal(n);
    (*t)[2]->incrGouraudNormal(n);
  }
  // finally, normalize the sum at each vertex
  for (i = 0; i < numVertices(); i++) {
    getVertex(i)->normalizeGouraudNormal();
  }
}

// =================================================================

bool Mesh::fitsInVolume(float width, float height, float length) {
  // sort the dimensions of our working volume into small, medium, and large dimensions
  float dims[] = {width, height, length};
  int smallIndex = 0, largeIndex = 0;
  for (int i = 1; i < 3; ++i) {
    if (dims[i] < dims[smallIndex]) { smallIndex = i; }
    if (dims[i] > dims[largeIndex]) { largeIndex = i; }
  }
  float small = dims[smallIndex];
  float medium = dims[3-smallIndex-largeIndex];
  float large = dims[largeIndex];

  // for now just use axis-aligned bounding box
  // and sort their dimensions just as before
  glm::vec3 boundingBoxDimensions = bbox.getMax() - bbox.getMin();
  float bdims[] = {boundingBoxDimensions.x, boundingBoxDimensions.y, boundingBoxDimensions.z};
  int bsmallIndex = 0, blargeIndex = 0;
  for (int i = 0; i < 3; ++i) {
    if (bdims[i] < bdims[bsmallIndex]) { bsmallIndex = i; }
    if (bdims[i] > bdims[blargeIndex]) { blargeIndex = i; }
  }

  float bsmall = bdims[bsmallIndex];
  float bmedium = bdims[3-bsmallIndex-blargeIndex];
  float blarge = bdims[blargeIndex];
  return bsmall <= small && bmedium <= medium && blarge <= large;
}

// OBJECTIVE FUNCTION: fPart
// estimates the number of print volumes required to make the current part
bool Mesh::fPart(float width, float height, float length) {
  // sort the dimensions of our working volume into small, medium, and large dimensions
  float dims[] = {width, height, length};
  int smallIndex = 0, largeIndex = 0;
  for (int i = 1; i < 3; ++i) {
    if (dims[i] < dims[smallIndex]) { smallIndex = i; }
    if (dims[i] > dims[largeIndex]) { largeIndex = i; }
  }
  float small = dims[smallIndex];
  float medium = dims[3-smallIndex-largeIndex];
  float large = dims[largeIndex];

  // for now just use axis-aligned bounding box
  // and sort their dimensions just as before
  glm::vec3 boundingBoxDimensions = bbox.getMax() - bbox.getMin();
  float bdims[] = {boundingBoxDimensions.x, boundingBoxDimensions.y, boundingBoxDimensions.z};
  int bsmallIndex = 0, blargeIndex = 0;
  for (int i = 0; i < 3; ++i) {
    if (bdims[i] < bdims[bsmallIndex]) { bsmallIndex = i; }
    if (bdims[i] > bdims[blargeIndex]) { blargeIndex = i; }
  }

  float bsmall = bdims[bsmallIndex];
  float bmedium = bdims[3-bsmallIndex-blargeIndex];
  float blarge = bdims[blargeIndex];

  int numSmall = (int)ceil(bsmall/small);
  int numMedium = (int)ceil(bmedium/medium);
  int numLarge = (int)ceil(blarge/large);

  return std::max(std::max(numSmall, numMedium), numLarge);
}
