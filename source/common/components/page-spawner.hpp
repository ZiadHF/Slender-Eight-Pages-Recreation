namespace our {

    class PageSpawnerComponent : public Component {
      public:
        // Total number of pages in the game
        int totalPages = 0;
        std::vector<std::pair<glm::vec3, glm::vec3>> spawnPoints;
        std::vector<std::string> pageTextures;

        static std::string getID() {
            return "Page Spawner Component";
        }

        void deserialize(const nlohmann::json& data) override {
            // Get total pages from JSON
            if (data.contains("totalPages") && data["totalPages"].is_number_integer()) {
                totalPages = data["totalPages"];
            }

            // Get spawn points and rotations from JSON
            if (data.contains("spawnPoints") && data["spawnPoints"].is_array()) {
                for (const auto& obj : data["spawnPoints"]) {
                    const auto &point = obj["spawnPoint"];
                    if (point.is_array() && point.size() == 3) {
                        glm::vec3 spawnPoint;
                        spawnPoint.x = point[0];
                        spawnPoint.y = point[1];
                        spawnPoint.z = point[2];
                        spawnPoints.push_back({spawnPoint, glm::vec3(0.0f)});
                    }

                    const auto &rotation = obj["rotation"];
                    if (rotation.is_array() && rotation.size() == 3) {
                        glm::vec3 rot;
                        rot.x = rotation[0];
                        rot.y = rotation[1];
                        rot.z = rotation[2];
                        spawnPoints.back().second = rot;
                    }
                }
            }

            // Get page textures from JSON
            if (data.contains("pageTextures") && data["pageTextures"].is_array()) {
                for (const auto& texture : data["pageTextures"]) {
                    if (texture.is_string()) {
                        pageTextures.push_back(texture.get<std::string>());
                    }
                }
            }
        }
    };

};