#include "mesh-utils.hpp"

// We will use "Tiny OBJ Loader" to read and process '.obj" files
#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobj/tiny_obj_loader.h>

#include "../material/mtl-material-registry.hpp"

#include <iostream>
#include <vector>
#include <unordered_map>

our::Mesh *our::mesh_utils::loadOBJ(const std::string &filename)
{
    std::cout << "Loading OBJ file: " << filename << std::endl;
    // The data that we will use to initialize our mesh
    std::vector<our::Vertex> vertices;
    std::vector<GLuint> elements;

    // Since the OBJ can have duplicated vertices, we make them unique using this map
    // The key is the vertex, the value is its index in the vector "vertices".
    // That index will be used to populate the "elements" vector.
    std::unordered_map<our::Vertex, GLuint> vertex_map;

    // The data loaded by Tiny OBJ Loader
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    // Extract directory for MTL file loading
    std::string mtl_basedir = filename.substr(0, filename.find_last_of("/\\") + 1);

    // Add mtl_basedir parameter
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str(), mtl_basedir.c_str()))
    {
        std::cerr << "Failed to load obj file \"" << filename << "\" due to error: " << err << std::endl;
        return nullptr;
    }
    if (!warn.empty())
    {
        std::cout << "WARN while loading obj file \"" << filename << "\": " << warn << std::endl;
    }

    // ✓ ADD: Register materials to global registry
    for (const auto &mat : materials)
    {
        std::cout << "\n=== Registering MTL Material: " << mat.name << " ===" << std::endl;
        MTLMaterialProperties props;
        props.name = mat.name;
        props.ambient = glm::vec3(mat.ambient[0], mat.ambient[1], mat.ambient[2]);
        props.diffuse = glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
        props.specular = glm::vec3(mat.specular[0], mat.specular[1], mat.specular[2]);
        props.shininess = mat.shininess;
        props.dissolve = mat.dissolve;
        props.illuminationModel = mat.illum;
        props.diffuseTexture = mat.diffuse_texname;
        props.specularTexture = mat.specular_texname;
        props.normalTexture = mat.bump_texname;

        std::cout << "  Ambient: (" << props.ambient.x << ", " << props.ambient.y << ", " << props.ambient.z << ")" << std::endl;
        std::cout << "  Diffuse: (" << props.diffuse.x << ", " << props.diffuse.y << ", " << props.diffuse.z << ")" << std::endl;
        std::cout << "  Specular: (" << props.specular.x << ", " << props.specular.y << ", " << props.specular.z << ")" << std::endl;
        std::cout << "  Shininess: " << props.shininess << std::endl;
        std::cout << "  Dissolve: " << props.dissolve << std::endl;
        std::cout << "  Illumination Model: " << props.illuminationModel << std::endl;
        std::cout << "  Diffuse Texture: " << (props.diffuseTexture.empty() ? "(none)" : props.diffuseTexture) << std::endl;
        std::cout << "  Specular Texture: " << (props.specularTexture.empty() ? "(none)" : props.specularTexture) << std::endl;
        std::cout << "  Normal Texture: " << (props.normalTexture.empty() ? "(none)" : props.normalTexture) << std::endl;

        MTLMaterialRegistry::getInstance().registerMaterial(mat.name, props);
        std::cout << "=== Registration Complete ===\n"
                  << std::endl;
    }

    bool hasNormals = !attrib.normals.empty();
    bool hasTexCoords = !attrib.texcoords.empty();
    bool hasColors = !attrib.colors.empty();

    for (const auto &shape : shapes)
    {
        for (const auto &index : shape.mesh.indices)
        {
            Vertex vertex = {};

            // Read the data for a vertex from the "attrib" object
            vertex.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]};

            if (hasNormals && index.normal_index >= 0)
            {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]};
            }
            else
            {
                vertex.normal = {0.0f, 1.0f, 0.0f}; // Default up
            }

            if (hasTexCoords && index.texcoord_index >= 0)
            {
                vertex.tex_coord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]};
            }
            else
            {
                vertex.tex_coord = {0.0f, 0.0f};
            }

            if (hasColors)
            {
                vertex.color = {
                    (unsigned char)(attrib.colors[3 * index.vertex_index + 0] * 255),
                    (unsigned char)(attrib.colors[3 * index.vertex_index + 1] * 255),
                    (unsigned char)(attrib.colors[3 * index.vertex_index + 2] * 255),
                    255};
            }
            else
            {
                vertex.color = {255, 255, 255, 255}; // Default white
            }

            // See if we already stored a similar vertex
            auto it = vertex_map.find(vertex);
            if (it == vertex_map.end())
            {
                // if no, add it to the vertices and record its index
                auto new_vertex_index = static_cast<GLuint>(vertices.size());
                vertex_map[vertex] = new_vertex_index;
                elements.push_back(new_vertex_index);
                vertices.push_back(vertex);
            }
            else
            {
                // if yes, just add its index in the elements vector
                elements.push_back(it->second);
            }
        }
    }

    return new our::Mesh(vertices, elements);
}

