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
// MESH DESTRUCTOR
// =======================================================================

Mesh::~Mesh() {
  // delete all the triangles
  std::vector< std::pair<Triangle*,int> > todo;
  for (int i = 0; i < triangles.size(); i++) {
    for (triangleshashtype::iterator iter = triangles[i].begin();
         iter != triangles[i].end(); iter++) {
      Triangle *t = iter->second;
      todo.push_back(std::make_pair(t,i));
    }
  }
  int num_triangles = todo.size();
  for (int i = 0; i < num_triangles; i++) {
    removeTriangle(todo[i].first,todo[i].second);
  }
  // delete all the vertices
  int num_objects = numObjects();
  for (int i = 0; i < num_objects; i++) {
    int num_vertices = numVertices(i);
    for (int j = 0; j < num_vertices; j++) {
        delete vertices[i][j];
    }
  }
  cleanupVBOs();
}

// =======================================================================
// MODIFIERS:   ADD & REMOVE
// =======================================================================

Vertex* Mesh::addVertex(const glm::vec3 &position, int i) {
  int index = totalVertices();
  Vertex *v = new Vertex(index, i, position);
  vertices[i].push_back(v);
  total_vertices++;
  if (totalVertices() == 1)
    bbox = BoundingBox(position,position);
  else
    bbox.Extend(position);
  return v;
}


void Mesh::addTriangle(Vertex *a, Vertex *b, Vertex *c, int index) {
  // create the triangle
  Triangle *t = new Triangle(index);
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
  assert (edges[index].find(std::make_pair(a,b)) == edges[index].end());
  assert (edges[index].find(std::make_pair(b,c)) == edges[index].end());
  assert (edges[index].find(std::make_pair(c,a)) == edges[index].end());
  // add the edges to the master list
  edges[index][std::make_pair(a,b)] = ea;
  edges[index][std::make_pair(b,c)] = eb;
  edges[index][std::make_pair(c,a)] = ec;
  // connect up with opposite edges (if they exist)
  edgeshashtype::iterator ea_op = edges[index].find(std::make_pair(b,a));
  edgeshashtype::iterator eb_op = edges[index].find(std::make_pair(c,b));
  edgeshashtype::iterator ec_op = edges[index].find(std::make_pair(a,c));
  if (ea_op != edges[index].end()) { ea_op->second->setOpposite(ea); }
  if (eb_op != edges[index].end()) { eb_op->second->setOpposite(eb); }
  if (ec_op != edges[index].end()) { ec_op->second->setOpposite(ec); }
  // add the triangle to the master list
  assert (triangles[index].find(t->getID()) == triangles[index].end());
  triangles[index][t->getID()] = t;
}


void Mesh::removeTriangle(Triangle *t, int i) {
  Edge *ea = t->getEdge();
  Edge *eb = ea->getNext();
  Edge *ec = eb->getNext();
  Vertex *a = ea->getStartVertex();
  Vertex *b = eb->getStartVertex();
  Vertex *c = ec->getStartVertex();
  // remove these elements from master lists
  edges[i].erase(std::make_pair(a,b));
  edges[i].erase(std::make_pair(b,c));
  edges[i].erase(std::make_pair(c,a));
  triangles[i].erase(t->getID());
  // clean up memory
  delete ea;
  delete eb;
  delete ec;
  delete t;
}


// Helper function for accessing data in the hash table
Edge* Mesh::getMeshEdge(Vertex *a, Vertex *b, int index) const {
  edgeshashtype::const_iterator iter = edges[index].find(std::make_pair(a,b));
  if (iter == edges[index].end()) return NULL;
  return iter->second;
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

  char prevline[200] = "";
  char line[200] = "";
  char token[100] = "";
  char token2[100] = "";
  char atoken[100] = "";
  char btoken[100] = "";
  char ctoken[100] = "";
  float x,y,z;
  int a,b,c,d,e;

  int index = 0;
  int vert_count = 0;
  int vert_index = 1;

  assert(vertices.size() == 0);

  vertices.push_back(std::vector<Vertex*>());


  //in order for the rendering of multiple objects with different colors to work
  //obj file must use groups to denote each object. group must be defined after vertices

  while (fgets(line, 200, objfile)) {
    int token_count = sscanf (line, "%s\n",token);
    sscanf (prevline, "%s\n", token2);
    if (token_count == -1) continue;
    a = b = c = d = e = -1;

    if (!strcmp(token,"usemtl") ||
	!strcmp(token,"g")) {
      vert_index = 1;
      index++;
      vertices.push_back( std::vector<Vertex*>() );
      edges.push_back(edgeshashtype());
      triangles.push_back(triangleshashtype());
    } else if (!strcmp(token,"v")) {
      vert_count++;
      sscanf (line, "%s %f %f %f\n",token,&x,&y,&z);
      addVertex(glm::vec3(x,y,z), index);
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
      assert (a >= 0 && a < totalVertices());
      assert (b >= 0 && b < totalVertices());
      assert (c >= 0 && c < totalVertices());
      addTriangle(getVertex(a),getVertex(b),getVertex(c), index-1);
    } else if (!strcmp(token,"vt")) {
    } else if (!strcmp(token,"vn")) {
    } else if (token[0] == '#') {
    } else {
      printf ("LINE: '%s'",line);
    }

    //change prevline to current line
    strcpy(prevline, line);
  }

  ComputeGouraudNormals();

  std::cout << "loaded " << numTriangles() << " triangles " << std::endl;
}

