#pragma once
#include <d3d11.h>

namespace tex
{
    inline ID3D11ShaderResourceView* bg = nullptr;
    inline ID3D11ShaderResourceView* sidebar_bg = nullptr;
    inline ID3D11ShaderResourceView* main_logo = nullptr;
    inline ID3D11ShaderResourceView* brand_logo = nullptr;   // Quantix eye logo (watermark)
    inline ID3D11ShaderResourceView* esp = nullptr;
}
