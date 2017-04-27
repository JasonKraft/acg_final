#include <cstdlib>
#include <queue>
#include "glCanvas.h"
#include "argparser.h"
#include "camera.h"

#if _WIN32
#include <Winsock2.h>
#else
#include <sys/time.h>
#endif

#include "bsptree.h"
// #include "mesh.h"
#include "utils.h"

// ========================================================
// static variables of GLCanvas class

ArgParser* GLCanvas::args = NULL;
Camera* GLCanvas::camera = NULL;
// Mesh* GLCanvas::mesh = NULL;
BSPTree* GLCanvas::tree = NULL;
BoundingBox GLCanvas::bbox;
GLFWwindow* GLCanvas::window = NULL;

// mouse position
int GLCanvas::mouseX = 0;
int GLCanvas::mouseY = 0;
// which mouse button
bool GLCanvas::leftMousePressed = false;
bool GLCanvas::middleMousePressed = false;
bool GLCanvas::rightMousePressed = false;
// current state of modifier keys
bool GLCanvas::shiftKeyPressed = false;
bool GLCanvas::controlKeyPressed = false;
bool GLCanvas::altKeyPressed = false;
bool GLCanvas::superKeyPressed = false;

GLuint GLCanvas::render_VAO;

GLuint GLCanvas::ViewMatrixID;
GLuint GLCanvas::ModelMatrixID;
GLuint GLCanvas::LightID;
GLuint GLCanvas::MatrixID;
GLuint GLCanvas::programID;
GLuint GLCanvas::colormodeID;
GLuint GLCanvas::wireframeID;


// ========================================================
// Initialize all appropriate OpenGL variables, set
// callback functions, and start the main event loop.
// This function will not return but can be terminated
// by calling 'exit(0)'
// ========================================================

void GLCanvas::initialize(ArgParser *_args) {

  args = _args;
  // mesh = new Mesh(args);
  // mesh->Load();
  // bbox.Set(mesh->getBoundingBox());
  printf("hi\n");
  tree = new BSPTree(args);
  tree->Load();
  bbox.Set(tree->getBoundingBox());

  glfwSetErrorCallback(error_callback);

  // Initialize GLFW
  if( !glfwInit() ) {
    std::cerr << "ERROR: Failed to initialize GLFW" << std::endl;
    exit(1);
  }

  // We will ask it to specifically open an OpenGL 3.2 context
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create a GLFW window
  window = glfwCreateWindow(args->width,args->height, "OpenGL viewer", NULL, NULL);
  if (!window) {
    std::cerr << "ERROR: Failed to open GLFW window" << std::endl;
    glfwTerminate();
    exit(1);
  }
  glfwMakeContextCurrent(window);
  HandleGLError("in glcanvas first");

  // Initialize GLEW
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    std::cerr << "ERROR: Failed to initialize GLEW" << std::endl;
    glfwTerminate();
    exit(1);
  }

  // there seems to be a "GL_INVALID_ENUM" error in glewInit that is a
  // know issue, but can safely be ignored
  HandleGLError("after glewInit()",true);

  std::cout << "-------------------------------------------------------" << std::endl;
  std::cout << "OpenGL Version: " << (char*)glGetString(GL_VERSION) << '\n';
  std::cout << "-------------------------------------------------------" << std::endl;

  // Initialize callback functions
  glfwSetCursorPosCallback(GLCanvas::window,GLCanvas::mousemotionCB);
  glfwSetMouseButtonCallback(GLCanvas::window,GLCanvas::mousebuttonCB);
  glfwSetKeyCallback(GLCanvas::window,GLCanvas::keyboardCB);

  programID = LoadShaders( args->path+"/"+args->shader_filename+".vs",
                           args->path+"/"+args->shader_filename+".fs");

