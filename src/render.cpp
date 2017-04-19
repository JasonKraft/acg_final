#include <iostream>

#include "glCanvas.h"
#include "mesh.h"
#include "edge.h"
#include "vertex.h"
#include "triangle.h"
#include "argparser.h"
#include "utils.h"

glm::vec4 mesh_color(0.8,0.8,0.8,1);

glm::vec4 red(1.0,0,0,1);
glm::vec4 green(0,1,0,0.5);

float floor_factor = 0.75;

// =======================================================================
// =======================================================================


// the light position can be animated
glm::vec3 Mesh::LightPosition() const {
  glm::vec3 min = bbox.getMin();
  glm::vec3 max = bbox.getMax();
  glm::vec3 tmp;
  bbox.getCenter(tmp);
  tmp += glm::vec3(0,1.5*(max.y-min.y),0);
  return tmp;
}


void Mesh::initializeVBOs() {
  glGenBuffers(1,&mesh_tri_verts_VBO);
  glGenBuffers(1,&mesh_tri_indices_VBO);
  bbox.initializeVBOs();
}

void Mesh::cleanupVBOs() {
  glDeleteBuffers(1,&mesh_tri_verts_VBO);
  glDeleteBuffers(1,&mesh_tri_indices_VBO);
  bbox.cleanupVBOs();
}

// boundary edges are red, crease edges are yellow
glm::vec4 EdgeColor(Edge *e) {
  if (e->getOpposite() == NULL) {
    return glm::vec4(1,0,0,1);
  } else if (e->getCrease() > 0) {
    return glm::vec4(1,1,0,1);
  } else {
    return glm::vec4(0,0,0.0,1);
  }
}

// ================================================================================
// ================================================================================


void Mesh::TriVBOHelper( const glm::vec3 &pos_a,
                         const glm::vec3 &pos_b,
                         const glm::vec3 &pos_c,
                         const glm::vec3 &normal_a,
                         const glm::vec3 &normal_b,
                         const glm::vec3 &normal_c,
                         const glm::vec4 &color_ab,
                         const glm::vec4 &color_bc,
                         const glm::vec4 &color_ca,
                         const glm::vec4 &center_color) {

  /*
  // To create a wireframe rendering...
  // Each mesh triangle is actually rendered as 3 small triangles
  //           b
  //          /|\
  //         / | \
  //        /  |  \
  //       /   |   \
  //      /    |    \
  //     /    .'.    \
  //    /  .'     '.  \
  //   /.'           '.\
  //  a-----------------c
  //
  */

  // use simple averaging to find centroid & average normal
  glm::vec3 centroid = 1.0f / 3.0f * (pos_a + pos_b + pos_c);
  glm::vec3 normal = normal_a + normal_b + normal_c;
  normal = glm::normalize(normal);

  int i = mesh_tri_verts.size();

  if (args->wireframe) {
    // WIREFRAME

    // make the 3 small triangles
    mesh_tri_verts.push_back(VBOPosNormalColor(pos_a,normal_a,color_ab));
    mesh_tri_verts.push_back(VBOPosNormalColor(pos_b,normal_b,color_ab));
    mesh_tri_verts.push_back(VBOPosNormalColor(centroid,normal,center_color));

    mesh_tri_verts.push_back(VBOPosNormalColor(pos_b,normal_b,color_bc));
    mesh_tri_verts.push_back(VBOPosNormalColor(pos_c,normal_c,color_bc));
    mesh_tri_verts.push_back(VBOPosNormalColor(centroid,normal,center_color));

    mesh_tri_verts.push_back(VBOPosNormalColor(pos_c,normal_c,color_ca));
    mesh_tri_verts.push_back(VBOPosNormalColor(pos_a,normal_a,color_ca));
    mesh_tri_verts.push_back(VBOPosNormalColor(centroid,normal,center_color));

    // add all of the triangle vertices to the indices list
    for (int j = 0; j < 9; j+=3) {
      mesh_tri_indices.push_back(VBOIndexedTri(i+j,i+j+1,i+j+2));
    }
  } else {
    // NON WIREFRAME
    // Note: gouraud shading with the mini triangles looks bad... :(

    // don't make the 3 small triangles
    mesh_tri_verts.push_back(VBOPosNormalColor(pos_a,normal_a,center_color));
    mesh_tri_verts.push_back(VBOPosNormalColor(pos_b,normal_b,center_color));
    mesh_tri_verts.push_back(VBOPosNormalColor(pos_c,normal_c,center_color));

    // add all of the triangle vertices to the indices list
    mesh_tri_indices.push_back(VBOIndexedTri(i,i+1,i+2));
  }

}



