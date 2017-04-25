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
  friend class BSPTree;
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
  }
  ~Mesh();
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
  void addTriangle(Vertex *a, Vertex *b, Vertex *c);
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
  std::vector<glm::vec4> colors;  //pre-defined colors for different objects in mesh

  // VBOs
  GLuint mesh_tri_verts_VBO;
  GLuint mesh_tri_indices_VBO;

  std::vector<VBOPosNormalColor> mesh_tri_verts;
  std::vector<VBOIndexedTri> mesh_tri_indices;

};

// ======================================================================
// ======================================================================

#endif
