#include "forward-renderer.hpp"

#include <GLFW/glfw3.h>

#include "../components/instanced-renderer.hpp"
#include "../components/player.hpp"
#include "../mesh/mesh-utils.hpp"
#include "../texture/texture-utils.hpp"
namespace our
{

    void ForwardRenderer::initialize(glm::ivec2 windowSize,
                                     const nlohmann::json &config)
    {
        // First, we store the window size for later use
        this->windowSize = windowSize;

        // Read fog configuration
        this->fogEnabled = config.value("fog_enabled", true);

        // Then we check if there is a sky texture in the configuration
        if (config.contains("sky"))
        {
            // First, we create a sphere which will be used to draw the sky
            this->skySphere = mesh_utils::sphere(glm::ivec2(16, 16));

            // We can draw the sky using dedicated sky shaders
            // which have Y-level fog for proper tree occlusion
            ShaderProgram *skyShader = new ShaderProgram();
            skyShader->attach("assets/shaders/sky.vert", GL_VERTEX_SHADER);
            skyShader->attach("assets/shaders/sky.frag", GL_FRAGMENT_SHADER);
            skyShader->link();

            // Then, we setup the pipeline state for rendering the sky
            PipelineState skyPipelineState{};

            // Enable depth testing with LESS_EQUAL function
            skyPipelineState.depthTesting.enabled = true;
            skyPipelineState.depthTesting.function = GL_LEQUAL;

            // Enable face culling and cull the front faces
            skyPipelineState.faceCulling.enabled = true;
            skyPipelineState.faceCulling.culledFace = GL_FRONT;

            // Load the sky texture (note that we don't need mipmaps since we want
            // to avoid any unnecessary blurring while rendering the sky)
            std::string skyTextureFile = config.value<std::string>("sky", "");
            Texture2D *skyTexture = texture_utils::loadImage(skyTextureFile, false);

            // Setup a sampler for the sky
            Sampler *skySampler = new Sampler();
            skySampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_WRAP_S, GL_REPEAT);
            skySampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Combine all the aforementioned objects (except the mesh) into a
            // material
            this->skyMaterial = new TexturedMaterial();
            this->skyMaterial->shader = skyShader;
            this->skyMaterial->texture = skyTexture;
            this->skyMaterial->sampler = skySampler;
            this->skyMaterial->pipelineState = skyPipelineState;
            this->skyMaterial->tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            this->skyMaterial->alphaThreshold = 1.0f;
            this->skyMaterial->transparent = false;
        }

        // Then we check if there is a postprocessing shader in the configuration
        if (config.contains("postprocess"))
        {
            glGenFramebuffers(1, &postprocessFrameBuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, postprocessFrameBuffer);

            // Hints: The color format can be (Red, Green, Blue and Alpha components
            // with 8 bits for each channel). The depth format can be (Depth
            // component with 24 bits).
            colorTarget = texture_utils::empty(GL_RGBA8, windowSize);
            depthTarget = texture_utils::empty(GL_DEPTH_COMPONENT32F, windowSize);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, colorTarget->getOpenGLName(), 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                   GL_TEXTURE_2D, depthTarget->getOpenGLName(), 0);

            // Unbind the framebuffer just to be safe
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Create a vertex array to use for drawing the texture
            glGenVertexArrays(1, &postProcessVertexArray);

            // Create a sampler to use for sampling the scene texture in the post
            // processing shader
            Sampler *postprocessSampler = new Sampler();
            postprocessSampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            postprocessSampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Create the post processing shader
            ShaderProgram *postprocessShader = new ShaderProgram();
            postprocessShader->attach("assets/shaders/fullscreen.vert",
                                      GL_VERTEX_SHADER);
            postprocessShader->attach(config.value<std::string>("postprocess", ""),
                                      GL_FRAGMENT_SHADER);
            postprocessShader->link();

