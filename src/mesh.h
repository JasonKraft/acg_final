#ifndef MESH_H
#define MESH_H

#include "glCanvas.h"
#include <vector>
#include <string>
#include "hash.h"
#include "boundingbox.h"
#include "vbo_structs.h"
#include "argparser.h"

class ArgParser;
class Vertex;
class Edge;
class Triangle;

// ======================================================================
// ======================================================================

class Mesh {
  friend class BSPTree;
public:

  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Mesh() {
    args = NULL;
    float r = (float)(args->rand());
    float g = (float)(args->rand());
    float b = (float)(args->rand());
    meshColor = glm::vec4(r,g,b,1);
  }
  Mesh(ArgParser *_args) {
    args = _args;
    float r = (float)(args->rand());
    float g = (float)(args->rand());
    float b = (float)(args->rand());
    meshColor = glm::vec4(r,g,b,1);
  }
  Mesh(const Mesh &oldMesh);
  ~Mesh();

  // ASSIGNMENT OPERATOR
  Mesh& operator= (const Mesh& oldMesh);

  void clear();
  void Load();
  void ComputeGouraudNormals();

  void initializeVBOs();
  void setupVBOs();
  void drawVBOs();
  void cleanupVBOs();

  // ========
  // VERTICES
  int numVertices() const { return vertices.size(); }
  Vertex* addVertex(const glm::vec3 &pos);
  Vertex* addVertex(const glm::vec3 &pos, bool addToBoundingBox);
  // look up vertex by index from original .obj file
  Vertex* getVertex(int i) const {
    assert (i >= 0 && i < numVertices());
    Vertex *v = vertices[i];
    assert (v != NULL);
    return v; }

  // =====
  // EDGES
  int numEdges() const { return edges.size(); }
  // this efficiently looks for an edge with the given vertices, using a hash table
  Edge* getMeshEdge(Vertex *a, Vertex *b) const;

  // =========
  // TRIANGLES
  int numTriangles() const { return triangles.size(); }
  Triangle* addTriangle(Vertex *a, Vertex *b, Vertex *c);
  void removeTriangle(Triangle *t);

  // ===============
  // OTHER ACCESSORS
  const BoundingBox& getBoundingBox() const { return bbox; }
  glm::vec3 LightPosition() const;

  // function that helps with doing the wireframe stuff
  void TriVBOHelper( const glm::vec3 &pos_a,
                     const glm::vec3 &pos_b,
                     const glm::vec3 &pos_c,
                     const glm::vec3 &normal_a,
                     const glm::vec3 &normal_b,
                     const glm::vec3 &normal_c,
                     const glm::vec4 &color_ab,
                     const glm::vec4 &color_bc,
                     const glm::vec4 &color_ca,
                     const glm::vec4 &center_color);

  // function to output scene into obj file
  void OutputFile();

  // Determines whether mesh can fit inside of specified volume dimensions
  bool fitsInVolume(float width, float height, float length);

  // HELPER FUNCTIONS FOR OBJECTIVE FUNCTIONS
  int numPrintVolumes(float width, float height, float length);
  float getBBVolume() { return bbox.getVolume(); }
  glm::vec3 getBoundingBoxDims();

  // ==================================================
  // PARENT VERTEX RELATIONSHIPS (used for subdivision)
  // this creates a relationship between 3 vertices (2 parents, 1 child)
  void setParentsChild(Vertex *p1, Vertex *p2, Vertex *child);
  // this accessor will find a child vertex (if it exists) when given
  // two parent vertices
  Vertex* getChildVertex(Vertex *p1, Vertex *p2) const;

private:

  // HELPER FUNCTIONS FOR PAINT
  void SetupMesh();

  void DrawMesh();

  // ==============
  // REPRESENTATION
  ArgParser *args;
  std::vector<Vertex*> vertices;
  edgeshashtype edges;
  triangleshashtype triangles;
  BoundingBox bbox;
  glm::vec4 meshColor;  //pre-defined colors for different objects in mesh
  vphashtype vertex_parents;

  // VBOs
  GLuint mesh_tri_verts_VBO;
  GLuint mesh_tri_indices_VBO;

  std::vector<VBOPosNormalColor> mesh_tri_verts;
  std::vector<VBOIndexedTri> mesh_tri_indices;

};

// ======================================================================
// ======================================================================

#endif
