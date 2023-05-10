#include <iostream>
#include <memory>

#include "window.hpp"
#include "mygraphics.hpp"
#include "model.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

// specs
unsigned int W_WIDTH = 1366;
unsigned int W_HEIGHT = 768;

// camera
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

bool firstMouse = true;
float yaw   = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch =  0.0f;
float lastX =  800.0f / 2.0;
float lastY =  600.0 / 2.0;
float fov   =  45.0f;


/* OBJECTS */
std::string objects_path[] = {
  "assets/blue_car.fbx", 
  "assets/white_car.fbx"
};
std::vector<Model> objects;
std::vector<glm::vec3> objects_pos = {
    glm::vec3(0.0f, 1.0f, 5.0f),
    glm::vec3(0.0f, 0.0f, 0.0f)
};
GLuint VAO[2];
GLuint VBO[2];
GLuint EBO[2];

GLuint inter_VAO;
GLuint inter_VBO;
GLuint inter_EBO;

float speed = 0.01f;

struct bbox {
  glm::vec3 min_extents;
  glm::vec3 max_extents;
};
std::vector<bbox> bounding_boxes;

/* Callbacks */
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

/* Broad Phase */
struct bbox get_mesh_min_max_coord(Mesh mesh){
  glm::vec3 minExtents(FLT_MAX);
  glm::vec3 maxExtents(-FLT_MAX);

  // Iterate over all vertices in the mesh
  for (unsigned int i = 0; i < mesh.vertices.size(); i++){
    // Update minimum and maximum extents for each dimension
    if (mesh.vertices[i].Position.x < minExtents.x){
      minExtents.x = mesh.vertices[i].Position.x;
    }
    if (mesh.vertices[i].Position.y < minExtents.y){
      minExtents.y = mesh.vertices[i].Position.y;
    }
    if (mesh.vertices[i].Position.z < minExtents.z){
      minExtents.z = mesh.vertices[i].Position.z;
    }

    if (mesh.vertices[i].Position.x > maxExtents.x){
      maxExtents.x = mesh.vertices[i].Position.x;
    }
    if (mesh.vertices[i].Position.y > maxExtents.y){
      maxExtents.y = mesh.vertices[i].Position.y;
    }
    if (mesh.vertices[i].Position.z > maxExtents.z){
      maxExtents.z = mesh.vertices[i].Position.z;
    }
  }
  return {minExtents, maxExtents};
}

void draw_bbox(glm::vec3 minExtents, glm::vec3 maxExtents, GLuint *VAO, GLuint *VBO, GLuint *EBO) {
  // create bounding box vertices
  std::vector<glm::vec3> bb_vertices = {
    glm::vec3(minExtents.x, minExtents.y, minExtents.z),
    glm::vec3(maxExtents.x, minExtents.y, minExtents.z),
    glm::vec3(maxExtents.x, maxExtents.y, minExtents.z),
    glm::vec3(minExtents.x, maxExtents.y, minExtents.z),
    glm::vec3(minExtents.x, minExtents.y, maxExtents.z),
    glm::vec3(maxExtents.x, minExtents.y, maxExtents.z),
    glm::vec3(maxExtents.x, maxExtents.y, maxExtents.z),
    glm::vec3(minExtents.x, maxExtents.y, maxExtents.z)
  };

  unsigned int indices[] = {
    0, 1, 1, 2, 2, 3, 3, 0,
    3, 2, 2, 6, 6, 7, 7, 3,
    7, 6, 6, 5, 5, 4, 4, 7,
    4, 0, 0, 3, 1, 5, 2, 6
  };

  // Generate and bind VAO, VBO, and EBO
  glGenVertexArrays(1, VAO);
  glGenBuffers(1, VBO);
  glGenBuffers(1, EBO);
  glBindVertexArray(*VAO);
  glBindBuffer(GL_ARRAY_BUFFER, *VBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);

  // Pass vertex and index data to VBO and EBO
  glBufferData(GL_ARRAY_BUFFER, bb_vertices.size() * sizeof(glm::vec3), &bb_vertices[0], GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);

  // Set vertex attribute pointers
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

}

