#ifndef MESH_H
#define MESH_H

#include "glCanvas.h"
#include <vector>
#include <string>
#include "hash.h"
#include "boundingbox.h"
#include "vbo_structs.h"

class ArgParser;
class Vertex;
class Edge;
class Triangle;

// ======================================================================
// ======================================================================

class Mesh {

public:

  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Mesh(ArgParser *_args) {
    args = _args;
    glm::vec4 colorsArray[10] = {
        glm::vec4(0.886f, 0.313f, 0.345f,1),
        glm::vec4(0.760f, 0.419f, 0.592f,1),
        glm::vec4(0.4f, 0.647f, 0.882f,1),
        glm::vec4(0.415f, 0.823f, 0.745f,1),
        glm::vec4(0.439f, 0.360f, 0.8f,1),
        glm::vec4(0.898f, 0.513f, 0.321f,1),
        glm::vec4(0.921f, 0.886f, 0.439f,1),
        glm::vec4(0.537f, 0.858f, 0.341f,1),
        glm::vec4(0.345f, 0.447f, 0.835f,1),
        glm::vec4(0.603f, 0.835f, 0.345f,1)
    };
    colors.insert(colors.end(), &colorsArray[0], &colorsArray[10]);
    total_vertices = 0;

  }
  ~Mesh();
  void Load();
  void ComputeGouraudNormals();

  void initializeVBOs();
  void setupVBOs();
  void drawVBOs();
  void cleanupVBOs();

  int numObjects() const { return vertices.size(); }

  // ========
  // VERTICES
  int totalVertices() const { return total_vertices; }
  int numVertices(int i) const { return vertices[i].size(); }
  Vertex* addVertex(const glm::vec3 &pos, int index);
  // look up vertex by index from original .obj file
  Vertex* getVertex(int i) const {
    assert (i >= 0 && i < totalVertices());
    Vertex *v;
    int count = 0;
    for (int j=0; j<numObjects(); j++) {
      if (count + numVertices(j) > i) {
        v = vertices[j][i - count];
        break;
      }
      count += numVertices(j);
    }
    assert (v != NULL);
    return v; }
  Vertex* getVertex(int i, int j) const {
    assert (i >= 0 && i < totalVertices());
    Vertex *v = vertices[i][j];
    assert (v != NULL);
    return v; }
  int getVertexObjectIndex(int i) {
    assert (i >= 0 && i < totalVertices());
    int count = 0;
    for (int j=0; j<numObjects(); j++) {
      if (count + numVertices(j) > i) {
        return j;
      }
      count += numVertices(j);
    }
    return -1;
  }

  // =====
  // EDGES
  int numEdges() const { return edges.size(); }
  // this efficiently looks for an edge with the given vertices, using a hash table
  Edge* getMeshEdge(Vertex *a, Vertex *b) const;

  // =========
  // TRIANGLES
  int numTriangles() const { return triangles.size(); }
  void addTriangle(Vertex *a, Vertex *b, Vertex *c);
  void removeTriangle(Triangle *t);

  // ===============
  // OTHER ACCESSORS
  const BoundingBox& getBoundingBox() const { return bbox; }
  glm::vec3 LightPosition() const;

private:

  // HELPER FUNCTIONS FOR PAINT
  // void SetupLight(const glm::vec3 &light_position);
  void SetupMesh();

  // void DrawLight();
  void DrawMesh();

  // ==============
  // REPRESENTATION
  ArgParser *args;
  std::vector<std::vector<Vertex*> > vertices;  //each vector of vertices reps an object
  edgeshashtype edges;
  triangleshashtype triangles;
  BoundingBox bbox;
  std::vector<glm::vec4> colors;  //pre-defined colors for different objects in mesh
  int total_vertices;

  // VBOs
  GLuint mesh_tri_verts_VBO;
  GLuint mesh_tri_indices_VBO;
  // GLuint light_vert_VBO;

  std::vector<VBOPosNormalColor> mesh_tri_verts;
  std::vector<VBOIndexedTri> mesh_tri_indices;
  // std::vector<VBOPosNormalColor> light_vert;

};

// ======================================================================
// ======================================================================

#endif