void Mesh::SetupMesh() {
  for (triangleshashtype::iterator iter = triangles.begin();
       iter != triangles.end(); iter++) {
    Triangle *t = iter->second;
    glm::vec3 a = (*t)[0]->getPos();
    glm::vec3 b = (*t)[1]->getPos();
    glm::vec3 c = (*t)[2]->getPos();

    // determine edge colors (when wireframe is enabled)
    glm::vec4 edgecolor_ab = EdgeColor(t->getEdge());
    glm::vec4 edgecolor_bc = EdgeColor(t->getEdge()->getNext());
    glm::vec4 edgecolor_ca = EdgeColor(t->getEdge()->getNext()->getNext());

    //calculate normals
    glm::vec3 na = ComputeNormal(a,b,c);
    glm::vec3 nb = na;
    glm::vec3 nc = na;
    if (args->gouraud_normals) {
      na = (*t)[0]->getGouraudNormal();
      nb = (*t)[1]->getGouraudNormal();
      nc = (*t)[2]->getGouraudNormal();
    }
    glm::vec4 center_color = colors[(*t)[0]->getObjectIndex()];

    TriVBOHelper(a,b,c,
                 na,nb,nc,
                 edgecolor_ab,edgecolor_bc,edgecolor_ca, center_color);
  }

  glBindBuffer(GL_ARRAY_BUFFER,mesh_tri_verts_VBO);
  glBufferData(GL_ARRAY_BUFFER,
	       sizeof(VBOPosNormalColor) * mesh_tri_verts.size(),
	       &mesh_tri_verts[0],
	       GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mesh_tri_indices_VBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
	       sizeof(VBOIndexedTri) * mesh_tri_indices.size(),
	       &mesh_tri_indices[0], GL_STATIC_DRAW);

  num_mini_triangles = mesh_tri_verts.size();

}



// ================================================================================
// ================================================================================


void Mesh::DrawMesh() {
  HandleGLError("enter draw mesh");
  glBindBuffer(GL_ARRAY_BUFFER,mesh_tri_verts_VBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mesh_tri_indices_VBO);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor),(void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor),(void*)sizeof(glm::vec3) );
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT,GL_FALSE,sizeof(VBOPosNormalColor), (void*)(sizeof(glm::vec3)*2));
  glDrawElements(GL_TRIANGLES, mesh_tri_indices.size()*3,GL_UNSIGNED_INT, 0);
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
  HandleGLError("leaving draw mesh");
}


// ======================================================================================
// ======================================================================================

void Mesh::setupVBOs() {
  // delete all the old geometry
  mesh_tri_verts.clear();
  mesh_tri_indices.clear();

  // setup the new geometry
  SetupMesh();
  bbox.setupVBOs();
}

void Mesh::drawVBOs() {


  // mode 1: STANDARD PHONG LIGHTING (LIGHT ON)
  glUniform1i(GLCanvas::colormodeID, 1);

  glUniform1i(GLCanvas::wireframeID, args->wireframe);


  HandleGLError("enter draw vbos");

  DrawMesh();

  HandleGLError();
}

// =================================================================
