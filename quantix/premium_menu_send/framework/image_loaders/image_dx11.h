#pragma once
#include "../headers/includes.h"
#include <d3d11.h>
#include <D3DX11.h>
#pragma comment (lib, "d3dx11.lib")
#include "imgui_impl_dx11.h"

#include "../settings/variables.h"

using BackendDeviceType = ID3D11Device*;
using TextureType = ID3D11ShaderResourceView*;

class c_image
{
private:
    struct image_data {
        TextureType data;
        ImVec2 size;
    };

public:
    void init(BackendDeviceType device);

    inline void add(BackendDeviceType device, const std::string& name, const unsigned char* data, size_t data_size, ImVec2&& image_size) {
        D3DX11CreateShaderResourceViewFromMemory(device, data, data_size, nullptr, nullptr, &images[name].data, 0);
        images[name].size = std::move(image_size);
    }

    inline bool add_file(BackendDeviceType device, const std::string& name, const char* path) {
        TextureType srv = nullptr;
        const HRESULT hr = D3DX11CreateShaderResourceViewFromFileA(device, path, nullptr, nullptr, &srv, nullptr);
        if (FAILED(hr) || !srv) {
            return false;
        }

        images[name].data = srv;
        images[name].size = { 0.f, 0.f };

        ID3D11Resource* resource = nullptr;
        srv->GetResource(&resource);
        if (!resource) {
            return true;
        }

        ID3D11Texture2D* texture = nullptr;
        const HRESULT hrQuery = resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&texture);
        resource->Release();
        if (FAILED(hrQuery) || !texture) {
            return true;
        }

        D3D11_TEXTURE2D_DESC desc{};
        texture->GetDesc(&desc);
        texture->Release();

        images[name].size = { (float)desc.Width, (float)desc.Height };
        return true;
    }

    ImTextureID get(const std::string& name) const {
        return (ImTextureID)(intptr_t)(images.at(name).data);
    }

    ImVec2 get_size(const std::string& name) const {
        return images.at(name).size * 0.5f * var->gui.stored_dpi;
    }

    ImVec2 get_real_size(const std::string& name) const {
        return images.at(name).size * var->gui.stored_dpi;
    }

    ~c_image() {
        for (auto& image : images) {
            free(image.second.data);
        }

        images.clear();
    }

private:
    std::unordered_map<std::string, image_data> images;
};

inline std::unique_ptr<c_image> image = std::make_unique<c_image>();