// Create a sphere (the vertex order in the triangles are CCW from the outside)
// Segments define the number of divisions on the both the latitude and the longitude
our::Mesh *our::mesh_utils::sphere(const glm::ivec2 &segments)
{
    std::vector<our::Vertex> vertices;
    std::vector<GLuint> elements;

    // We populate the sphere vertices by looping over its longitude and latitude
    for (int lat = 0; lat <= segments.y; lat++)
    {
        float v = (float)lat / segments.y;
        float pitch = v * glm::pi<float>() - glm::half_pi<float>();
        float cos = glm::cos(pitch), sin = glm::sin(pitch);
        for (int lng = 0; lng <= segments.x; lng++)
        {
            float u = (float)lng / segments.x;
            float yaw = u * glm::two_pi<float>();
            glm::vec3 normal = {cos * glm::cos(yaw), sin, cos * glm::sin(yaw)};
            glm::vec3 position = normal;
            glm::vec2 tex_coords = glm::vec2(u, v);
            our::Color color = our::Color(255, 255, 255, 255);
            vertices.push_back({position, color, tex_coords, normal});
        }
    }

    for (int lat = 1; lat <= segments.y; lat++)
    {
        int start = lat * (segments.x + 1);
        for (int lng = 1; lng <= segments.x; lng++)
        {
            int prev_lng = lng - 1;
            elements.push_back(lng + start);
            elements.push_back(lng + start - segments.x - 1);
            elements.push_back(prev_lng + start - segments.x - 1);
            elements.push_back(prev_lng + start - segments.x - 1);
            elements.push_back(prev_lng + start);
            elements.push_back(lng + start);
        }
    }

    return new our::Mesh(vertices, elements);
}

our::Mesh *our::mesh_utils::loadOBJWithMaterials(const std::string &filename)
{
    std::vector<our::Vertex> vertices;
    std::vector<GLuint> elements;
    std::unordered_map<our::Vertex, GLuint> vertex_map;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    // Extract directory from filename for loading MTL files
    std::string mtl_basedir = filename.substr(0, filename.find_last_of("/\\") + 1);

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str(), mtl_basedir.c_str()))
    {
        std::cerr << "Failed to load obj file \"" << filename << "\" due to error: " << err << std::endl;
        return nullptr;
    }
    if (!warn.empty())
    {
        std::cout << "WARN while loading obj file \"" << filename << "\": " << warn << std::endl;
    }

    // Register materials to global registry
    for (const auto &mat : materials)
    {
        std::cout << "\n=== Registering MTL Material (submesh): " << mat.name << " ===" << std::endl;
        MTLMaterialProperties props;
        props.name = mat.name;
        props.ambient = glm::vec3(mat.ambient[0], mat.ambient[1], mat.ambient[2]);
        props.diffuse = glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
        props.specular = glm::vec3(mat.specular[0], mat.specular[1], mat.specular[2]);
        props.shininess = mat.shininess;
        props.dissolve = mat.dissolve;
        props.illuminationModel = mat.illum;
        props.diffuseTexture = mat.diffuse_texname;
        props.specularTexture = mat.specular_texname;
        props.normalTexture = mat.bump_texname;

        std::cout << "  Ambient: (" << props.ambient.x << ", " << props.ambient.y << ", " << props.ambient.z << ")" << std::endl;
        std::cout << "  Diffuse: (" << props.diffuse.x << ", " << props.diffuse.y << ", " << props.diffuse.z << ")" << std::endl;
        std::cout << "  Specular: (" << props.specular.x << ", " << props.specular.y << ", " << props.specular.z << ")" << std::endl;
        std::cout << "  Shininess: " << props.shininess << std::endl;
        std::cout << "  Dissolve: " << props.dissolve << std::endl;
        std::cout << "  Illumination Model: " << props.illuminationModel << std::endl;
        std::cout << "  Diffuse Texture: " << (props.diffuseTexture.empty() ? "(none)" : props.diffuseTexture) << std::endl;
        std::cout << "  Specular Texture: " << (props.specularTexture.empty() ? "(none)" : props.specularTexture) << std::endl;
        std::cout << "  Normal Texture: " << (props.normalTexture.empty() ? "(none)" : props.normalTexture) << std::endl;

        MTLMaterialRegistry::getInstance().registerMaterial(mat.name, props);
        std::cout << "=== Registration Complete ===\n"
                  << std::endl;
    }

    std::vector<our::Submesh> submeshes;

    // Process each shape
    for (const auto &shape : shapes)
    {
        // Group faces by material
        std::map<int, std::vector<tinyobj::index_t>> material_faces;

        for (size_t f = 0; f < shape.mesh.material_ids.size(); f++)
        {
            int mat_id = shape.mesh.material_ids[f];
            // Get indices for this face (assuming triangles)
            for (size_t v = 0; v < 3; v++)
            {
                material_faces[mat_id].push_back(shape.mesh.indices[3 * f + v]);
            }
        }

        // Create submesh for each material
        for (const auto &[mat_id, indices] : material_faces)
        {
            GLsizei startElement = elements.size();

            for (const auto &index : indices)
            {
                Vertex vertex = {};

                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]};

                if (index.normal_index >= 0)
                {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]};
                }

                if (index.texcoord_index >= 0)
                {
                    vertex.tex_coord = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1] // Don't flip V coordinate
                    };
                }

                vertex.color = {255, 255, 255, 255};

                auto it = vertex_map.find(vertex);
                if (it == vertex_map.end())
                {
                    auto new_vertex_index = static_cast<GLuint>(vertices.size());
                    vertex_map[vertex] = new_vertex_index;
                    elements.push_back(new_vertex_index);
                    vertices.push_back(vertex);
                }
                else
                {
                    elements.push_back(it->second);
                }
            }

            // Create submesh entry
            our::Submesh submesh;
            submesh.elementOffset = startElement;
            submesh.elementCount = elements.size() - startElement;
            submesh.materialName = (mat_id >= 0 && mat_id < materials.size())
                                       ? materials[mat_id].name
                                       : "default";
            std::cout << "Created submesh with material: " << submesh.materialName
                      << " (elements: " << submesh.elementCount << ")" << std::endl;
            submeshes.push_back(submesh);
        }
    }

    auto mesh = new our::Mesh(vertices, elements);
    mesh->setSubmeshes(submeshes);
    return mesh;
}