// =======================================================================
// this function outputs scene into a very simple .obj file
// =======================================================================

void Mesh::OutputFile() {
  printf("WRITING SCENE TO FILE\n");

  std::string output_file = args->path + "/output.obj";

  FILE *objfile = fopen(output_file.c_str(),"w");
  if (objfile == NULL) {
    std::cout << "ERROR! CANNOT OPEN '" << output_file << "'\n";
    return;
  }

  //loop through all the vertices and write all vertices of one object
  //after finishing the vertices of an object, create a group
  //then loop through the indices of the faces of this object and print them

  int indexStart = 0;  //index of the starting face of each object

  for (unsigned int i=0; i<vertices.size(); i++) {
    unsigned int j;

    //writing the vertices in this object to the file
    for (j=0; j<vertices[i].size(); j++) {
      glm::vec3 vPos = vertices[i][j]->getPos();
      fprintf(objfile, "v %.6f %.6f %.6f\n", vPos.x, vPos.y, vPos.z);
    }

    //create a group
    //need to also check that this vector of verices isn't empty
    if (vertices[i].size() > 0) {
      fprintf(objfile, "g partition%d\n", i);
    }

    //writing the faces in this object to the file
    for (j=indexStart; j<triangles.size(); j++) {
      Triangle *t = triangles[j];
      if (t->getObjectIndex() == (int)i) {
        //this face is part of this object
        //need to add 1 to index number because for vertices it starts at 1 not 0
        fprintf(objfile, "f %d %d %d\n", (*t)[0]->getIndex()+1, (*t)[1]->getIndex()+1, (*t)[2]->getIndex()+1);
      } else {
        indexStart = j;
        break;
      }
    }
  }
}

// =======================================================================

// compute the gouraud normals of all vertices of the mesh and store at each vertex
void Mesh::ComputeGouraudNormals() {
  int i;
  // clear the normals
  for (i = 0; i < totalVertices(); i++) {
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
  for (i = 0; i < totalVertices(); i++) {
    getVertex(i)->normalizeGouraudNormal();
  }
}

// =================================================================

// function chop(index, normal, offest)
// 	Mesh m = meshes[index]
// 	for each triangle t in m
// 		get the distance of each vertex of t to the plane
// 		if each vertex distance is >= 0
// 			add triangle to right submesh (meshes[index+1])
// 		else if each vertex distance is <= 0
// 			add triangle to left submesh (meshes[index+2])
// 		else
// 			vector Vab, Vbc, Vca
// 			vector anew = a + norm(Vab) * castRay(norm(Vab), normal, offset)
// 			vector bnew = b + norm(Vbc) * castRay(norm(Vbc), normal, offset)
// 			vector cnew = c + norm(Vca) * castRay(norm(Vca), normal, offset)
// 			vector p1, p2
// 			if a is left and b and c are right
// 				p1 = anew
// 				p2 = cnew
// 				meshes[index+1] += triangle(b,p2,p1)
// 				meshes[index+1] += triangle(c,p2,b)
// 				meshes[index+2] += triangle(a,p1,p2)
// 			if b is left and a and c are right
// 				p1 = anew
// 				p2 = bnew
// 				meshes[index+1] += triangle(a,p1,p2)
// 				meshes[index+1] += triangle(c,a,p2)
// 				meshes[index+2] += triangle(b,p2,p1)
// 			if c is left and a and b are right
// 				p1 = bnew
// 				p2 = cnew
// 				meshes[index+1] += triangle(b,p1,p2)
// 				meshes[index+1] += triangle(a,b,p2)
// 				meshes[index+2] += triangle(c,p2,p1)
// 			if a is right and b and c are left
// 				p1 = anew
// 				p2 = cnew
// 				meshes[index+1] += triangle(a,p1,p2)
// 				meshes[index+2] += triangle(b,p2,p1)
// 				meshes[index+2] += tirangle(c,p2,b)
// 			if b is right and a and c are left
// 				p1 = bnew
// 				p2 = anew
// 				meshes[index+1] += triangle

void Mesh::chop(unsigned int index, const glm::vec3& normal, float offset) {
  // for (unsigned int i = 0; i < vertices[i].size(); i += 3) {
  //   glm::vec3
  // }
}
