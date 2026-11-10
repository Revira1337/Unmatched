// interface_stub.cpp - Dummy interfaceObject for compatibility
#include "Render/Interface.hpp"

// Create dummy interfaceObject - not actually used in new system
M_base::Interface interfaceObject;

// Dummy ShutDown implementation
namespace M_base {
    void Interface::ShutDown() {
        // Not used in new system
    }
}
