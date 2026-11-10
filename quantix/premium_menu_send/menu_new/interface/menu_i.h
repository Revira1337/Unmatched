#pragma once
#include <string>
#include <d3d11.h>

class c_menu
{
public:
    virtual ~c_menu() = default;

    virtual void setup_data(ID3D11Device* device) = 0;
    virtual void setup_imgui() = 0;
    virtual void draw() = 0;
    virtual bool begin(const char* name) = 0;
    virtual void background() = 0;
    virtual void end() = 0;
    virtual void watermark() {}          // drawn every frame (even when the menu is closed)
    virtual bool splash_finished() { return true; }   // false while the intro is still playing

    virtual void destroy() = 0;
};

c_menu* create_menu();
