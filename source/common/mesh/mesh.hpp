#pragma once

#include <glad/gl.h>

#include <string>
#include <vector>

#include "vertex.hpp"

namespace our {

#define ATTRIB_LOC_POSITION 0
#define ATTRIB_LOC_COLOR 1
#define ATTRIB_LOC_TEXCOORD 2
#define ATTRIB_LOC_NORMAL 3

// Represents a range of elements that use the same material
struct Submesh {
    std::string materialName;  // Name of the material used by this submesh
    GLsizei elementCount;      // Number of elements in this submesh
    GLsizei elementOffset;     // Offset in the element buffer (in bytes)
};

class Mesh {
    // Here, we store the object names of the 3 main components of a mesh:
    // A vertex array object, A vertex buffer and an element buffer
    unsigned int VBO, EBO;
    unsigned int VAO;
    unsigned int instanceVBO = 0;     // Buffer for the instances only.
    size_t currentInstanceCount = 0;  // Track current instance buffer size
    bool instanceBufferInitialized =
        false;  // Track if buffer is already set up
    // We need to remember the number of elements that will be draw by
    // glDrawElements
    GLsizei elementCount;
    std::vector<Submesh>
        submeshes;  // List of submeshes, each with a different material
    glm::vec3 minBound =
        glm::vec3(0.0f);  // Will be used in map this is the minimum bound of
                          // the 3d box covering the map obj
    glm::vec3 maxBound = glm::vec3(0.0f);  // Same thing but max
   public:
    // The constructor takes two vectors:
    // - vertices which contain the vertex data.
    // - elements which contain the indices of the vertices out of which each
    // rectangle will be constructed. The mesh class does not keep a these data
    // on the RAM. Instead, it should create a vertex buffer to store the vertex
    // data on the VRAM, an element buffer to store the element data on the
    // VRAM, a vertex array object to define how to read the vertex & element
    // buffer during rendering
    Mesh(const std::vector<Vertex>& vertices,
         const std::vector<unsigned int>& elements) {
        elementCount = elements.size();
        // Getting the min and max bounds of the vert vector
        if (!vertices.empty()) {
            minBound = vertices[0].position;
            maxBound = vertices[0].position;
            for (const auto& vertex : vertices) {
                minBound = glm::min(minBound, vertex.position);
                maxBound = glm::max(maxBound, vertex.position);
            }
        }
        // Generate and bind VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // Generate and bind VBO
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                     vertices.data(), GL_STATIC_DRAW);

        // Generate and bind EBO
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     elements.size() * sizeof(unsigned int), elements.data(),
                     GL_STATIC_DRAW);

        // Define vertex attribute pointers
        glEnableVertexAttribArray(ATTRIB_LOC_POSITION);
        glVertexAttribPointer(ATTRIB_LOC_POSITION, 3, GL_FLOAT, GL_FALSE,
                              sizeof(Vertex),
                              (void*)offsetof(Vertex, position));

        // Color attribute
        glEnableVertexAttribArray(ATTRIB_LOC_COLOR);
        glVertexAttribPointer(ATTRIB_LOC_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE,
                              sizeof(Vertex), (void*)offsetof(Vertex, color));

        // Texture coordinate attribute
        glEnableVertexAttribArray(ATTRIB_LOC_TEXCOORD);
        glVertexAttribPointer(ATTRIB_LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE,
                              sizeof(Vertex),
                              (void*)offsetof(Vertex, tex_coord));

        // Normal attribute
        glEnableVertexAttribArray(ATTRIB_LOC_NORMAL);
        glVertexAttribPointer(ATTRIB_LOC_NORMAL, 3, GL_FLOAT, GL_FALSE,
                              sizeof(Vertex), (void*)offsetof(Vertex, normal));