bool checkCollision(struct bbox bbox1, struct bbox bbox2) {
  bool x_overlap = bbox1.max_extents.x >= bbox2.min_extents.x && bbox2.max_extents.x >= bbox1.min_extents.x;
  bool y_overlap = bbox1.max_extents.y >= bbox2.min_extents.y && bbox2.max_extents.y >= bbox1.min_extents.y;
  bool z_overlap = bbox1.max_extents.z >= bbox2.min_extents.z && bbox2.max_extents.z >= bbox1.min_extents.z;
  return x_overlap && y_overlap && z_overlap;
}

struct bbox getIntersectionExtents(struct bbox bbox1, struct bbox bbox2) {
  struct bbox intersectionExtents;
  // Calculate the minimum and maximum extents of the intersection in the x and z axes
  intersectionExtents.min_extents.x = std::max(bbox1.min_extents.x, bbox2.min_extents.x);
  intersectionExtents.min_extents.z = std::max(bbox1.min_extents.z, bbox2.min_extents.z);

  intersectionExtents.max_extents.x = std::min(bbox1.max_extents.x, bbox2.max_extents.x);
  intersectionExtents.max_extents.z = std::min(bbox1.max_extents.z, bbox2.max_extents.z);

  // Calculate the union of the extents in the y axis
  intersectionExtents.min_extents.y = std::min(bbox1.min_extents.y, bbox2.min_extents.y);
  intersectionExtents.max_extents.y = std::max(bbox1.max_extents.y, bbox2.max_extents.y);


  return intersectionExtents;
}

/*
std::unique_ptr<Mesh> get_mesh_within_bbox(const Mesh& mesh, const bbox& boundingBox)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // Iterate over the mesh vertices
    for (unsigned int i = 0; i < mesh.vertices.size(); i++)
    {
        const Vertex& vertex = mesh.vertices[i];

        // Check if the vertex position is inside the bounding box
        if (vertex.Position.x >= boundingBox.min_extents.x && vertex.Position.x <= boundingBox.max_extents.x &&
            vertex.Position.y >= boundingBox.min_extents.y && vertex.Position.y <= boundingBox.max_extents.y &&
            vertex.Position.z >= boundingBox.min_extents.z && vertex.Position.z <= boundingBox.max_extents.z)
        {
            // Add the vertex to the new mesh
            vertices.push_back(vertex);
            // Add the index of the vertex in the original mesh
            //indices.push_back(i);
            indices.push_back(vertices.size()-1);
        }
    }

    // Iterate over the mesh textures
    for (unsigned int i = 0; i < mesh.textures.size(); i++)
    {
        const Texture& texture = mesh.textures[i];

        // Add the texture to the new mesh
        textures.push_back(texture);
    }

    // Check if any vertices were found within the bounding box
    if (vertices.empty())
    {
        return nullptr; // Return nullptr or use optional return value to indicate no mesh
    }

    // Create and return the new mesh object
    return std::make_unique<Mesh>(vertices, indices, textures);
}*/
std::unique_ptr<Mesh> get_mesh_within_bbox(const Mesh& mesh, const bbox& boundingBox)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // Track the mapping between original vertex indices and new vertex indices
    std::unordered_map<unsigned int, unsigned int> vertexIndexMap;

    // Iterate over the mesh indices
    for (unsigned int i = 0; i < mesh.indices.size(); i++)
    {
        unsigned int originalIndex = mesh.indices[i];
        const Vertex& vertex = mesh.vertices[originalIndex];

        // Check if the vertex position is inside the bounding box
        if (vertex.Position.x >= boundingBox.min_extents.x && vertex.Position.x <= boundingBox.max_extents.x &&
            vertex.Position.y >= boundingBox.min_extents.y && vertex.Position.y <= boundingBox.max_extents.y &&
            vertex.Position.z >= boundingBox.min_extents.z && vertex.Position.z <= boundingBox.max_extents.z)
        {
            // Check if the vertex already exists in the subset mesh
            auto it = vertexIndexMap.find(originalIndex);
            if (it != vertexIndexMap.end())
            {
                // Vertex already added, use the existing index
                indices.push_back(it->second);
            }
            else
            {
                // Add the vertex to the new mesh
                unsigned int newIndex = static_cast<unsigned int>(vertices.size());
                vertexIndexMap[originalIndex] = newIndex;
                vertices.push_back(vertex);
                indices.push_back(newIndex);
            }
        }
    }

    // Check if any vertices were found within the bounding box
    if (vertices.empty())
    {
        return nullptr; // Return nullptr or use optional return value to indicate no mesh
    }

    // Create and return the new mesh object
    return std::make_unique<Mesh>(vertices, indices, textures);
}




