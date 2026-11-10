// gui_stubs.cpp - Dummy implementations for old framework functions
// These are needed by some helper files but not actually used in the new menu system

#include "headers/includes.h"
#include "headers/functions.h"

// Dummy c_gui functions
void c_gui::render() {
    // Not used - new menu system handles rendering
}

void c_gui::set_style() {
    // Not used - new menu handles styling
}

// Dummy DebugLog function
void DebugLog(const char* format, ...) {
    // Can be implemented if needed for debugging
}

// Dummy OnRender and OnDraw
void OnRender() {
    // Not used - new menu system handles rendering
}

void OnDraw() {
    // Not used - new menu system handles rendering
}
