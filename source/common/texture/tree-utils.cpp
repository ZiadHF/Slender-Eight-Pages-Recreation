#include "tree-utils.hpp"
#include "../texture/texture-utils.hpp"
#include <stb/stb_image.h>
#include <random>
#include <iostream>
namespace our
{
    std::vector<TreeInstance> generateFromMap(const std::string &mapFilename, glm::vec2 worldSize, float density)
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
        std::uniform_real_distribution<float> randomOffset(-10.0f, 10.0f); // Increased range for more variation
        std::uniform_real_distribution<float> randomScale(0.5f, 1.5f);   // Wider scale range
        std::uniform_real_distribution<float> randomRotation(0.0f, 6.28318530718f); // 0 to 2*PI radians
        std::uniform_real_distribution<float> randomWidthScale(0.7f, 2.4f); // Width variation
        std::vector<TreeInstance> instances;
        // How many pixels to traverse according to the density 
        float pixelsPerTree = 1.0f/density;
        int step = glm::max(1, (int) glm::sqrt(pixelsPerTree));
     
        // Loop on grayscale map
        int blackPixels = 0;
        for (int y = 0; y < height; y+=step)
        {
            for (int x = 0; x < width; x+=step)
            {
                int index = (y * width + x) * channels;
                unsigned char pixelValue = data[index];
                
                if (pixelValue > 128) {
                    blackPixels++;
                    float normalizedX = (float)x / width;
                    float normalizedY = (float)y / height;
                    
                    TreeInstance instance;
                    instance.pos = glm::vec3(
                        (normalizedX - 0.5f) * worldSize.x + randomOffset(gen),
                        0.0f,
                        (normalizedY - 0.5f) * worldSize.y + randomOffset(gen)
                    );
                    instance.scale = randomScale(gen);
                    instance.rotation = randomRotation(gen);
                    instance.widthScale = randomWidthScale(gen);
                    
                    instances.push_back(instance);
                }
            }
        }

       
        stbi_image_free(data);
        
      
        
        return instances;
    }
}