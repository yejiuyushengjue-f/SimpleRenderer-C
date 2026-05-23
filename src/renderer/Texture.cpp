#include "renderer/Texture.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

#ifdef _WIN32
#include <wincodec.h>
#include <wrl/client.h>
#endif

namespace sr {

#ifdef _WIN32
namespace {

class ComScope {
public:
    ComScope()
    {
        result_ = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        initialized_ = SUCCEEDED(result_);
    }

    ~ComScope()
    {
        if (initialized_) {
            CoUninitialize();
        }
    }

    HRESULT result() const { return result_; }

private:
    HRESULT result_ = S_OK;
    bool initialized_ = false;
};

} // namespace
#endif

Texture::Texture(int width, int height, std::vector<Color> pixels)
    : width_(width)
    , height_(height)
    , pixels_(std::move(pixels))
{
    if (width_ <= 0 || height_ <= 0 || pixels_.size() != static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_)) {
        throw std::runtime_error("Invalid texture dimensions or pixel data.");
    }
}

Color Texture::sample(Vec2 uv) const
{
    if (empty()) {
        return { 255, 255, 255, 255 };
    }

    uv.x = uv.x - std::floor(uv.x);
    uv.y = uv.y - std::floor(uv.y);

    const int x = std::clamp(static_cast<int>(uv.x * static_cast<float>(width_)), 0, width_ - 1);
    const int y = std::clamp(static_cast<int>((1.0f - uv.y) * static_cast<float>(height_)), 0, height_ - 1);
    return pixels_[static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x)];
}

Texture Texture::loadFromFile(const wchar_t* path)
{
#ifdef _WIN32
    ComScope com;
    if (FAILED(com.result()) && com.result() != RPC_E_CHANGED_MODE) {
        throw std::runtime_error("Failed to initialize COM for texture loading.");
    }

    Microsoft::WRL::ComPtr<IWICImagingFactory> factory;
    HRESULT result = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&factory));
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create WIC imaging factory.");
    }

    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    result = factory->CreateDecoderFromFilename(path, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
    if (FAILED(result)) {
        throw std::runtime_error("Failed to open texture file.");
    }

    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
    result = decoder->GetFrame(0, &frame);
    if (FAILED(result)) {
        throw std::runtime_error("Failed to decode texture frame.");
    }

    Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
    result = factory->CreateFormatConverter(&converter);
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create texture format converter.");
    }

    result = converter->Initialize(
        frame.Get(),
        GUID_WICPixelFormat32bppRGBA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeCustom);
    if (FAILED(result)) {
        throw std::runtime_error("Failed to convert texture to RGBA.");
    }

    UINT width = 0;
    UINT height = 0;
    converter->GetSize(&width, &height);

    std::vector<Color> pixels(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));
    result = converter->CopyPixels(
        nullptr,
        width * sizeof(Color),
        static_cast<UINT>(pixels.size() * sizeof(Color)),
        reinterpret_cast<BYTE*>(pixels.data()));
    if (FAILED(result)) {
        throw std::runtime_error("Failed to copy texture pixels.");
    }

    return Texture(static_cast<int>(width), static_cast<int>(height), std::move(pixels));
#else
    (void)path;
    throw std::runtime_error("Texture file loading is currently implemented for Windows only.");
#endif
}

Texture Texture::makeCheckerboard(int width, int height, int cells)
{
    std::vector<Color> pixels(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));
    const int cellWidth = std::max(1, width / cells);
    const int cellHeight = std::max(1, height / cells);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const bool dark = ((x / cellWidth) + (y / cellHeight)) % 2 == 0;
            const bool line = x % cellWidth == 0 || y % cellHeight == 0;

            Color color = dark ? Color { 38, 42, 58, 255 } : Color { 228, 236, 246, 255 };
            if (line) {
                color = { 255, 196, 87, 255 };
            }

            pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)] = color;
        }
    }

    return Texture(width, height, std::move(pixels));
}

} // namespace sr