printf("here 1\n");
  GLCanvas::initializeVBOs();
  GLCanvas::setupVBOs();
  printf("here 2\n");

  // ===========================
  // initial placement of camera
  // look at an object scaled & positioned to just fit in the box (-1,-1,-1)->(1,1,1)
  glm::vec3 camera_position = glm::vec3(1,3,8);
  glm::vec3 point_of_interest = glm::vec3(0,0,0);
  glm::vec3 up = glm::vec3(0,1,0);
  float angle = 20.0;
  camera = new PerspectiveCamera(camera_position, point_of_interest, up, angle);
  camera->glPlaceCamera();

  HandleGLError("finished glcanvas initialize");
}



void GLCanvas::initializeVBOs(){
  HandleGLError("enter initilizeVBOs()");
  glGenVertexArrays(1, &render_VAO);
  glBindVertexArray(render_VAO);
  GLCanvas::MatrixID = glGetUniformLocation(GLCanvas::programID, "MVP");
  GLCanvas::LightID = glGetUniformLocation(GLCanvas::programID, "LightPosition_worldspace");
  GLCanvas::ViewMatrixID = glGetUniformLocation(GLCanvas::programID, "V");
  GLCanvas::ModelMatrixID = glGetUniformLocation(GLCanvas::programID, "M");
  GLCanvas::colormodeID = glGetUniformLocation(GLCanvas::programID, "colormode");
  GLCanvas::wireframeID = glGetUniformLocation(GLCanvas::programID, "wireframe");

  // mesh->initializeVBOs();
  printf("test 1\n");
  tree->initializeVBOs();
  printf("test 2\n");
  HandleGLError("leaving initilizeVBOs()");
}


void GLCanvas::setupVBOs(){
  HandleGLError("enter GLCanvas::setupVBOs()");
  // assert (mesh != NULL);
  // mesh->setupVBOs();
  assert (tree != NULL);
  tree->setupVBOs();
  HandleGLError("leaving GLCanvas::setupVBOs()");
}