            // Create a post processing material
            postprocessMaterial = new TexturedMaterial();
            postprocessMaterial->shader = postprocessShader;
            postprocessMaterial->texture = colorTarget;
            postprocessMaterial->sampler = postprocessSampler;
            // The default options are fine but we don't need to interact with the
            // depth buffer so it is more performant to disable the depth mask
            postprocessMaterial->pipelineState.depthMask = false;
        }
    }

    void ForwardRenderer::destroy()
    {
        // Delete all objects related to the sky
        if (skyMaterial)
        {
            delete skySphere;
            delete skyMaterial->shader;
            delete skyMaterial->texture;
            delete skyMaterial->sampler;
            delete skyMaterial;
        }
        // Delete all objects related to post processing
        if (postprocessMaterial)
        {
            glDeleteFramebuffers(1, &postprocessFrameBuffer);
            glDeleteVertexArrays(1, &postProcessVertexArray);
            delete colorTarget;
            delete depthTarget;
            delete postprocessMaterial->sampler;
            delete postprocessMaterial->shader;
            delete postprocessMaterial;
        }
    }

    void ForwardRenderer::render(World *world, float deltaTime)
    {
        // First of all, we search for a camera and for all the mesh renderers
        CameraComponent *camera = nullptr;
        PlayerComponent *playerComp = nullptr;
        opaqueCommands.clear();
        transparentCommands.clear();
        lightCommands.clear();
        std::vector<InstancedRendererComponent *> instancedRenderers;
        for (auto entity : world->getEntities())
        {
            // If we hadn't found a camera yet, we look for a camera in this entity
            if (!camera)
                camera = entity->getComponent<CameraComponent>();
            // Find player component for flashlight state
            if (!playerComp)
                playerComp = entity->getComponent<PlayerComponent>();
            if (auto instancedRenderer =
                    entity->getComponent<InstancedRendererComponent>();
                instancedRenderer)
            {
                instancedRenderers.push_back(instancedRenderer);
            }
            // Collect light components and update their flicker
            if (auto light = entity->getComponent<LightComponent>(); light)
            {
                // Skip flashlight if player has it turned off
                if (light->isFlashlight && playerComp && !playerComp->flashlightOn) {
                    continue;
                }
                light->updateFlicker(deltaTime);
                lightCommands.push_back(light);
            }
            // If this entity has a mesh renderer component
            if (auto meshRenderer = entity->getComponent<MeshRendererComponent>();
                meshRenderer)
            {
                glm::mat4 localToWorld =
                    meshRenderer->getOwner()->getLocalToWorldMatrix();
                glm::vec3 center = glm::vec3(localToWorld * glm::vec4(0, 0, 0, 1));

                // If mesh has submeshes, create a command for each submesh with its
                // material
                if (meshRenderer->mesh &&
                    meshRenderer->mesh->getSubmeshCount() > 0)
                {
                    for (size_t i = 0; i < meshRenderer->mesh->getSubmeshCount();
                         i++)
                    {
                        const auto &submesh = meshRenderer->mesh->getSubmeshes()[i];
                        RenderCommand command;
                        command.localToWorld = localToWorld;
                        command.center = center;
                        command.mesh = meshRenderer->mesh;
                        command.submeshIndex = i;
                        command.material = meshRenderer->getMaterialForSubmesh(
                            submesh.materialName);

                        if (command.material->transparent)
                        {
                            transparentCommands.push_back(command);
                        }
                        else
                        {
                            opaqueCommands.push_back(command);
                        }
                    }
                }
                else
                {
                    // No submeshes, use default material
                    RenderCommand command;
                    command.localToWorld = localToWorld;
                    command.center = center;
                    command.mesh = meshRenderer->mesh;
                    command.submeshIndex = -1;
                    command.material = meshRenderer->material;

                    if (command.material->transparent)
                    {
                        transparentCommands.push_back(command);
                    }
                    else
                    {
                        opaqueCommands.push_back(command);
                    }
                }
            }
        }
        // If there is no camera, we return (we cannot render without a camera)
        if (camera == nullptr)
            return;

        auto M = camera->getOwner()->getLocalToWorldMatrix();
        glm::vec3 eye = M * glm::vec4(0, 0, 0, 1);
        glm::vec3 center = M * glm::vec4(0, 0, -1, 1);
        glm::vec3 cameraForward = glm::normalize(center - eye);

        std::sort(transparentCommands.begin(), transparentCommands.end(),
                  [cameraForward](const RenderCommand &first,
                                  const RenderCommand &second)
                  {
                      return glm::dot(first.center, cameraForward) >
                             glm::dot(second.center, cameraForward);
                  });

        // Get the camera ViewProjection matrix and store it in VP
        glm::mat4 VP =
            camera->getProjectionMatrix(windowSize) * camera->getViewMatrix();

        // Extract frustum for culling
        frustum.extractFromVP(VP);

        // Set the OpenGL viewport using viewportStart and viewportSize
        glViewport(0, 0, windowSize.x, windowSize.y);

        // Set the clear color to black and the clear depth to 1
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepth(1.0f);

        // Set the color mask to true and the depth mask to true (to ensure the
        // glClear will affect the framebuffer)
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);

        // If there is a postprocess material, bind the framebuffer
        if (postprocessMaterial)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, postprocessFrameBuffer);
        }
        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // Clear the color and depth buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Don't forget to set the "transform" uniform to be equal the
        // model-view-projection matrix for each render command

        for (const auto &command : opaqueCommands)
        {
            // Setup the material
            command.material->setup();
            // Compute the model-view-projection matrix
            glm::mat4 M = command.localToWorld;
            glm::mat4 MVP = VP * M;
            // Set the "transform" uniform
            command.material->shader->set("transform", MVP);

            // If this is a lit material, set lighting uniforms
            if (dynamic_cast<LitMaterial *>(command.material))
            {
                command.material->shader->set("camera_position", eye);
                command.material->shader->set("light_count", (int)lightCommands.size());
                command.material->shader->set("fog_enabled", fogEnabled);
                command.material->shader->set("M", M);
                command.material->shader->set("M_IT", glm::transpose(glm::inverse(M)));

                // Set uniforms for each light
                for (size_t i = 0; i < lightCommands.size(); i++)
                {
                    LightComponent *light = lightCommands[i];
                    std::string prefix = "lights[" + std::to_string(i) + "].";

                    // Get light world position and direction from its entity transform
                    glm::mat4 lightMatrix = light->getOwner()->getLocalToWorldMatrix();
                    glm::vec3 lightPos = glm::vec3(lightMatrix * glm::vec4(0, 0, 0, 1));
                    glm::vec3 worldDirection = glm::normalize(glm::vec3(lightMatrix * glm::vec4(light->direction, 0.0f)));

                    command.material->shader->set(prefix + "type", (int)light->lightType);
                    command.material->shader->set(prefix + "position", lightPos);
                    command.material->shader->set(prefix + "direction", worldDirection);
                    command.material->shader->set(prefix + "color", light->getEffectiveColor());
                    command.material->shader->set(prefix + "attenuation", light->attenuation);
                    command.material->shader->set(prefix + "inner_cone_angle", light->inner_cone_angle);
                    command.material->shader->set(prefix + "outer_cone_angle", light->outer_cone_angle);
                }
            }

            // Draw the mesh
            if (command.submeshIndex >= 0)
            {
                command.mesh->drawSubmesh(command.submeshIndex);
            }
            else
            {
                command.mesh->draw();
            }
        }

        for (auto &instancedRenderer : instancedRenderers)
        {
            if (instancedRenderer->mesh && instancedRenderer->material &&
                !instancedRenderer->InstanceMats.empty())
            {
                // Perform culling if enabled and positions are available
                const std::vector<glm::mat4> *instanceMatrices =
                    &instancedRenderer->InstanceMats;
                size_t instanceCount = instancedRenderer->InstanceMats.size();

                if (!instancedRenderer->instancePositions.empty() &&
                    (instancedRenderer->enableDistanceCulling ||
                     instancedRenderer->enableFrustumCulling))
                {
                    instancedRenderer->updateVisibleInstances(eye, frustum);

                    if (instancedRenderer->visibleInstanceMats.empty())
                    {
                        continue; // Skip if no visible instances
                    }

                    instanceMatrices = &instancedRenderer->visibleInstanceMats;
                    instanceCount = instancedRenderer->visibleInstanceMats.size();

                    // Update the instance buffer with culled instances
                    instancedRenderer->mesh->updateInstanceBuffer(
                        *instanceMatrices);
                }
                else
                {
                    // No culling, use static buffer
                    instancedRenderer->mesh->setupInstancing(*instanceMatrices);
                }

                // Check if mesh has submeshes
                if (instancedRenderer->mesh->getSubmeshCount() > 0)
                {
                    // Render each submesh with its specific material
                    for (size_t i = 0;
                         i < instancedRenderer->mesh->getSubmeshCount(); i++)
                    {
                        const auto &submesh =
                            instancedRenderer->mesh->getSubmesh(i);
                        Material *submeshMaterial =
                            instancedRenderer->getMaterialForSubmesh(
                                submesh.materialName);

                        submeshMaterial->setup();
                        submeshMaterial->shader->set("VP", VP);

                        // Set lighting uniforms for instanced lit materials
                        submeshMaterial->shader->set("camera_position", eye);
                        submeshMaterial->shader->set("light_count", (int)lightCommands.size());
                        submeshMaterial->shader->set("fog_enabled", fogEnabled);
                        for (size_t li = 0; li < lightCommands.size(); li++)
                        {
                            LightComponent *light = lightCommands[li];
                            std::string prefix = "lights[" + std::to_string(li) + "].";
                            glm::mat4 lightMatrix = light->getOwner()->getLocalToWorldMatrix();
                            glm::vec3 lightPos = glm::vec3(lightMatrix * glm::vec4(0, 0, 0, 1));
                            glm::vec3 worldDirection = glm::normalize(glm::vec3(lightMatrix * glm::vec4(light->direction, 0.0f)));
                            submeshMaterial->shader->set(prefix + "type", (int)light->lightType);
                            submeshMaterial->shader->set(prefix + "position", lightPos);
                            submeshMaterial->shader->set(prefix + "direction", worldDirection);
                            submeshMaterial->shader->set(prefix + "color", light->getEffectiveColor());
                            submeshMaterial->shader->set(prefix + "attenuation", light->attenuation);
                            submeshMaterial->shader->set(prefix + "inner_cone_angle", light->inner_cone_angle);
                            submeshMaterial->shader->set(prefix + "outer_cone_angle", light->outer_cone_angle);
                        }

                        glBindVertexArray(instancedRenderer->mesh->getVAO());
                        glDrawElementsInstanced(
                            GL_TRIANGLES, submesh.elementCount, GL_UNSIGNED_INT,
                            (void *)(submesh.elementOffset * sizeof(GLuint)),
                            instanceCount);
                    }
                }
                else
                {
                    // No submeshes, use default material
                    instancedRenderer->material->setup();
                    instancedRenderer->material->shader->set("VP", VP);

                    // Set lighting uniforms for instanced lit materials
                    instancedRenderer->material->shader->set("camera_position", eye);
                    instancedRenderer->material->shader->set("light_count", (int)lightCommands.size());
                    instancedRenderer->material->shader->set("fog_enabled", fogEnabled);
                    
                    for (size_t li = 0; li < lightCommands.size(); li++)
                    {
                        LightComponent *light = lightCommands[li];
                        std::string prefix = "lights[" + std::to_string(li) + "].";
                        glm::mat4 lightMatrix = light->getOwner()->getLocalToWorldMatrix();
                        glm::vec3 lightPos = glm::vec3(lightMatrix * glm::vec4(0, 0, 0, 1));
                        glm::vec3 worldDirection = glm::normalize(glm::vec3(lightMatrix * glm::vec4(light->direction, 0.0f)));
                        instancedRenderer->material->shader->set(prefix + "type", (int)light->lightType);
                        instancedRenderer->material->shader->set(prefix + "position", lightPos);
                        instancedRenderer->material->shader->set(prefix + "direction", worldDirection);
                        instancedRenderer->material->shader->set(prefix + "color", light->getEffectiveColor());
                        instancedRenderer->material->shader->set(prefix + "attenuation", light->attenuation);
                        instancedRenderer->material->shader->set(prefix + "inner_cone_angle", light->inner_cone_angle);
                        instancedRenderer->material->shader->set(prefix + "outer_cone_angle", light->outer_cone_angle);
                    }

                    instancedRenderer->mesh->drawInstanced(instanceCount);
                }
            }
        }
        // If there is a sky material, draw the sky
        if (this->skyMaterial)
        {
            // Set up the sky material
            this->skyMaterial->setup();

            // Set fog uniforms for sky
            skyMaterial->shader->set("fog_color", glm::vec3(0.02f, 0.02f, 0.02f));
            skyMaterial->shader->set("fog_enabled", fogEnabled);
            skyMaterial->shader->set("apply_fog", true);

            // Get the camera position
            glm::vec3 cameraPos = M * glm::vec4(0, 0, 0, 1);

            // Create model matrix
            glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), cameraPos);

            // We can acheive the is by multiplying by an extra matrix after the
            // projection but what values should we put in it?
            glm::mat4 alwaysBehindTransform =
                glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                          0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f);

            // Set the "transform" uniform
            skyMaterial->shader->set("transform",
                                     alwaysBehindTransform * VP * modelMatrix);

            // Draw the sky sphere
            skySphere->draw();
        }
        // Draw all the transparent commands
        for (const auto &command : transparentCommands)
        {
            // Setup the material
            command.material->setup();
            // Compute the model-view-projection matrix
            glm::mat4 M = command.localToWorld;
            glm::mat4 MVP = VP * M;
            // Set the "transform" uniform
            command.material->shader->set("transform", MVP);

            // If this is a lit material, set lighting uniforms
            if (dynamic_cast<LitMaterial *>(command.material))
            {
                command.material->shader->set("camera_position", eye);
                command.material->shader->set("light_count", (int)lightCommands.size());
                command.material->shader->set("M", M);
                command.material->shader->set("M_IT", glm::transpose(glm::inverse(M)));

                // Set uniforms for each light
                for (size_t i = 0; i < lightCommands.size(); i++)
                {
                    LightComponent *light = lightCommands[i];
                    std::string prefix = "lights[" + std::to_string(i) + "].";

                    // Get light world position and direction from its entity transform
                    glm::mat4 lightMatrix = light->getOwner()->getLocalToWorldMatrix();
                    glm::vec3 lightPos = glm::vec3(lightMatrix * glm::vec4(0, 0, 0, 1));
                    glm::vec3 worldDirection = glm::normalize(glm::vec3(lightMatrix * glm::vec4(light->direction, 0.0f)));

                    command.material->shader->set(prefix + "type", (int)light->lightType);
                    command.material->shader->set(prefix + "position", lightPos);
                    command.material->shader->set(prefix + "direction", worldDirection);
                    command.material->shader->set(prefix + "color", light->getEffectiveColor());
                    command.material->shader->set(prefix + "attenuation", light->attenuation);
                    command.material->shader->set(prefix + "inner_cone_angle", light->inner_cone_angle);
                    command.material->shader->set(prefix + "outer_cone_angle", light->outer_cone_angle);
                }
            }

            // Draw the mesh
            if (command.submeshIndex >= 0)
            {
                command.mesh->drawSubmesh(command.submeshIndex);
            }
            else
            {
                command.mesh->draw();
            }
        }

        // If there is a postprocess material, apply postprocessing
        if (postprocessMaterial)
        {
            // First, bind the default framebuffer for postprocessing output
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Set viewport to full window
            glViewport(0, 0, windowSize.x, windowSize.y);

            // Render postprocess quad
            postprocessMaterial->setup();

            // Values that will be set by the game logic
            postprocessMaterial->shader->set("health", postprocessUniforms.health);
            postprocessMaterial->shader->set("maxHealth", postprocessUniforms.maxHealth);
            postprocessMaterial->shader->set("time", postprocessUniforms.time);

            glBindVertexArray(postProcessVertexArray);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);
        }
    }

    void ForwardRenderer::setStaticParams(const float maxHealth, const float health)
    {
        postprocessUniforms.maxHealth = maxHealth;
        postprocessUniforms.health = health;
        postprocessUniforms.time = (float)glfwGetTime();
    }

    const Frustum &ForwardRenderer::getFrustum() const { return frustum; }
} // namespace our