        // Unbind VAO to prevent accidental modification
        glBindVertexArray(0);
    }

    // this function should render the mesh
    void draw() {
        // Bind the VAO and draw the elements
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, elementCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void setupInstancing(const std::vector<glm::mat4>& instanceMats) {
        // instance mats are the position rotation and scale of each instance
        // but in a mat format so we only do 1 draw call for the big amount of
        // objs, here used for the trees which will be created using a greyscale
        // image made on the map texture.

        // Skip if already initialized with the same data (static instances)
        if (instanceBufferInitialized &&
            currentInstanceCount == instanceMats.size()) {
            return;  // Buffer already set up, no need to re-upload
        }

        if (instanceVBO == 0) {
            glGenBuffers(1, &instanceVBO);
        }
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

        // Use GL_STATIC_DRAW for data that won't change
        glBufferData(GL_ARRAY_BUFFER, instanceMats.size() * sizeof(glm::mat4),
                     instanceMats.data(), GL_STATIC_DRAW);

        // Only set up vertex attributes if not already done
        if (!instanceBufferInitialized) {
            for (int i = 0; i < 4; i++) {
                glEnableVertexAttribArray(4 + i);
                glVertexAttribPointer(4 + i, 4, GL_FLOAT, GL_FALSE,
                                      sizeof(glm::mat4),
                                      (void*)(sizeof(glm::vec4) * i));
                glVertexAttribDivisor(4 + i, 1);
            }
        }

        currentInstanceCount = instanceMats.size();
        instanceBufferInitialized = true;
        glBindVertexArray(0);
    }

    // Update instance buffer with culled instances (dynamic, uses
    // GL_DYNAMIC_DRAW)
    void updateInstanceBuffer(const std::vector<glm::mat4>& instanceMats) {
        if (instanceMats.empty()) return;

        if (instanceVBO == 0) {
            glGenBuffers(1, &instanceVBO);
        }

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

        // Reallocate if size changed, otherwise just update data
        if (currentInstanceCount != instanceMats.size()) {
            glBufferData(GL_ARRAY_BUFFER,
                         instanceMats.size() * sizeof(glm::mat4),
                         instanceMats.data(), GL_DYNAMIC_DRAW);
            currentInstanceCount = instanceMats.size();
        } else {
            glBufferSubData(GL_ARRAY_BUFFER, 0,
                            instanceMats.size() * sizeof(glm::mat4),
                            instanceMats.data());
        }

        // Set up vertex attributes if not already done
        if (!instanceBufferInitialized) {
            for (int i = 0; i < 4; i++) {
                glEnableVertexAttribArray(4 + i);
                glVertexAttribPointer(4 + i, 4, GL_FLOAT, GL_FALSE,
                                      sizeof(glm::mat4),
                                      (void*)(sizeof(glm::vec4) * i));
                glVertexAttribDivisor(4 + i, 1);
            }
            instanceBufferInitialized = true;
        }

        glBindVertexArray(0);
    }

    // this function should delete the vertex & element buffers and the vertex
    // array object
    ~Mesh() {
        // Delete the vertex buffer, element buffer, and vertex array object
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        if (instanceVBO != 0) {
            glDeleteBuffers(1, &instanceVBO);
        }
        glDeleteVertexArrays(1, &VAO);
    }

    // Draw a specific submesh by index
    void drawSubmesh(size_t submeshIndex) const {
        if (submeshIndex >= submeshes.size()) return;

        // Bind the VAO before drawing
        glBindVertexArray(VAO);

        const auto& submesh = submeshes[submeshIndex];
        glDrawElements(GL_TRIANGLES, submesh.elementCount, GL_UNSIGNED_INT,
                       (void*)(submesh.elementOffset * sizeof(GLuint)));

        // Unbind VAO after drawing
        glBindVertexArray(0);
    }

    void drawInstanced(GLsizei instanceCount) {
        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, elementCount, GL_UNSIGNED_INT, 0,
                                instanceCount);
        glBindVertexArray(0);
    }

    // Get the number of submeshes
    size_t getSubmeshCount() const { return submeshes.size(); }

    // Get submesh by index
    const Submesh& getSubmesh(size_t index) const { return submeshes[index]; }

    // Get all submeshes
    const std::vector<Submesh>& getSubmeshes() const { return submeshes; }

    // Set submeshes (used when loading multi-material meshes)
    void setSubmeshes(const std::vector<Submesh>& subs) { submeshes = subs; }

    // Get VAO for manual drawing
    unsigned int getVAO() const { return VAO; }

    Mesh(Mesh const&) = delete;
    Mesh& operator=(Mesh const&) = delete;
};

}  // namespace our