/* Narrow Phase */
std::vector<glm::vec3> calculateMeshAxes(const Mesh &mesh){
  std::vector<glm::vec3> axes;
  for (unsigned int i = 0; i < mesh.indices.size(); i += 3){
    const Vertex &v1 = mesh.vertices[mesh.indices[i]];
    //const Vertex &v2 = mesh.vertices[mesh.indices[i + 1]];
    //const Vertex &v3 = mesh.vertices[mesh.indices[i + 2]];
    const Vertex &v2 = mesh.vertices[mesh.indices[i + 1 >= mesh.vertices.size() ? 0 : i + 1]];
    const Vertex &v3 = mesh.vertices[mesh.indices[i + 2 >= mesh.vertices.size() ? 0 : i + 2]];

    glm::vec3 faceNormal = glm::normalize(glm::cross(v2.Position - v1.Position, v3.Position - v1.Position));
    axes.push_back(faceNormal);
  }
  return axes;
    /*
    std::cout << v1.Position.x << " " << v1.Position.y << " " << v1.Position.z << std::endl;
    std::cout << v2.Position.x << " " << v2.Position.y << " " << v2.Position.z << std::endl;
    std::cout << v3.Position.x << " " << v3.Position.y << " " << v3.Position.z << std::endl;
    */
}

struct Projection {
    float min;
    float max;
};

Projection projectMeshOnAxis(const Mesh& mesh, const glm::vec3& axis) {
    float min = glm::dot(axis, mesh.vertices[0].Position);
    float max = min;

    for (int i=1; i<mesh.vertices.size(); i++) {
        // Project vertex onto axis
        float projection = glm::dot(axis, mesh.vertices[i].Position);

        // Update min/max values
        if (projection < min) {
            min = projection;
        }
        if (projection > max) {
            max = projection;
        }
    }

    return { min, max };
}

bool overlap(Projection p1, Projection p2) {
    return !(p1.max < p2.min || p2.max < p1.min);
}

bool sat(Mesh mesh1, Mesh mesh2){
  vector<glm::vec3> axes1 = calculateMeshAxes(mesh1);
  vector<glm::vec3> axes2 = calculateMeshAxes(mesh2);

  // loop over the axes1
  for (int i = 0; i < axes1.size(); i++) {
    glm::vec3 axis = axes1[i];
    // project both shapes onto the axis
    Projection p1 = projectMeshOnAxis(mesh1, axis);
    Projection p2 = projectMeshOnAxis(mesh2, axis);
    // do the projections overlap?
    if (!overlap(p1, p2)){
      // then we can guarantee that the shapes do not overlap
      return false;
    }
  }

  // loop over the axes2
  for (int i = 0; i < axes2.size(); i++) {
    glm::vec3 axis = axes2[i];
    // project both shapes onto the axis
    Projection p1 = projectMeshOnAxis(mesh1, axis);
    Projection p2 = projectMeshOnAxis(mesh2, axis);
    // do the projections overlap?
    if (!overlap(p1, p2)){
      // then we can guarantee that the shapes do not overlap
      return false;
    }
  }


  // if we get here then we know that every axis had overlap on it
  // so we can guarantee an intersection
  return true;

}

void translateMesh(Mesh &mesh, const glm::vec3 translationVector){
  for (int i = 0; i < mesh.vertices.size(); i++){
    mesh.vertices[i].Position = mesh.vertices[i].Position + translationVector;
  }
}

