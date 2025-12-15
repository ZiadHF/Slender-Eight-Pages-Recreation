#include "tree-utils.hpp"

#include <stb/stb_image.h>

#include <iostream>
#include <random>

#include "../texture/texture-utils.hpp"
#include "../debug-utils.hpp"

namespace our {
std::vector<TreeInstance> generateFromMap(const std::string& mapFilename,
                                          glm::vec2 worldSize, float density,
                                          glm::vec3 posRandomRange,
                                          glm::vec3 rotRandomRange,
                                          glm::vec2 scaleRandomRange) {
    int width, height, channels;
    // Load the image
    unsigned char* data =
        stbi_load(mapFilename.c_str(), &width, &height, &channels, 0);

    if (!data) {
        std::cerr << "Failed to load tree map: " << mapFilename << std::endl;
        std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
        return {};
    }

    std::random_device dev;
    std::mt19937 gen(dev());

    // Print seed for debugging
    if (our::g_debugMode)
        std::cout << "Random generator seeded with: " << dev() << std::endl;

    // Create distributions based on provided ranges
    std::uniform_real_distribution<float> randomOffsetX(-posRandomRange.x,
                                                        posRandomRange.x);
    std::uniform_real_distribution<float> randomOffsetY(-posRandomRange.y,
                                                        posRandomRange.y);
    std::uniform_real_distribution<float> randomOffsetZ(-posRandomRange.z,
                                                        posRandomRange.z);

    std::uniform_real_distribution<float> randomRotX(-rotRandomRange.x,
                                                     rotRandomRange.x);
    std::uniform_real_distribution<float> randomRotY(-rotRandomRange.y,
                                                     rotRandomRange.y);
    std::uniform_real_distribution<float> randomRotZ(-rotRandomRange.z,
                                                     rotRandomRange.z);

    std::uniform_real_distribution<float> randomScale(scaleRandomRange.x,
                                                      scaleRandomRange.y);
    std::uniform_real_distribution<float> randomWidthScale(0.7f, 2.4f);

    std::vector<TreeInstance> instances;
    // How many pixels to traverse according to the density
    float pixelsPerTree = 1.0f / density;
    int step = glm::max(1, (int)glm::sqrt(pixelsPerTree));

    // Loop on grayscale map
    int blackPixels = 0;
    for (int y = 0; y < height; y += step) {
        for (int x = 0; x < width; x += step) {
            int index = (y * width + x) * channels;
            unsigned char pixelValue = data[index];

            if (pixelValue > 128) {
                blackPixels++;
                float normalizedX = (float)x / width;
                float normalizedY = (float)y / height;

                TreeInstance instance;
                instance.pos =
                    glm::vec3((normalizedX - 0.5f) * worldSize.x, 0.0f,
                              (normalizedY - 0.5f) * worldSize.y);

                instance.posRandom = glm::vec3(
                    randomOffsetX(gen), randomOffsetY(gen), randomOffsetZ(gen));

                instance.scale = 1.0f;
                instance.scaleRandom = randomScale(gen);

                instance.rotation = 0.0f;
                instance.rotationRandom = glm::vec3(
                    randomRotX(gen), randomRotY(gen), randomRotZ(gen));

                instance.widthScale = randomWidthScale(gen);

                instances.push_back(instance);
            }
        }
    }

    stbi_image_free(data);

    return instances;
}
}  // namespace our