#include <string>
#include <string.h>
#include <vector>
#include <iostream>
#include <math.h>
#include <stdio.h>

int main(int argc, char const *argv[]) {

  FILE *objfile = fopen("deleted_octahedron.obj","r");
  if (objfile == NULL) {
    std::cout << "ERROR! CANNOT OPEN '" << "'\n";
    return 0;
  }

  char line[200] = "";
  char token[100] = "";
  float x,y,z;
  int numVertices = 0;

  // vector that stores all of the coordinates of the 129 vertices of the octahedron
  std::vector<std::vector<float> > vertices(129, std::vector<float>(3));


  while (fgets(line, 200, objfile)) {
    int token_count = sscanf (line, "%s\n",token);
    if (token_count == -1) continue;
    if (!strcmp(token,"v")) {
      sscanf (line, "%s %f %f %f\n",token,&x,&y,&z);

      // store the vertex coordinates
      vertices[numVertices][0] = x;
      vertices[numVertices][1] = y;
      vertices[numVertices][2] = z;

      numVertices++;
    }
  }


  // normalize the vertex positions
  for (unsigned int i=0; i<vertices.size(); i++) {
    x = vertices[i][0];
    y = vertices[i][1];
    z = vertices[i][2];
    float magnitude = sqrt(x*x + y*y + z*z);

    // replace old coordinates with normalized coordinates
    vertices[i][0] = x / magnitude;
    vertices[i][1] = y / magnitude;
    vertices[i][2] = z / magnitude;
  }

  // writing normalized coordinates into a vector into a text file that can be loaded
  // so can obtain the 129 normals needed for testing possible cuts
  FILE *outputfile = fopen("octahedron_normals.txt","w");
  if (objfile == NULL) {
    std::cout << "ERROR! CANNOT OPEN '" << "'\n";
    return 0;
  }

  fprintf(outputfile, "glm::vec3 normals[129] = { \n");
  for (unsigned int i=0; i<vertices.size(); i++) {
    fprintf(outputfile, "\t\t\tglm::vec3(%f, %f, %f)", vertices[i][0], vertices[i][1], vertices[i][2]);

    if (i < vertices.size() - 1) {
      fprintf(outputfile, ", \n");
    }
  }
  fprintf(outputfile, "\n};\n");
  fprintf(outputfile, "std::vector<glm::vec3> vec (normals, normals + sizeof(normals) / sizeof(normals[0]) );\n");



  return 0;
}