/* Main code */
void Window::refresh(void){
  // frame time logic
  float currentFrame = static_cast<float>(glfwGetTime());
  deltaTime = currentFrame - lastFrame;
  lastFrame = currentFrame;
  // input
  processInput(this->window);

  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // model view projection
  glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
  glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
  glm::mat4 projection = glm::perspective(glm::radians(fov), (float)W_WIDTH / (float)W_HEIGHT, 0.1f, 1000.0f);

  // pass model view projection at each obj and bbox
  for(long unsigned int i=0; i<objects_pos.size(); i++){
    glm::mat4 aux_model = glm::translate(model, objects_pos[i]);
    // activate objs shader
    this->shaders[0]->use();
    this->shaders[0]->setMat4("model", aux_model);
    this->shaders[0]->setMat4("view", view);
    this->shaders[0]->setMat4("projection", projection);
    // draw objs
    objects[i].Draw(*this->shaders[0]);

    // update bboxes
    for(unsigned long int i=0; i<objects.size(); i++){
      struct bbox aux = get_mesh_min_max_coord(objects[i].meshes[0]);
      draw_bbox(aux.min_extents, aux.max_extents, &VAO[i], &VBO[i], &EBO[i]);
      // add pos
      aux.max_extents += objects_pos[i];
      aux.min_extents += objects_pos[i];
      // save obj bbox
      bounding_boxes[i] = aux;
    }

    // activate bbox shader
    this->shaders[1]->use();
    this->shaders[1]->setMat4("model", aux_model);
    this->shaders[1]->setMat4("view", view);
    this->shaders[1]->setMat4("projection", projection);
    // draw bboxes
    glBindVertexArray(VAO[i]);
    glLineWidth(3.0f);
    glDrawElements(GL_LINES, 36, GL_UNSIGNED_INT, 0);
  }

  // Check collisions (broad phase)
  if(checkCollision(bounding_boxes[0], bounding_boxes[1])){
    // bbox shader
    this->shaders[1]->setVec4("color", glm::vec4(1.0f, 1.0f, 0.0f, 0.5f));

    // calculate intersection extents
    struct bbox inter_bbox = getIntersectionExtents(bounding_boxes[0], bounding_boxes[1]);
    draw_bbox(inter_bbox.min_extents, inter_bbox.max_extents, &inter_VAO, &inter_VBO, &inter_EBO);
    // intersection bbox shader
    this->shaders[2]->use();
    this->shaders[2]->setMat4("model", model);
    this->shaders[2]->setMat4("view", view);
    this->shaders[2]->setMat4("projection", projection);
    // draw intersection bbox
    glBindVertexArray(inter_VAO);
    glLineWidth(8.0f);
    glDrawElements(GL_LINES, 36, GL_UNSIGNED_INT, 0);
    // change color to red
    this->shaders[2]->setVec4("color", glm::vec4(1.0f, 0.0f, 0.0f, 0.5f));


    // Narrow Pahse
    // get the part of mesh1 and mesh2 that is inside inter_bbox
    Mesh translated_mesh1(objects[0].meshes[0]);
    Mesh translated_mesh2(objects[1].meshes[0]);
    translateMesh(translated_mesh1, objects_pos[0]);
    translateMesh(translated_mesh2, objects_pos[1]);
    std::unique_ptr<Mesh> mesh1_inter = get_mesh_within_bbox(translated_mesh1, inter_bbox);
    std::unique_ptr<Mesh> mesh2_inter = get_mesh_within_bbox(translated_mesh2, inter_bbox);

    if(!(mesh1_inter == nullptr || mesh2_inter == nullptr)){
      Mesh& mesh1_inter_obj = *mesh1_inter;
      Mesh& mesh2_inter_obj = *mesh2_inter;
      // translate the new meshes
      //translateMesh(mesh1_inter_obj, objects_pos[0]);
      //translateMesh(mesh2_inter_obj, objects_pos[1]);
      //if(sat(mesh1_inter_obj, mesh2_inter_obj)){

        /*
        glm::mat4 aux_model = glm::translate(model, objects_pos[0]);
        // activate objs shader
        this->shaders[0]->use();
        this->shaders[0]->setMat4("model", aux_model);
        this->shaders[0]->setMat4("view", view);
        this->shaders[0]->setMat4("projection", projection);
        mesh1_inter_obj.Draw(*this->shaders[0]);

        aux_model = glm::translate(model, objects_pos[1]);
        // activate objs shader
        this->shaders[0]->use();
        this->shaders[0]->setMat4("model", aux_model);
        this->shaders[0]->setMat4("view", view);
        this->shaders[0]->setMat4("projection", projection);
        mesh2_inter_obj.Draw(*this->shaders[0]);
        */
      
      //std::cout << mesh1_inter_obj.vertices.size() << " " << mesh1_inter_obj.indices.size() << std::endl;

      if(sat(mesh1_inter_obj, mesh2_inter_obj)){
        std::cout << "Collision detected" << std::endl;
        this->shaders[0]->use();
        this->shaders[0]->setBool("collide", true);
      }
      else{
        this->shaders[0]->use();
        this->shaders[0]->setBool("collide", false);
      }
    }
  }
  else{
    this->shaders[1]->setVec4("color", glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));
  }
  /*

  // Copy object mesh and translate vertices
  Mesh translated_mesh(objects[0].meshes[0]);
  translateMesh(translated_mesh, objects_pos[0]);

  // 

  // Check collisions (narrow phase)
  this->shaders[0]->use();
  if(sat(translated_mesh, objects[1].meshes[0])){
    std::cout << "Collision detected" << std::endl;
    this->shaders[0]->setBool("collide", true);
  }
  else{
    this->shaders[0]->setBool("collide", false);
  }

  */


  // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
  // -------------------------------------------------------------------------------
  glfwSwapBuffers(this->get_window());
  glfwPollEvents();
}

