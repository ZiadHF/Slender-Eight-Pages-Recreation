#include "tree-utils.hpp"
#include "../texture/texture-utils.hpp"
#include <stb/stb_image.h>
#include <random>
#include <iostream>
namespace our
{
    std::vector<TreeInstance> generateFromMap(const std::string &mapFilename, glm::vec2 worldSize, float density,
                                          glm::vec3 posRandomRange,
                                          glm::vec3 rotRandomRange,
                                          glm::vec2 scaleRandomRange,
                                          float minTreeDistance,
                                          bool usePositionRegistry)
    {
        int width, height, channels;
        // Load the image
        unsigned char *data = stbi_load(mapFilename.c_str(), &width, &height, &channels, 0);
        
        if (!data) {
            std::cerr << "Failed to load tree map: " << mapFilename << std::endl;
            std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
            return {};
        }
   
        std::random_device dev;
        std::mt19937 gen(dev());

        // Print seed for debugging
        std::cout << "Random generator seeded with: " << dev() << std::endl;
        
        // Create distributions based on provided ranges
        std::uniform_real_distribution<float> randomOffsetX(-posRandomRange.x, posRandomRange.x);
        std::uniform_real_distribution<float> randomOffsetY(-posRandomRange.y, posRandomRange.y);
        std::uniform_real_distribution<float> randomOffsetZ(-posRandomRange.z, posRandomRange.z);
        
        std::uniform_real_distribution<float> randomRotX(-rotRandomRange.x, rotRandomRange.x);
        std::uniform_real_distribution<float> randomRotY(-rotRandomRange.y, rotRandomRange.y);
        std::uniform_real_distribution<float> randomRotZ(-rotRandomRange.z, rotRandomRange.z);
        
        std::uniform_real_distribution<float> randomScale(scaleRandomRange.x, scaleRandomRange.y);
        std::uniform_real_distribution<float> randomWidthScale(0.7f, 2.4f);
        
        std::vector<TreeInstance> instances;
        TreePositionRegistry& registry = TreePositionRegistry::getInstance();
        
        // How many pixels to traverse according to the density 
        float pixelsPerTree = 1.0f/density;
        int step = glm::max(1, (int) glm::sqrt(pixelsPerTree));
     
        
        for (int y = 0; y < height; y+=step)
        {
            for (int x = 0; x < width; x+=step)
            {
                int index = (y * width + x) * channels;
                unsigned char pixelValue = data[index];
                
                if (pixelValue > 128) {
                    float normalizedX = (float)x / width;
                    float normalizedY = (float)y / height;
                    
                    glm::vec3 basePos = glm::vec3(
                        (normalizedX - 0.5f) * worldSize.x,
                        0.0f,
                        (normalizedY - 0.5f) * worldSize.y
                    );
                    
                    glm::vec3 randomOffset = glm::vec3(
                        randomOffsetX(gen),
                        randomOffsetY(gen),
                        randomOffsetZ(gen)
                    );
                    
                    glm::vec3 finalPos = basePos + randomOffset;
                    
                    // Check if position is valid before adding (or skip if registry disabled)
                    bool positionValid = !usePositionRegistry || registry.isPositionValid(finalPos, minTreeDistance);
                    if (positionValid) {
                        TreeInstance instance;
                        instance.pos = basePos;
                        instance.posRandom = randomOffset;
                        instance.scale = 1.0f;
                        instance.scaleRandom = randomScale(gen);
                        instance.rotation = 0.0f;
                        instance.rotationRandom = glm::vec3(
                            randomRotX(gen),
                            randomRotY(gen),
                            randomRotZ(gen)
                        );
                        instance.widthScale = randomWidthScale(gen);
                        
                        instances.push_back(instance);
                        if (usePositionRegistry) {
                            registry.registerPosition(finalPos);
                        }
                    }
                }
            }
        }

       
        stbi_image_free(data);
        
      
        
        return instances;
    }
}