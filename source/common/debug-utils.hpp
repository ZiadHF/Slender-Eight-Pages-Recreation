#pragma once

// Simple debug utility - controls verbose output for materials and meshes
// Set via app.json: "debug_mode": true/false in the root object

namespace our {
    // Global debug flag - can be set by application on startup
    inline bool g_debugMode = false;
    
    inline void setDebugMode(bool enabled) {
        g_debugMode = enabled;
    }
    
    inline bool isDebugMode() {
        return g_debugMode;
    }
}

// Convenience macro for conditional debug output
#define DEBUG_LOG(msg) if (our::g_debugMode) { std::cout << msg; }
#define DEBUG_LOG_LINE(msg) if (our::g_debugMode) { std::cout << msg << std::endl; }