int main(void){
  Window window;

  // initialize window
  window.initialize(W_WIDTH, W_HEIGHT, "");
  // callbacks
  glfwSetFramebufferSizeCallback(window.get_window(), framebuffer_size_callback);
  glfwSetCursorPosCallback(window.get_window(), mouse_callback);
  glfwSetScrollCallback(window.get_window(), scroll_callback);
  // tell GLFW to capture our mouse
  glfwSetInputMode(window.get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  //stbi_set_flip_vertically_on_load(true);
  glEnable(GL_DEPTH_TEST);

  // build and compile shaders
  window.add_shader(new Shader("shader.vs", "shader.fs"));
  window.add_shader(new Shader("bbox.vs", "bbox.fs"));
  window.add_shader(new Shader("bbox.vs", "bbox.fs"));

  // load objs
  for(unsigned long int i=0; i<sizeof(objects_path) / sizeof(objects_path[0]); i++) {
    objects.push_back(Model(objects_path[i]));
  }

  // create bbox VAO, VBO and EBO for each obj
  for(unsigned long int i=0; i<objects.size(); i++){
    struct bbox aux = get_mesh_min_max_coord(objects[i].meshes[0]);
    draw_bbox(aux.min_extents, aux.max_extents, &VAO[i], &VBO[i], &EBO[i]);
    // add pos
    aux.max_extents += objects_pos[i];
    aux.min_extents += objects_pos[i];
    // save obj bbox
    bounding_boxes.push_back(aux);
  }

  // run
  window.run();

  return 0;
}


// Inputs and Callbacks
void processInput(GLFWwindow *window){
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
    glfwSetWindowShouldClose(window, true);
  }

  float cameraSpeed = static_cast<float>(2.5 * deltaTime);
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    cameraPos += cameraSpeed * cameraFront;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    cameraPos -= cameraSpeed * cameraFront;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  // options
  if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS){
    objects_pos[0].z += speed;
    bounding_boxes[0].max_extents += glm::vec3(0.0f, 0.0f, speed);
    bounding_boxes[0].min_extents += glm::vec3(0.0f, 0.0f, speed);
  }
  if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS){
    objects_pos[0].z -= speed;
    bounding_boxes[0].max_extents -= glm::vec3(0.0f, 0.0f, speed);
    bounding_boxes[0].min_extents -= glm::vec3(0.0f, 0.0f, speed);
  }
  if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS){
    objects_pos[0].y += speed;
    bounding_boxes[0].max_extents += glm::vec3(0.0f, speed, 0.0f);
    bounding_boxes[0].min_extents += glm::vec3(0.0f, speed, 0.0f);
  }
  if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS){
    objects_pos[0].y -= speed;
    bounding_boxes[0].max_extents -= glm::vec3(0.0f, speed, 0.0f);
    bounding_boxes[0].min_extents -= glm::vec3(0.0f, speed, 0.0f);
  }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn){
  float xpos = static_cast<float>(xposIn);
  float ypos = static_cast<float>(yposIn);

  if (firstMouse){
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
  lastX = xpos;
  lastY = ypos;

  float sensitivity = 0.1f; // change this value to your liking
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  yaw += xoffset;
  pitch += yoffset;

  // make sure that when pitch is out of bounds, screen doesn't get flipped
  if (pitch > 89.0f)
    pitch = 89.0f;
  if (pitch < -89.0f)
    pitch = -89.0f;

  glm::vec3 front;
  front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  front.y = sin(glm::radians(pitch));
  front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
  fov -= (float)yoffset;
  if (fov < 1.0f)
    fov = 1.0f;
  if (fov > 45.0f)
    fov = 45.0f;
}