namespace our {
    class PageComponent : public Component {
      public:
        // Note: This component is pretty much just a marker component and can probably be removed
        // Has the page been collected by the player
        bool isCollected = false;

        static std::string getID() {
            return "Page Component";
        }

        void deserialize(const nlohmann::json& data) override{
            
        }
    };
};