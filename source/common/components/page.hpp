namespace our {
    class PageComponent : public Component {
      public:
        bool isCollected = false;

        static std::string getID() {
            return "Page Component";
        }

        void deserialize(const nlohmann::json& data) override{
            
        }
    };
};