void GLCanvas::drawVBOs(const glm::mat4 &ProjectionMatrix,const glm::mat4 &ViewMatrix,const glm::mat4 &ModelMatrix){
  HandleGLError("enter GlCanvas::drawVBOs()");

  // prepare data to send to the shaders
  glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
  glm::vec3 lightPos = tree->LightPosition();
  glm::vec4 lightPos2 = glm::vec4(lightPos.x,lightPos.y,lightPos.z,1);
  lightPos2 = ModelMatrix * lightPos2;
  glUniform3f(GLCanvas::LightID, lightPos2.x, lightPos2.y, lightPos2.z);

  glUniformMatrix4fv(GLCanvas::MatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniformMatrix4fv(GLCanvas::ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
  glUniformMatrix4fv(GLCanvas::ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

  // mesh->drawVBOs();
  tree->drawVBOs();
  HandleGLError("leaving GlCanvas::drawVBOs()");
}


void GLCanvas::cleanupVBOs(){
  bbox.cleanupVBOs();
  // mesh->cleanupVBOs();
  tree->cleanupVBOs();
}


// ========================================================
// Callback function for mouse click or release
// ========================================================

void GLCanvas::mousebuttonCB(GLFWwindow *window, int which_button, int action, int mods) {
  // store the current state of the mouse buttons
  if (which_button == GLFW_MOUSE_BUTTON_1) {
    if (action == GLFW_PRESS) {
      leftMousePressed = true;
    } else {
      assert (action == GLFW_RELEASE);
      leftMousePressed = false;
    }
  } else if (which_button == GLFW_MOUSE_BUTTON_2) {
    if (action == GLFW_PRESS) {
      rightMousePressed = true;
    } else {
      assert (action == GLFW_RELEASE);
      rightMousePressed = false;
    }
  } else if (which_button == GLFW_MOUSE_BUTTON_3) {
    if (action == GLFW_PRESS) {
      middleMousePressed = true;
    } else {
      assert (action == GLFW_RELEASE);
      middleMousePressed = false;
    }
  }
}

// ========================================================
// Callback function for mouse drag
// ========================================================

void GLCanvas::mousemotionCB(GLFWwindow *window, double x, double y) {

  // camera controls that work well for a 3 button mouse
  if (!shiftKeyPressed && !controlKeyPressed && !altKeyPressed) {
    if (leftMousePressed) {
      camera->rotateCamera(mouseX-x,mouseY-y);
    } else if (middleMousePressed)  {
      camera->truckCamera(mouseX-x, y-mouseY);
    } else if (rightMousePressed) {
      camera->dollyCamera(mouseY-y);
    }
  }

  if (leftMousePressed || middleMousePressed || rightMousePressed) {
    if (shiftKeyPressed) {
      camera->zoomCamera(mouseY-y);
    }
    // allow reasonable control for a non-3 button mouse
    if (controlKeyPressed) {
      camera->truckCamera(mouseX-x, y-mouseY);
    }
    if (altKeyPressed) {
      camera->dollyCamera(y-mouseY);
    }
  }
  mouseX = x;
  mouseY = y;
}

// ========================================================
// Callback function for keyboard events
// ========================================================

void GLCanvas::keyboardCB(GLFWwindow* window, int key, int scancode, int action, int mods) {
  // store the modifier keys
  shiftKeyPressed = (GLFW_MOD_SHIFT & mods);
  controlKeyPressed = (GLFW_MOD_CONTROL & mods);
  altKeyPressed = (GLFW_MOD_ALT & mods);
  superKeyPressed = (GLFW_MOD_SUPER & mods);
  // non modifier key actions

  if (key == GLFW_KEY_ESCAPE || key == 'q' || key == 'Q') {
    glfwSetWindowShouldClose(GLCanvas::window, GL_TRUE);
  }

  int tempWire;

  // other normal ascii keys...
  if ( (action == GLFW_PRESS || action == GLFW_REPEAT) && key < 256) {
    switch (key) {
    case 'b': case 'B':
      args->bounding_box = !args->bounding_box;
      // mesh->setupVBOs();
      tree->setupVBOs();
      break;
    case 'g': case 'G':
      args->geometry = !args->geometry;
      // mesh->setupVBOs();
      tree->setupVBOs();
      break;
    case 'n': case 'N':
      printf("ADDING GOURAUD SHADING\n");
      args->gouraud_normals = !args->gouraud_normals;
      // mesh->setupVBOs();
      tree->setupVBOs();
      break;
    case 'w': case 'W':
      printf("CREATING WIREFRAME\n");
      args->wireframe = !args->wireframe;
      // mesh->setupVBOs();
      tree->setupVBOs();
      break;
    case 'c': case 'C':
      //cut the mesh into partitions for printing
      printf("CUTTING THE MESH INTO PARTITIONS FOR PRINTING\n");
      tempWire = args->wireframe;
      args->wireframe = 0;
      tree->setupVBOs();
      tree->chop(glm::vec3(1.0f, 0.0f, 0.0f), 0.0);
      tree->initializeVBOs();
      tree->setupVBOs();
      tree->leftChild->chop(glm::vec3(0.0f, 1.0f, 0.0f), 0.1511);
      tree->initializeVBOs();
      tree->setupVBOs();
      args->wireframe = tempWire;
      tree->setupVBOs();
      break;
    case 'o': case 'O':
      //output scene into obj file
      // printf("WRITING SCENE TO FILE\n");
      // mesh->OutputFile();
      break;
    case 'l' : case 'L':
      //LoadCompileLinkShaders();
      // mesh->setupVBOs();
      tree->setupVBOs();
      break;
    case 'q':  case 'Q':
      // quit
      glfwSetWindowShouldClose(GLCanvas::window, GL_TRUE);
      break;
    default:
      std::cout << "UNKNOWN KEYBOARD INPUT  '" << (char)key << "'" << std::endl;
    }
    setupVBOs();
  }
}


// ========================================================
// Load the vertex & fragment shaders
// ========================================================

GLuint LoadShaders(const std::string &vertex_file_path,const std::string &fragment_file_path){

  std::cout << "load shaders" << std::endl;

  // Create the shaders
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  // Read the Vertex Shader code from the file
  std::string VertexShaderCode;
  std::ifstream VertexShaderStream(vertex_file_path.c_str(), std::ios::in);
  if (VertexShaderStream.is_open()){
    std::string Line = "";
    while(getline(VertexShaderStream, Line))
      VertexShaderCode += "\n" + Line;
    VertexShaderStream.close();
  } else {
    std::cerr << "ERROR: cannot open " << vertex_file_path << std::endl;
    exit(0);
  }
  // Read the Fragment Shader code from the file
  std::string FragmentShaderCode;
  std::ifstream FragmentShaderStream(fragment_file_path.c_str(), std::ios::in);
  if(FragmentShaderStream.is_open()){
    std::string Line = "";
    while(getline(FragmentShaderStream, Line))
      FragmentShaderCode += "\n" + Line;
    FragmentShaderStream.close();
  } else {
    std::cerr << "ERROR: cannot open " << vertex_file_path << std::endl;
    exit(0);
  }

  GLint Result = GL_FALSE;

  // Compile Vertex Shader
  std::cout << "Compiling shader : " << vertex_file_path << std::endl;
  char const * VertexSourcePointer = VertexShaderCode.c_str();
  glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
  glCompileShader(VertexShaderID);

  // Check Vertex Shader
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  if (Result != GL_TRUE) {
    GLsizei log_length = 0;
    GLchar message[1024];
    glGetShaderInfoLog(VertexShaderID, 1024, &log_length, message);
    std::cout << "VERTEX ERROR " << message << std::endl;
    exit(1);
  }

  // Compile Fragment Shader
  std::cout << "Compiling shader : " << fragment_file_path << std::endl;
  char const * FragmentSourcePointer = FragmentShaderCode.c_str();
  glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
  glCompileShader(FragmentShaderID);

  // Check Fragment Shader
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  if (Result != GL_TRUE) {
    GLsizei log_length = 0;
    GLchar message[1024];
    glGetShaderInfoLog(FragmentShaderID, 1024, &log_length, message);
    std::cout << "FRAGMENT ERROR " << message << std::endl;
    exit(1);
  }

  // Link the program
  std::cout << "Linking shader program" << std::endl;
  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  glLinkProgram(ProgramID);

  // Check the program
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  if (Result != GL_TRUE) {
    GLsizei log_length = 0;
    GLchar message[1024];
    glGetShaderInfoLog(ProgramID, 1024, &log_length, message);
    std::cout << "SHADER PROGRAM ERROR " << message << std::endl;
    exit(1);
  }

  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

  printf("asdf\n");
  return ProgramID;
}

// ========================================================
// Functions related to error handling
// ========================================================

void GLCanvas::error_callback(int error, const char* description) {
  std::cerr << "ERROR CALLBACK: " << description << std::endl;
}

std::string WhichGLError(GLenum &error) {
  switch (error) {
  case GL_NO_ERROR:
    return "NO_ERROR";
  case GL_INVALID_ENUM:
    return "GL_INVALID_ENUM";
  case GL_INVALID_VALUE:
    return "GL_INVALID_VALUE";
  case GL_INVALID_OPERATION:
    return "GL_INVALID_OPERATION";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "GL_INVALID_FRAMEBUFFER_OPERATION";
  case GL_OUT_OF_MEMORY:
    return "GL_OUT_OF_MEMORY";
  case GL_STACK_UNDERFLOW:
    return "GL_STACK_UNDERFLOW";
  case GL_STACK_OVERFLOW:
    return "GL_STACK_OVERFLOW";
  default:
    return "OTHER GL ERROR";
  }
}

int HandleGLError(const std::string &message, bool ignore) {
  GLenum error;
  int i = 0;
  while ((error = glGetError()) != GL_NO_ERROR) {
    if (!ignore) {
      if (message != "") {
	std::cerr << "[" << message << "] ";
      }
      std::cerr << "GL ERROR(" << i << ") " << WhichGLError(error) << std::endl;
    }
    i++;
  }
  if (i == 0) return 1;
  return 0;
}

// ========================================================
// ========================================================

bool allAtGoal(const std::vector<BSPTree*> &currentBSPs) {
  for (unsigned int i = 0; i < currentBSPs.size(); ++i) {
    if (currentBSPs[i] == NULL ||
        !currentBSPs[i]->fitsInVolume(GLCanvas::args->printing_width,
                                      GLCanvas::args->printing_height,
                                      GLCanvas::args->printing_length)) {
      return false;
    }
  }

  return true;
}

BSPTree* GLCanvas::beamSearch(BSPTree* tree) {
  std::vector<BSPTree*> currentBSPs(args->beam_width, NULL);
  currentBSPs[0] = tree;

  // while not all trees in currentBSPs can fit into the working volume
  while(!allAtGoal(currentBSPs)) {
    // this priority queue will store all possible new cuts in order of grade
    // (lowest grade is first)
    std::priority_queue<BSPTree*, std::vector<BSPTree*>, BSPTreeGreaterThan> newBSPs;
    for (unsigned int i = 0; i < currentBSPs.size(); ++i) {
      // skip over any null tree pointers or ones that already reach our goal
      if (currentBSPs[i] == NULL ||
        currentBSPs[i]->fitsInVolume(args->printing_width,
                                       args->printing_height,
                                       args->printing_length)) {
        continue;
      }

      // remove this BSP tree from currentBSPs
      BSPTree* t = new BSPTree(*(currentBSPs[i]));
      delete currentBSPs[i];
      currentBSPs[i] = NULL;
      BSPTree* p = NULL;
      t->largestPart(args->printing_width, args->printing_height, args->printing_length, p);
      std::list<BSPTree*> resultSet = evalCuts(t,p);
      for (std::list<BSPTree*>::iterator iter = resultSet.begin(); iter != resultSet.end(); ++iter) {
        newBSPs.push(*iter);
      }
    }

    // replace any null trees in currentBSPs with the best graded ones from newBSPs
    for (unsigned int i = 0; i < currentBSPs.size(); ++i) {
      if (currentBSPs[i] == NULL) {
        currentBSPs[i] = newBSPs.top();
        newBSPs.pop();
      }
    }

    // discard our remaining BSPs in newBSPs
    while(!newBSPs.empty()) {
      BSPTree *temp = newBSPs.top();
      newBSPs.pop();
      delete temp;
    }
  }

  // return the BSP tree with the lowest grade
  // (lower is better)
  int bestTreeIndex = 0;
  for (unsigned int i = 1; i < currentBSPs.size(); ++i) {
    if (currentBSPs[i]->getGrade() < currentBSPs[bestTreeIndex]->getGrade()) {
      bestTreeIndex = i;
    }
  }

  return currentBSPs[bestTreeIndex];
}

std::list<BSPTree*> GLCanvas::evalCuts(BSPTree* t, BSPTree* p) {
  std::vector<glm::vec3> uniNorms;

  // #pragma omp parallel for
  for (unsigned int i = 0; i < uniNorms.size(); ++i) {
    // Genate all cuts with given norm uniNorms[i] and offsets
    float curOffset, maxOffset;
    p->getMinMaxOffsetsAlongNorm(uniNorms[i], curOffset, maxOffset);

    typedef std::list< std::pair<float, std::pair<BSPTree*, BSPTree*> > > CutList;

    CutList potentialCuts;
    while(curOffset <= maxOffset) {
      p->chop(uniNorms[i], curOffset);
      potentialCuts.push_back(std::make_pair(curOffset, std::make_pair(p->leftChild, p->rightChild)));
      p->leftChild = NULL;
      p->rightChild = NULL;
      curOffset += args->offset_increment;
    }

    // Evaluate objective functions

  }
}
