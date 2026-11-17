// Development-only collision + material capture for the static minimap.
//
// This module is inert unless MANAKA_CAPTURE_COLOR_MAP is set before the game
// starts. It never enables a camera and never changes scene materials. The
// normal MOD startup path executes one environment lookup and returns.

enum DevMapCaptureState
{
    DevMapCapture_Disabled = 0,
    DevMapCapture_WaitForWarehouse,
    DevMapCapture_WaitForStage,
    DevMapCapture_WaitForSettle,
    DevMapCapture_WaitForTerrainRender,
    DevMapCapture_Scanning,
    DevMapCapture_RasterizingVisuals,
    DevMapCapture_PostProcess,
    DevMapCapture_Complete,
};

struct DevMapCaptureBounds
{
    float minX;
    float maxX;
    float minY;
    float maxY;
    float minZ;
    float maxZ;
};

struct DevMapRawHit
{
    float point[3];
    float normal[3];
    uint32_t faceId;
    float distance;
    float uv[2];
    int colliderId;
};

struct DevMapNavMeshHit
{
    float position[3];
    float normal[3];
    float distance;
    int mask;
    int hit;
};

struct DevMapCaptureTexture
{
    Il2CppObject* readable = nullptr;
    uint32_t gcHandle = 0;
    int width = 0;
    int height = 0;
    std::vector<uint8_t> rgba;
};

struct DevMapCaptureMaterial
{
    Il2CppObject* material = nullptr;
    Il2CppObject* texture = nullptr;
    Il2CppObject* readableTexture = nullptr;
    float baseColor[4]{1.0f, 1.0f, 1.0f, 1.0f};
    float averageColor[4]{0.0f, 0.0f, 0.0f, 0.0f};
    float uvScale[2]{1.0f, 1.0f};
    float uvOffset[2]{0.0f, 0.0f};
    bool hasTexture = false;
    bool hasAverageColor = false;
};

struct DevMapCapturePaint
{
    std::vector<DevMapCaptureMaterial> materials;
    std::vector<int> faceEnds;
    Il2CppObject* visualTransform = nullptr;
    const float* vertices = nullptr;
    const float* uvs = nullptr;
    std::vector<int> triangles;
    int vertexCount = 0;
};

constexpr int kDevMapCaptureSize = 2560;
constexpr float kDevMapWalkableNormalY = 0.55f;
constexpr float kDevMapNavMeshSnapDistance = 0.65f;
constexpr float kDevMapNavMeshHeightTolerance = 0.50f;
constexpr int kDevMapCellsPerFrame = 4096;
constexpr int kDevMapRaycastCapacity = 64;
constexpr int kDevMapColliderCapacity = 32;
constexpr int kDevMapRenderersPerFrame = 24;
constexpr float kDevMapPlayerCapsuleRadius = 0.25f;
constexpr float kDevMapPlayerCapsuleHeight = 1.55f;
constexpr float kDevMapPlayerCapsuleGroundClearance = 0.04f;
constexpr float kDevMapVisualSurfaceToleranceBelow = 0.60f;
constexpr float kDevMapVisualSurfaceToleranceAbove = 0.80f;

DevMapCaptureState g_devMapState = DevMapCapture_Disabled;
DevMapCaptureBounds g_devMapBounds{};
int g_devMapStageType = 14;
int g_devMapSettleFrames = 0;
int g_devMapCursor = 0;
int g_devMapFrames = 0;
int g_devMapAutoStartDelay = -1;
uint32_t g_devMapHitArrayHandle = 0;
Il2CppObject* g_devMapHitArray = nullptr;
uint32_t g_devMapColliderArrayHandle = 0;
Il2CppObject* g_devMapColliderArray = nullptr;
uint32_t g_devMapRendererArrayHandle = 0;
Il2CppObject* g_devMapRendererArray = nullptr;
int g_devMapRendererCursor = 0;
int g_devMapRendererCount = 0;
std::vector<uint8_t> g_devMapPixels;
std::vector<float> g_devMapHeights;
std::vector<uint8_t> g_devMapWalkable;
std::vector<float> g_devMapSurfaceScores;
std::vector<float> g_devMapVisualHeights;
std::vector<uint8_t> g_devMapVisualValid;
std::unordered_map<int, Il2CppObject*> g_devMapColliders;
std::unordered_map<int, DevMapCapturePaint> g_devMapPaints;
std::unordered_map<Il2CppObject*, DevMapCaptureTexture> g_devMapTextures;
std::vector<std::pair<Il2CppObject*, bool>> g_devMapTerrainRendererStates;
Il2CppObject* g_devMapTerrainCameraObject = nullptr;
Il2CppObject* g_devMapTerrainCamera = nullptr;
Il2CppObject* g_devMapTerrainRenderTexture = nullptr;
Il2CppObject* g_devMapTerrainPreviousActiveTexture = nullptr;
int g_devMapTerrainRenderFrames = 0;
long long g_devMapRaycastCount = 0;
long long g_devMapNavAccepted = 0;
long long g_devMapRendererResolved = 0;
long long g_devMapRendererMissing = 0;
long long g_devMapTextureSamples = 0;
long long g_devMapVisualUvSamples = 0;
long long g_devMapColorFallbacks = 0;
long long g_devMapCapsuleQueries = 0;
long long g_devMapCapsuleBlocked = 0;
long long g_devMapVisualRenderersVisited = 0;
long long g_devMapVisualTriangles = 0;
long long g_devMapVisualPixels = 0;
long long g_devMapVisualSkippedUnreadable = 0;
long long g_devMapVisualTrianglesUpward = 0;
long long g_devMapVisualTriangleCandidates = 0;
long long g_devMapVisualHeightRejected = 0;
long long g_devMapVisualAlphaRejected = 0;

bool DevMapCapture_ReadBool(Il2CppObject* value)
{
    return value != nullptr &&
        il2cpp_object_unbox != nullptr &&
        *static_cast<bool*>(il2cpp_object_unbox(value));
}

int DevMapCapture_ReadInt(Il2CppObject* value)
{
    return value != nullptr && il2cpp_object_unbox != nullptr
        ? *static_cast<int*>(il2cpp_object_unbox(value))
        : 0;
}

float DevMapCapture_ReadFloat(Il2CppObject* value)
{
    return value != nullptr && il2cpp_object_unbox != nullptr
        ? *static_cast<float*>(il2cpp_object_unbox(value))
        : 0.0f;
}

void DevMapCapture_Initialize()
{
    if (g_devMapState != DevMapCapture_Disabled)
    {
        return;
    }

    wchar_t request[32]{};
    const DWORD length = GetEnvironmentVariableW(
        L"MANAKA_CAPTURE_COLOR_MAP",
        request,
        static_cast<DWORD>(sizeof(request) / sizeof(request[0])));
    if (length == 0 || request[0] == L'\0' || request[0] == L'0')
    {
        g_devMapState = DevMapCapture_Complete;
        return;
    }

    if (_wcsicmp(request, L"park") != 0)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"Colored map capture ignored unsupported request: %s.", request);
        g_devMapState = DevMapCapture_Complete;
        return;
    }

    g_devMapStageType = 14;
    g_devMapBounds = {
        6.190000f,
        155.544998f,
        49.001999f,
        52.145000f,
        8.386999f,
        157.720001f,
    };
    g_devMapState = DevMapCapture_WaitForWarehouse;
    LogInfo(LogCategory_Scene, L"Scene", 
        L"Colored map capture armed for Park: bounds x=[%.3f,%.3f] y=[%.3f,%.3f] z=[%.3f,%.3f].",
        g_devMapBounds.minX,
        g_devMapBounds.maxX,
        g_devMapBounds.minY,
        g_devMapBounds.maxY,
        g_devMapBounds.minZ,
        g_devMapBounds.maxZ);
}

bool DevMapCapture_GetReadableTexture(
    Il2CppObject* source,
    Il2CppObject*& readable)
{
    readable = nullptr;
    if (source == nullptr)
    {
        return false;
    }

    const auto cached = g_devMapTextures.find(source);
    if (cached != g_devMapTextures.end())
    {
        readable = cached->second.readable;
        return readable != nullptr;
    }

    auto* textureClass = FindClass("UnityEngine", "Texture");
    auto* texture2DClass = FindClass("UnityEngine", "Texture2D");
    auto* renderTextureClass = FindClass("UnityEngine", "RenderTexture");
    auto* graphicsClass = FindClass("UnityEngine", "Graphics");
    if (textureClass == nullptr ||
        texture2DClass == nullptr ||
        renderTextureClass == nullptr ||
        graphicsClass == nullptr)
    {
        g_devMapTextures[source] = {};
        return false;
    }

    int sourceWidth = DevMapCapture_ReadInt(
        Invoke(FindMethod(textureClass, "get_width", 0), source, nullptr));
    int sourceHeight = DevMapCapture_ReadInt(
        Invoke(FindMethod(textureClass, "get_height", 0), source, nullptr));
    if (sourceWidth <= 0 || sourceHeight <= 0)
    {
        g_devMapTextures[source] = {};
        return false;
    }

    constexpr int sampleSize = 256;
    auto* previousActive = Invoke(
        FindMethod(renderTextureClass, "get_active", 0),
        nullptr,
        nullptr);
    auto* renderTexture = il2cpp_object_new(renderTextureClass);
    int depth = 0;
    void* renderTextureParameters[] = {
        const_cast<int*>(&sampleSize),
        const_cast<int*>(&sampleSize),
        &depth,
    };
    Invoke(
        FindMethod(renderTextureClass, ".ctor", 3),
        renderTexture,
        renderTextureParameters);
    Invoke(
        FindMethod(renderTextureClass, "Create", 0),
        renderTexture,
        nullptr);
    auto* textureCopy = il2cpp_object_new(texture2DClass);
    int textureFormat = 4; // TextureFormat.RGBA32
    bool mipChain = false;
    void* textureParameters[] = {
        const_cast<int*>(&sampleSize),
        const_cast<int*>(&sampleSize),
        &textureFormat,
        &mipChain,
    };
    Invoke(
        FindMethod(texture2DClass, ".ctor", 4),
        textureCopy,
        textureParameters);
    if (renderTexture == nullptr || textureCopy == nullptr)
    {
        g_devMapTextures[source] = {};
        return false;
    }

    void* blitParameters[] = { source, renderTexture };
    Invoke(
        FindMethod(graphicsClass, "Blit", 2),
        nullptr,
        blitParameters);

    auto* setActive = FindMethod(renderTextureClass, "set_active", 1);
    void* activeParameters[] = { renderTexture };
    Invoke(setActive, nullptr, activeParameters);
    float rect[] = {
        0.0f,
        0.0f,
        static_cast<float>(sampleSize),
        static_cast<float>(sampleSize),
    };
    int destX = 0;
    int destY = 0;
    bool recalculateMipMaps = false;
    void* readParameters[] = {
        rect,
        &destX,
        &destY,
        &recalculateMipMaps,
    };
    Invoke(
        FindMethod(texture2DClass, "ReadPixels", 4),
        textureCopy,
        readParameters);
    Invoke(FindMethod(texture2DClass, "Apply", 0), textureCopy, nullptr);

    std::vector<uint8_t> rgba;
    auto* pixels = Invoke(
        FindMethod(texture2DClass, "GetPixels", 0),
        textureCopy,
        nullptr);
    if (pixels != nullptr &&
        il2cpp_array_length(pixels) >=
            static_cast<uint32_t>(sampleSize * sampleSize) &&
        il2cpp_array_object_header_size != nullptr)
    {
        const auto* colors = reinterpret_cast<const float*>(
            reinterpret_cast<const char*>(pixels) +
            il2cpp_array_object_header_size());
        rgba.resize(
            static_cast<size_t>(sampleSize) * sampleSize * 4);
        for (size_t pixel = 0;
             pixel < static_cast<size_t>(sampleSize) * sampleSize;
             ++pixel)
        {
            for (int channel = 0; channel < 4; ++channel)
            {
                const float value = std::clamp(
                    colors[pixel * 4 + channel],
                    0.0f,
                    1.0f);
                rgba[pixel * 4 + channel] =
                    static_cast<uint8_t>(std::round(value * 255.0f));
            }
        }
    }

    void* restoreParameters[] = { previousActive };
    Invoke(setActive, nullptr, restoreParameters);
    void* destroyParameters[] = { renderTexture };
    Invoke(
        FindMethod(FindClass("UnityEngine", "Object"), "Destroy", 1),
        nullptr,
        destroyParameters);

    if (il2cpp_gchandle_new != nullptr)
    {
        g_devMapTextures[source] = {
            textureCopy,
            il2cpp_gchandle_new(textureCopy, false),
            sampleSize,
            sampleSize,
            std::move(rgba),
        };
    }
    else
    {
        g_devMapTextures[source] = {
            textureCopy,
            0,
            sampleSize,
            sampleSize,
            std::move(rgba),
        };
    }
    readable = textureCopy;
    LogInfo(LogCategory_Scene, L"Scene", 
        L"Colored map capture copied non-readable texture %p (%dx%d) to %dx%d RGBA32.",
        source,
        sourceWidth,
        sourceHeight,
        sampleSize,
        sampleSize);
    return readable != nullptr;
}

float DevMapCapture_ReadColorChannel(
    Il2CppObject* material,
    const char* property,
    int channel,
    float fallback)
{
    auto* materialClass = FindClass("UnityEngine", "Material");
    auto* propertyString = il2cpp_string_new(property);
    void* hasParameters[] = { propertyString };
    if (!DevMapCapture_ReadBool(Invoke(
            FindMethod(materialClass, "HasProperty", 1),
            material,
            hasParameters)))
    {
        return fallback;
    }
    void* colorParameters[] = { propertyString };
    auto* color = Invoke(
        FindMethod(materialClass, "GetColor", 1),
        material,
        colorParameters);
    if (color == nullptr || il2cpp_object_unbox == nullptr)
    {
        return fallback;
    }
    return static_cast<float*>(il2cpp_object_unbox(color))[channel];
}

Il2CppObject* DevMapCapture_ReadTextureProperty(
    Il2CppObject* material,
    const char* property)
{
    auto* materialClass = FindClass("UnityEngine", "Material");
    if (materialClass == nullptr)
    {
        return nullptr;
    }
    auto* propertyString = il2cpp_string_new(property);
    void* hasParameters[] = { propertyString };
    if (!DevMapCapture_ReadBool(Invoke(
            FindMethod(materialClass, "HasProperty", 1),
            material,
            hasParameters)))
    {
        return nullptr;
    }
    void* textureParameters[] = { propertyString };
    return Invoke(
        FindMethod(materialClass, "GetTexture", 1),
        material,
        textureParameters);
}

DevMapCaptureMaterial DevMapCapture_BuildMaterial(Il2CppObject* material)
{
    DevMapCaptureMaterial result;
    result.material = material;
    if (material == nullptr)
    {
        return result;
    }

    auto* materialClass = FindClass("UnityEngine", "Material");
    auto* color = Invoke(
        FindMethod(materialClass, "get_color", 0),
        material,
        nullptr);
    if (color != nullptr && il2cpp_object_unbox != nullptr)
    {
        const auto* values = static_cast<float*>(il2cpp_object_unbox(color));
        std::copy(values, values + 4, result.baseColor);
    }
    else
    {
        for (int channel = 0; channel < 3; ++channel)
        {
            result.baseColor[channel] = DevMapCapture_ReadColorChannel(
                material,
                "_BaseColor",
                channel,
                1.0f);
        }
    }

    const char* textureProperties[] = {
        "_BaseMap",
        "_BaseColorMap",
        "_MainTex",
        "_Albedo",
        "_AlbedoMap",
    };
    for (const auto* property : textureProperties)
    {
        result.texture = DevMapCapture_ReadTextureProperty(material, property);
        if (result.texture != nullptr)
        {
            break;
        }
    }
    if (result.texture == nullptr)
    {
        result.texture = Invoke(
            FindMethod(materialClass, "get_mainTexture", 0),
            material,
            nullptr);
    }
    result.hasTexture = DevMapCapture_GetReadableTexture(
        result.texture,
        result.readableTexture);
    auto* scale = Invoke(
        FindMethod(materialClass, "get_mainTextureScale", 0),
        material,
        nullptr);
    auto* offset = Invoke(
        FindMethod(materialClass, "get_mainTextureOffset", 0),
        material,
        nullptr);
    if (scale != nullptr && il2cpp_object_unbox != nullptr)
    {
        std::copy(
            static_cast<float*>(il2cpp_object_unbox(scale)),
            static_cast<float*>(il2cpp_object_unbox(scale)) + 2,
            result.uvScale);
    }
    if (offset != nullptr && il2cpp_object_unbox != nullptr)
    {
        std::copy(
            static_cast<float*>(il2cpp_object_unbox(offset)),
            static_cast<float*>(il2cpp_object_unbox(offset)) + 2,
            result.uvOffset);
    }

    result.averageColor[0] = result.baseColor[0];
    result.averageColor[1] = result.baseColor[1];
    result.averageColor[2] = result.baseColor[2];
    result.averageColor[3] = result.baseColor[3];
    result.hasAverageColor = true;
    if (result.hasTexture && result.texture != nullptr)
    {
        const auto texture = g_devMapTextures.find(result.texture);
        if (texture != g_devMapTextures.end() &&
            !texture->second.rgba.empty())
        {
            double red = 0.0;
            double green = 0.0;
            double blue = 0.0;
            double alpha = 0.0;
            size_t samples = 0;
            const auto& rgba = texture->second.rgba;
            for (size_t pixel = 0;
                 pixel < rgba.size();
                 pixel += 4)
            {
                if (rgba[pixel + 3] < 24)
                {
                    continue;
                }
                red += rgba[pixel + 0];
                green += rgba[pixel + 1];
                blue += rgba[pixel + 2];
                alpha += rgba[pixel + 3];
                ++samples;
            }
            if (samples > 0)
            {
                result.averageColor[0] =
                    static_cast<float>(red / samples / 255.0);
                result.averageColor[1] =
                    static_cast<float>(green / samples / 255.0);
                result.averageColor[2] =
                    static_cast<float>(blue / samples / 255.0);
                result.averageColor[3] =
                    static_cast<float>(alpha / samples / 255.0);
            }
        }
    }
    return result;
}

bool DevMapCapture_SampleMaterialRgb(
    const DevMapCaptureMaterial& material,
    float u,
    float v,
    uint8_t output[3],
    float& alpha)
{
    float red = std::clamp(material.baseColor[0], 0.0f, 1.0f);
    float green = std::clamp(material.baseColor[1], 0.0f, 1.0f);
    float blue = std::clamp(material.baseColor[2], 0.0f, 1.0f);
    alpha = std::clamp(material.baseColor[3], 0.0f, 1.0f);

    if (material.hasTexture && material.texture != nullptr)
    {
        const auto texture = g_devMapTextures.find(material.texture);
        if (texture != g_devMapTextures.end() &&
            texture->second.width > 1 &&
            texture->second.height > 1 &&
            !texture->second.rgba.empty())
        {
            float sampleU =
                u * material.uvScale[0] + material.uvOffset[0];
            float sampleV =
                v * material.uvScale[1] + material.uvOffset[1];
            sampleU -= std::floor(sampleU);
            sampleV -= std::floor(sampleV);
            if (sampleU < 0.0f)
            {
                sampleU += 1.0f;
            }
            if (sampleV < 0.0f)
            {
                sampleV += 1.0f;
            }
            const float sourceX =
                sampleU * static_cast<float>(texture->second.width - 1);
            const float sourceY =
                sampleV * static_cast<float>(texture->second.height - 1);
            const int x0 = static_cast<int>(std::floor(sourceX));
            const int y0 = static_cast<int>(std::floor(sourceY));
            const int x1 = std::min(x0 + 1, texture->second.width - 1);
            const int y1 = std::min(y0 + 1, texture->second.height - 1);
            const float tx = sourceX - static_cast<float>(x0);
            const float ty = sourceY - static_cast<float>(y0);
            const auto sampleChannel = [&](int channel)
            {
                const auto at = [&](int x, int y)
                {
                    const size_t offset =
                        (static_cast<size_t>(y) *
                             texture->second.width +
                         static_cast<size_t>(x)) *
                            4 +
                        channel;
                    return static_cast<float>(
                        texture->second.rgba[offset]) /
                        255.0f;
                };
                const float top = at(x0, y0) * (1.0f - tx) +
                    at(x1, y0) * tx;
                const float bottom = at(x0, y1) * (1.0f - tx) +
                    at(x1, y1) * tx;
                return top * (1.0f - ty) + bottom * ty;
            };
            red *= sampleChannel(0);
            green *= sampleChannel(1);
            blue *= sampleChannel(2);
            alpha *= sampleChannel(3);
            ++g_devMapTextureSamples;
        }
        else
        {
            red *= material.averageColor[0];
            green *= material.averageColor[1];
            blue *= material.averageColor[2];
            ++g_devMapColorFallbacks;
        }
    }
    else
    {
        ++g_devMapColorFallbacks;
    }

    output[0] = static_cast<uint8_t>(
        std::round(std::clamp(red, 0.0f, 1.0f) * 255.0f));
    output[1] = static_cast<uint8_t>(
        std::round(std::clamp(green, 0.0f, 1.0f) * 255.0f));
    output[2] = static_cast<uint8_t>(
        std::round(std::clamp(blue, 0.0f, 1.0f) * 255.0f));
    return true;
}

void DevMapCapture_LogPaint(
    int colliderId,
    Il2CppObject* renderer,
    const DevMapCapturePaint& paint)
{
    static int logged = 0;
    if (logged >= 36)
    {
        return;
    }
    ++logged;
    auto* collider = g_devMapColliders[colliderId];
    LogInfo(LogCategory_Scene, L"Scene", 
        L"Colored map paint[%d]: collider=%s renderer=%s materials=%u.",
        logged,
        GetObjectName(GetComponentGameObject(collider)).c_str(),
        GetObjectName(GetComponentGameObject(renderer)).c_str(),
        static_cast<unsigned int>(paint.materials.size()));
    for (size_t index = 0; index < paint.materials.size(); ++index)
    {
        const auto& item = paint.materials[index];
        std::wstring shaderName;
        if (item.material != nullptr)
        {
            auto* shader = Invoke(
                FindMethod(FindClass("UnityEngine", "Material"), "get_shader", 0),
                item.material,
                nullptr);
            shaderName = GetObjectName(shader);
        }
        LogInfo(LogCategory_Scene, L"Scene", 
            L"  material[%u] name=%s shader=%s color=(%.3f,%.3f,%.3f,%.3f) texture=%s readable=%d.",
            static_cast<unsigned int>(index),
            GetObjectName(item.material).c_str(),
            shaderName.c_str(),
            item.baseColor[0],
            item.baseColor[1],
            item.baseColor[2],
            item.baseColor[3],
            GetObjectName(item.texture).c_str(),
            item.hasTexture ? 1 : 0);
    }
}

DevMapCapturePaint DevMapCapture_BuildPaint(int colliderId)
{
    DevMapCapturePaint result;
    auto* collider = g_devMapColliders[colliderId];
    auto* componentClass = FindClass("UnityEngine", "Component");
    auto* rendererClass = FindClass("UnityEngine", "Renderer");
    auto* meshColliderClass = FindClass("UnityEngine", "MeshCollider");
    auto* meshFilterClass = FindClass("UnityEngine", "MeshFilter");
    auto* skinnedMeshRendererClass =
        FindClass("UnityEngine", "SkinnedMeshRenderer");
    auto* meshClass = FindClass("UnityEngine", "Mesh");
    if (collider == nullptr ||
        componentClass == nullptr ||
        rendererClass == nullptr ||
        rendererClass == nullptr)
    {
        return result;
    }

    il2cpp_runtime_class_init(rendererClass);
    auto* rendererType = il2cpp_type_get_object(
        il2cpp_class_get_type(rendererClass));
    void* rendererParameters[] = { rendererType };
    auto* renderer = Invoke(
        FindMethod(componentClass, "GetComponent", 1),
        collider,
        rendererParameters);
    if (renderer == nullptr)
    {
        bool includeInactive = true;
        void* parentParameters[] = { rendererType, &includeInactive };
        renderer = Invoke(
            FindMethod(componentClass, "GetComponentInParent", 2),
            collider,
            parentParameters);
    }
    if (renderer == nullptr)
    {
        // Some imported scenes keep the visual mesh below the collision root
        // while the collider exists on the root object.
        bool includeInactive = true;
        void* childParameters[] = { rendererType, &includeInactive };
        renderer = Invoke(
            FindMethod(componentClass, "GetComponentInChildren", 2),
            collider,
            childParameters);
    }
    if (renderer == nullptr)
    {
        ++g_devMapRendererMissing;
        static int loggedMissing = 0;
        if (loggedMissing < 48)
        {
            ++loggedMissing;
            LogInfo(LogCategory_Scene, L"Scene", 
                L"Colored map paint missing renderer[%d]: collider=%s.",
                loggedMissing,
                GetObjectName(GetComponentGameObject(collider)).c_str());
        }
        return result;
    }
    ++g_devMapRendererResolved;

    auto* materials = Invoke(
        FindMethod(rendererClass, "get_sharedMaterials", 0),
        renderer,
        nullptr);
    if (materials == nullptr)
    {
        return result;
    }
    const auto materialCount = il2cpp_array_length(materials);
    auto* materialElements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(materials) +
        il2cpp_array_object_header_size());
    for (uint32_t index = 0;
         index < materialCount && index < 32;
         ++index)
    {
        result.materials.push_back(
            DevMapCapture_BuildMaterial(materialElements[index]));
    }
    DevMapCapture_LogPaint(colliderId, renderer, result);

    auto* transform = Invoke(
        FindMethod(componentClass, "get_transform", 0),
        renderer,
        nullptr);
    Il2CppObject* visualMesh = nullptr;
    if (meshFilterClass != nullptr)
    {
        il2cpp_runtime_class_init(meshFilterClass);
        auto* meshFilterType = il2cpp_type_get_object(
            il2cpp_class_get_type(meshFilterClass));
        void* meshFilterParameters[] = { meshFilterType };
        auto* meshFilter = Invoke(
            FindMethod(componentClass, "GetComponent", 1),
            renderer,
            meshFilterParameters);
        if (meshFilter != nullptr)
        {
            visualMesh = Invoke(
                FindMethod(meshFilterClass, "get_sharedMesh", 0),
                meshFilter,
                nullptr);
        }
    }
    if (visualMesh == nullptr && skinnedMeshRendererClass != nullptr)
    {
        il2cpp_runtime_class_init(skinnedMeshRendererClass);
        auto* skinnedType = il2cpp_type_get_object(
            il2cpp_class_get_type(skinnedMeshRendererClass));
        void* skinnedParameters[] = { skinnedType };
        auto* skinned = Invoke(
            FindMethod(componentClass, "GetComponent", 1),
            renderer,
            skinnedParameters);
        if (skinned != nullptr)
        {
            visualMesh = Invoke(
                FindMethod(skinnedMeshRendererClass, "get_sharedMesh", 0),
                skinned,
                nullptr);
        }
    }
    if (visualMesh != nullptr &&
        meshClass != nullptr &&
        il2cpp_array_length != nullptr &&
        il2cpp_array_object_header_size != nullptr)
    {
        auto* vertices = Invoke(
            FindMethod(meshClass, "get_vertices", 0),
            visualMesh,
            nullptr);
        auto* uvs = Invoke(
            FindMethod(meshClass, "get_uv", 0),
            visualMesh,
            nullptr);
        auto* triangles = Invoke(
            FindMethod(meshClass, "get_triangles", 0),
            visualMesh,
            nullptr);
        if (vertices != nullptr && uvs != nullptr && triangles != nullptr)
        {
            const auto vertexCount = il2cpp_array_length(vertices);
            const auto uvCount = il2cpp_array_length(uvs);
            const auto triangleCount = il2cpp_array_length(triangles);
            if (vertexCount > 0 &&
                uvCount >= vertexCount &&
                triangleCount >= 3 &&
                triangleCount <= 6000000)
            {
                result.visualTransform = transform;
                result.vertices = reinterpret_cast<const float*>(
                    reinterpret_cast<char*>(vertices) +
                    il2cpp_array_object_header_size());
                result.uvs = reinterpret_cast<const float*>(
                    reinterpret_cast<char*>(uvs) +
                    il2cpp_array_object_header_size());
                const auto* triangleData = reinterpret_cast<const int*>(
                    reinterpret_cast<char*>(triangles) +
                    il2cpp_array_object_header_size());
                result.triangles.assign(
                    triangleData,
                    triangleData + triangleCount);
                result.vertexCount = static_cast<int>(vertexCount);
            }
        }
    }

    if (meshColliderClass != nullptr && meshClass != nullptr)
    {
        il2cpp_runtime_class_init(meshColliderClass);
        auto* meshColliderType = il2cpp_type_get_object(
            il2cpp_class_get_type(meshColliderClass));
        void* meshColliderParameters[] = { meshColliderType };
        auto* meshCollider = Invoke(
            FindMethod(componentClass, "GetComponent", 1),
            collider,
            meshColliderParameters);
        if (meshCollider != nullptr)
        {
            auto* mesh = Invoke(
                FindMethod(meshColliderClass, "get_sharedMesh", 0),
                meshCollider,
                nullptr);
            if (mesh != nullptr)
            {
                const int subMeshCount = DevMapCapture_ReadInt(Invoke(
                    FindMethod(meshClass, "get_subMeshCount", 0),
                    mesh,
                    nullptr));
                int faceEnd = 0;
                for (int subMesh = 0; subMesh < subMeshCount; ++subMesh)
                {
                    void* triangleParameters[] = { &subMesh };
                    auto* triangles = Invoke(
                        FindMethod(meshClass, "GetTriangles", 1),
                        mesh,
                        triangleParameters);
                    if (triangles != nullptr)
                    {
                        faceEnd += static_cast<int>(
                            il2cpp_array_length(triangles) / 3);
                    }
                    result.faceEnds.push_back(faceEnd);
                }
            }
        }
    }
    return result;
}

bool DevMapCapture_InterpolateVisualUv(
    const DevMapCapturePaint& paint,
    const DevMapRawHit& hit,
    float& u,
    float& v)
{
    if (paint.visualTransform == nullptr ||
        paint.vertices == nullptr ||
        paint.uvs == nullptr ||
        paint.triangles.size() < 3 ||
        paint.vertexCount <= 0)
    {
        return false;
    }

    auto* transformClass = FindClass("UnityEngine", "Transform");
    if (transformClass == nullptr)
    {
        return false;
    }
    float world[3] = { hit.point[0], hit.point[1], hit.point[2] };
    void* localParameters[] = { world };
    auto* localValue = Invoke(
        FindMethod(transformClass, "InverseTransformPoint", 1),
        paint.visualTransform,
        localParameters);
    if (localValue == nullptr || il2cpp_object_unbox == nullptr)
    {
        return false;
    }
    const auto* local = static_cast<float*>(il2cpp_object_unbox(localValue));
    const int triangleCount =
        static_cast<int>(paint.triangles.size() / 3);
    const auto tryTriangle = [&](int faceIndex)
    {
        if (faceIndex < 0 || faceIndex >= triangleCount)
        {
            return false;
        }
        const int a = paint.triangles[faceIndex * 3 + 0];
        const int b = paint.triangles[faceIndex * 3 + 1];
        const int c = paint.triangles[faceIndex * 3 + 2];
        if (a < 0 || b < 0 || c < 0 ||
            a >= paint.vertexCount ||
            b >= paint.vertexCount ||
            c >= paint.vertexCount)
        {
            return false;
        }
        const float* pa = paint.vertices + a * 3;
        const float* pb = paint.vertices + b * 3;
        const float* pc = paint.vertices + c * 3;
        const float v0x = pb[0] - pa[0];
        const float v0z = pb[2] - pa[2];
        const float v1x = pc[0] - pa[0];
        const float v1z = pc[2] - pa[2];
        const float v2x = local[0] - pa[0];
        const float v2z = local[2] - pa[2];
        const float d00 = v0x * v0x + v0z * v0z;
        const float d01 = v0x * v1x + v0z * v1z;
        const float d11 = v1x * v1x + v1z * v1z;
        const float d20 = v2x * v0x + v2z * v0z;
        const float d21 = v2x * v1x + v2z * v1z;
        const float denominator = d00 * d11 - d01 * d01;
        if (std::fabs(denominator) < 1.0e-10f)
        {
            return false;
        }
        const float weightB =
            (d11 * d20 - d01 * d21) / denominator;
        const float weightC =
            (d00 * d21 - d01 * d20) / denominator;
        const float weightA = 1.0f - weightB - weightC;
        constexpr float epsilon = -0.002f;
        if (weightA < epsilon ||
            weightB < epsilon ||
            weightC < epsilon)
        {
            return false;
        }
        const float* uva = paint.uvs + a * 2;
        const float* uvb = paint.uvs + b * 2;
        const float* uvc = paint.uvs + c * 2;
        u = uva[0] * weightA + uvb[0] * weightB + uvc[0] * weightC;
        v = uva[1] * weightA + uvb[1] * weightB + uvc[1] * weightC;
        return true;
    };

    const int declaredFace = static_cast<int>(hit.faceId);
    if (tryTriangle(declaredFace))
    {
        ++g_devMapVisualUvSamples;
        return true;
    }
    constexpr int searchRadius = 24;
    for (int offset = 1; offset <= searchRadius; ++offset)
    {
        if (tryTriangle(declaredFace - offset) ||
            tryTriangle(declaredFace + offset))
        {
            ++g_devMapVisualUvSamples;
            return true;
        }
    }
    return false;
}

const DevMapCaptureMaterial* DevMapCapture_SelectMaterial(
    const DevMapCapturePaint& paint,
    uint32_t faceId)
{
    if (paint.materials.empty())
    {
        return nullptr;
    }
    if (!paint.faceEnds.empty())
    {
        for (size_t index = 0; index < paint.faceEnds.size(); ++index)
        {
            if (static_cast<int>(faceId) < paint.faceEnds[index])
            {
                return &paint.materials[
                    std::min(index, paint.materials.size() - 1)];
            }
        }
    }
    return &paint.materials[0];
}

void DevMapCapture_SampleColor(
    const DevMapCaptureMaterial& material,
    float u,
    float v,
    float normalY,
    uint8_t output[3])
{
    float red = std::clamp(material.baseColor[0], 0.0f, 1.0f);
    float green = std::clamp(material.baseColor[1], 0.0f, 1.0f);
    float blue = std::clamp(material.baseColor[2], 0.0f, 1.0f);
    if (material.hasTexture && material.readableTexture != nullptr)
    {
        u = u * material.uvScale[0] + material.uvOffset[0];
        v = v * material.uvScale[1] + material.uvOffset[1];
        u -= std::floor(u);
        v -= std::floor(v);
        void* sampleParameters[] = {
            &u,
            &v,
        };
        auto* sample = Invoke(
            FindMethod(
                FindClass("UnityEngine", "Texture2D"),
                "GetPixelBilinear",
                2),
            material.readableTexture,
            sampleParameters);
        if (sample != nullptr && il2cpp_object_unbox != nullptr)
        {
            const auto* textureColor =
                static_cast<float*>(il2cpp_object_unbox(sample));
            red *= textureColor[0];
            green *= textureColor[1];
            blue *= textureColor[2];
            ++g_devMapTextureSamples;
        }
    }
    else
    {
        ++g_devMapColorFallbacks;
    }

    const float slopeLight = 0.82f + 0.18f * std::clamp(normalY, 0.0f, 1.0f);
    red = std::clamp(red * slopeLight, 0.0f, 1.0f);
    green = std::clamp(green * slopeLight, 0.0f, 1.0f);
    blue = std::clamp(blue * slopeLight, 0.0f, 1.0f);
    output[0] = static_cast<uint8_t>(std::round(red * 255.0f));
    output[1] = static_cast<uint8_t>(std::round(green * 255.0f));
    output[2] = static_cast<uint8_t>(std::round(blue * 255.0f));
}

Il2CppObject* DevMapCapture_GetCollider(int instanceId)
{
    const auto found = g_devMapColliders.find(instanceId);
    if (found != g_devMapColliders.end())
    {
        return found->second;
    }
    int id = instanceId;
    void* parameters[] = { &id };
    auto* collider = Invoke(
        FindMethod(
            FindClass("UnityEngine", "Object"),
            "FindObjectFromInstanceID",
            1),
        nullptr,
        parameters);
    g_devMapColliders[instanceId] = collider;
    return collider;
}

bool DevMapCapture_CapsuleBlocked(
    float x,
    float z,
    float groundHeight,
    Il2CppObject* groundCollider)
{
    if (g_devMapColliderArray == nullptr)
    {
        return false;
    }
    float point0[] = {
        x,
        groundHeight +
            kDevMapPlayerCapsuleRadius +
            kDevMapPlayerCapsuleGroundClearance,
        z,
    };
    float point1[] = {
        x,
        groundHeight +
            kDevMapPlayerCapsuleHeight -
            kDevMapPlayerCapsuleRadius,
        z,
    };
    float radius = kDevMapPlayerCapsuleRadius;
    int layerMask =
        ~((1 << 12) | (1 << 23) | (1 << 26));
    int queryTriggerInteraction = 1;
    void* parameters[] = {
        point0,
        point1,
        &radius,
        g_devMapColliderArray,
        &layerMask,
        &queryTriggerInteraction,
    };
    const int count = std::clamp(
        DevMapCapture_ReadInt(Invoke(
            FindMethod(
                FindClass("UnityEngine", "Physics"),
                "OverlapCapsuleNonAlloc",
                6),
            nullptr,
            parameters)),
        0,
        kDevMapColliderCapacity);
    ++g_devMapCapsuleQueries;
    if (count <= 0)
    {
        return false;
    }
    auto* colliders = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(g_devMapColliderArray) +
        il2cpp_array_object_header_size());
    for (int index = 0; index < count; ++index)
    {
        auto* collider = colliders[index];
        if (collider == nullptr || collider == groundCollider)
        {
            continue;
        }
        ++g_devMapCapsuleBlocked;
        return true;
    }
    return false;
}

bool DevMapCapture_TryCell(
    int imageX,
    int imageY,
    uint8_t output[3],
    float& height)
{
    auto* colliderClass = FindClass("UnityEngine", "Collider");
    auto* navMeshClass = FindClass("UnityEngine.AI", "NavMesh");
    if (colliderClass == nullptr || navMeshClass == nullptr ||
        g_devMapHitArray == nullptr)
    {
        return false;
    }

    const float x = g_devMapBounds.minX +
        (static_cast<float>(imageX) + 0.5f) /
            static_cast<float>(kDevMapCaptureSize) *
            (g_devMapBounds.maxX - g_devMapBounds.minX);
    const float z = g_devMapBounds.maxZ -
        (static_cast<float>(imageY) + 0.5f) /
            static_cast<float>(kDevMapCaptureSize) *
            (g_devMapBounds.maxZ - g_devMapBounds.minZ);
    float origin[] = { x, g_devMapBounds.maxY + 4.0f, z };
    float direction[] = { 0.0f, -1.0f, 0.0f };
    float maxDistance =
        (g_devMapBounds.maxY - g_devMapBounds.minY) + 8.0f;
    int layerMask = -1;
    int queryTriggerInteraction = 1; // QueryTriggerInteraction.Ignore
    void* raycastParameters[] = {
        origin,
        direction,
        g_devMapHitArray,
        &maxDistance,
        &layerMask,
        &queryTriggerInteraction,
    };
    const int hitCount = DevMapCapture_ReadInt(Invoke(
        FindMethod(FindClass("UnityEngine", "Physics"), "RaycastNonAlloc", 6),
        nullptr,
        raycastParameters));
    ++g_devMapRaycastCount;
    if (hitCount <= 0)
    {
        return false;
    }

    auto* hits = reinterpret_cast<DevMapRawHit*>(
        reinterpret_cast<char*>(g_devMapHitArray) +
        il2cpp_array_object_header_size());
    int selectedHit = -1;
    DevMapNavMeshHit selectedNav{};
    for (int index = 0; index < hitCount; ++index)
    {
        const auto& hit = hits[index];
        if (hit.normal[1] < kDevMapWalkableNormalY)
        {
            continue;
        }
        if (hit.point[1] < g_devMapBounds.minY - 0.20f ||
            hit.point[1] > g_devMapBounds.maxY + 0.20f)
        {
            continue;
        }

        auto* collider = DevMapCapture_GetCollider(hit.colliderId);
        if (collider == nullptr || DevMapCapture_ReadBool(Invoke(
                FindMethod(colliderClass, "get_isTrigger", 0),
                collider,
                nullptr)))
        {
            continue;
        }

        float navSource[] = {
            hit.point[0],
            hit.point[1],
            hit.point[2],
        };
        float navDistance = kDevMapNavMeshSnapDistance;
        int areaMask = -1;
        DevMapNavMeshHit navHit{};
        void* navParameters[] = {
            navSource,
            &navHit,
            &navDistance,
            &areaMask,
        };
        if (!DevMapCapture_ReadBool(Invoke(
                FindMethod(navMeshClass, "SamplePosition", 4),
                nullptr,
                navParameters)))
        {
            continue;
        }
        if (std::fabs(navHit.position[1] - hit.point[1]) >
            kDevMapNavMeshHeightTolerance)
        {
            continue;
        }
        if (navHit.position[1] < g_devMapBounds.minY - 0.20f ||
            navHit.position[1] > g_devMapBounds.maxY + 0.20f)
        {
            continue;
        }
        const float navDeltaX = navHit.position[0] - hit.point[0];
        const float navDeltaZ = navHit.position[2] - hit.point[2];
        if (navDeltaX * navDeltaX + navDeltaZ * navDeltaZ >
            0.12f * 0.12f)
        {
            continue;
        }
        selectedHit = index;
        selectedNav = navHit;
        break;
    }
    if (selectedHit < 0)
    {
        return false;
    }

    const auto& hit = hits[selectedHit];
    auto* selectedCollider = DevMapCapture_GetCollider(hit.colliderId);
    height = selectedNav.position[1];
    if (DevMapCapture_CapsuleBlocked(
            hit.point[0],
            hit.point[2],
            height,
            selectedCollider))
    {
        return false;
    }
    output[0] = 55;
    output[1] = 62;
    output[2] = 64;
    ++g_devMapNavAccepted;
    return true;
}

void DevMapCapture_RasterizeWorldTriangle(
    const float world[3][3],
    const float uv[3][2],
    const DevMapCaptureMaterial& material)
{
    const float edge1[3] = {
        world[1][0] - world[0][0],
        world[1][1] - world[0][1],
        world[1][2] - world[0][2],
    };
    const float edge2[3] = {
        world[2][0] - world[0][0],
        world[2][1] - world[0][1],
        world[2][2] - world[0][2],
    };
    const float normalX =
        edge1[1] * edge2[2] - edge1[2] * edge2[1];
    const float normalY =
        edge1[2] * edge2[0] - edge1[0] * edge2[2];
    const float normalZ =
        edge1[0] * edge2[1] - edge1[1] * edge2[0];
    const float normalLength = std::sqrt(
        normalX * normalX +
        normalY * normalY +
        normalZ * normalZ);
    if (normalLength < 1.0e-8f ||
        std::fabs(normalY) / normalLength < 0.35f)
    {
        return;
    }
    ++g_devMapVisualTrianglesUpward;

    float sourceX[3]{};
    float sourceY[3]{};
    for (int vertex = 0; vertex < 3; ++vertex)
    {
        sourceX[vertex] =
            (world[vertex][0] - g_devMapBounds.minX) /
            (g_devMapBounds.maxX - g_devMapBounds.minX) *
            static_cast<float>(kDevMapCaptureSize - 1);
        sourceY[vertex] =
            (g_devMapBounds.maxZ - world[vertex][2]) /
            (g_devMapBounds.maxZ - g_devMapBounds.minZ) *
            static_cast<float>(kDevMapCaptureSize - 1);
    }
    const double area =
        (static_cast<double>(sourceX[1]) - sourceX[0]) *
            (static_cast<double>(sourceY[2]) - sourceY[0]) -
        (static_cast<double>(sourceX[2]) - sourceX[0]) *
            (static_cast<double>(sourceY[1]) - sourceY[0]);
    if (std::fabs(area) < 1.0e-8)
    {
        return;
    }
    const int minX = std::max(
        0,
        static_cast<int>(std::floor(
            *std::min_element(sourceX, sourceX + 3))));
    const int maxX = std::min(
        kDevMapCaptureSize - 1,
        static_cast<int>(std::ceil(
            *std::max_element(sourceX, sourceX + 3))));
    const int minY = std::max(
        0,
        static_cast<int>(std::floor(
            *std::min_element(sourceY, sourceY + 3))));
    const int maxY = std::min(
        kDevMapCaptureSize - 1,
        static_cast<int>(std::ceil(
            *std::max_element(sourceY, sourceY + 3))));
    for (int imageY = minY; imageY <= maxY; ++imageY)
    {
        for (int imageX = minX; imageX <= maxX; ++imageX)
        {
            const double weightA =
                ((static_cast<double>(sourceX[1]) - imageX) *
                     (static_cast<double>(sourceY[2]) - imageY) -
                 (static_cast<double>(sourceX[2]) - imageX) *
                     (static_cast<double>(sourceY[1]) - imageY)) /
                area;
            const double weightB =
                ((static_cast<double>(sourceX[2]) - imageX) *
                     (static_cast<double>(sourceY[0]) - imageY) -
                 (static_cast<double>(sourceX[0]) - imageX) *
                     (static_cast<double>(sourceY[2]) - imageY)) /
                area;
            const double weightC =
                1.0 - weightA - weightB;
            constexpr double epsilon = -0.00001;
            if (weightA < epsilon ||
                weightB < epsilon ||
                weightC < epsilon)
            {
                continue;
            }
            const size_t index =
                static_cast<size_t>(imageY) *
                    kDevMapCaptureSize +
                static_cast<size_t>(imageX);
            if (!g_devMapWalkable[index])
            {
                continue;
            }
            ++g_devMapVisualTriangleCandidates;
            const float surfaceY = static_cast<float>(
                world[0][1] * weightA +
                world[1][1] * weightB +
                world[2][1] * weightC);
            const float groundY = g_devMapHeights[index];
            if (surfaceY <
                    groundY - kDevMapVisualSurfaceToleranceBelow ||
                surfaceY >
                    groundY + kDevMapVisualSurfaceToleranceAbove)
            {
                ++g_devMapVisualHeightRejected;
                continue;
            }
            const float score = std::fabs(surfaceY - groundY);
            if (score >= g_devMapSurfaceScores[index])
            {
                continue;
            }
            const float sampleU = static_cast<float>(
                uv[0][0] * weightA +
                uv[1][0] * weightB +
                uv[2][0] * weightC);
            const float sampleV = static_cast<float>(
                uv[0][1] * weightA +
                uv[1][1] * weightB +
                uv[2][1] * weightC);
            uint8_t color[3]{};
            float alpha = 1.0f;
            DevMapCapture_SampleMaterialRgb(
                material,
                sampleU,
                sampleV,
                color,
                alpha);
            if (alpha < 0.25f)
            {
                ++g_devMapVisualAlphaRejected;
                continue;
            }
            auto* pixel =
                g_devMapPixels.data() + index * 3;
            pixel[0] = color[0];
            pixel[1] = color[1];
            pixel[2] = color[2];
            g_devMapSurfaceScores[index] = score;
            g_devMapVisualHeights[index] = surfaceY;
            g_devMapVisualValid[index] = 1;
            ++g_devMapVisualPixels;
        }
    }
}

void DevMapCapture_RasterizeRenderer(Il2CppObject* renderer)
{
    static int diagnosticTriangles = 0;
    auto* rendererClass = FindClass("UnityEngine", "Renderer");
    auto* componentClass = FindClass("UnityEngine", "Component");
    auto* meshFilterClass = FindClass("UnityEngine", "MeshFilter");
    auto* meshClass = FindClass("UnityEngine", "Mesh");
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    if (renderer == nullptr ||
        rendererClass == nullptr ||
        componentClass == nullptr ||
        meshFilterClass == nullptr ||
        meshClass == nullptr ||
        gameObjectClass == nullptr)
    {
        return;
    }
    ++g_devMapVisualRenderersVisited;
    if (!DevMapCapture_ReadBool(Invoke(
            FindMethod(rendererClass, "get_enabled", 0),
            renderer,
            nullptr)))
    {
        return;
    }

    auto* rendererObject = GetComponentGameObject(renderer);
    if (rendererObject == nullptr)
    {
        return;
    }
    const int layer = DevMapCapture_ReadInt(Invoke(
        FindMethod(gameObjectClass, "get_layer", 0),
        rendererObject,
        nullptr));
    if (layer != 0 && layer != 20)
    {
        return;
    }
    const std::wstring objectName = GetObjectName(rendererObject);
    if (objectName.rfind(L"EFM_", 0) == 0 ||
        objectName.find(L"Manustia") != std::wstring::npos ||
        objectName.find(L"NPC") != std::wstring::npos)
    {
        return;
    }

    il2cpp_runtime_class_init(meshFilterClass);
    auto* meshFilterType = il2cpp_type_get_object(
        il2cpp_class_get_type(meshFilterClass));
    void* meshFilterParameters[] = { meshFilterType };
    auto* meshFilter = Invoke(
        FindMethod(componentClass, "GetComponent", 1),
        renderer,
        meshFilterParameters);
    if (meshFilter == nullptr)
    {
        return;
    }
    auto* mesh = Invoke(
        FindMethod(meshFilterClass, "get_sharedMesh", 0),
        meshFilter,
        nullptr);
    if (mesh == nullptr)
    {
        return;
    }
    if (!DevMapCapture_ReadBool(Invoke(
            FindMethod(meshClass, "get_isReadable", 0),
            mesh,
            nullptr)))
    {
        ++g_devMapVisualSkippedUnreadable;
        return;
    }

    auto* vertices = Invoke(
        FindMethod(meshClass, "get_vertices", 0),
        mesh,
        nullptr);
    auto* uvs = Invoke(
        FindMethod(meshClass, "get_uv", 0),
        mesh,
        nullptr);
    auto* materials = Invoke(
        FindMethod(rendererClass, "get_sharedMaterials", 0),
        renderer,
        nullptr);
    if (vertices == nullptr || materials == nullptr)
    {
        return;
    }
    const uint32_t vertexCount = il2cpp_array_length(vertices);
    const uint32_t uvCount =
        uvs != nullptr ? il2cpp_array_length(uvs) : 0;
    const uint32_t materialCount = il2cpp_array_length(materials);
    if (vertexCount == 0 || materialCount == 0)
    {
        return;
    }

    auto* transform = Invoke(
        FindMethod(componentClass, "get_transform", 0),
        renderer,
        nullptr);
    auto* matrixValue = transform != nullptr
        ? Invoke(
            FindMethod(
                FindClass("UnityEngine", "Transform"),
                "get_localToWorldMatrix",
                0),
            transform,
            nullptr)
        : nullptr;
    if (matrixValue == nullptr || il2cpp_object_unbox == nullptr)
    {
        return;
    }
    const auto* matrix =
        static_cast<const float*>(il2cpp_object_unbox(matrixValue));
    const auto* localVertices = reinterpret_cast<const float*>(
        reinterpret_cast<const char*>(vertices) +
        il2cpp_array_object_header_size());
    const auto* localUvs = uvs != nullptr
        ? reinterpret_cast<const float*>(
            reinterpret_cast<const char*>(uvs) +
            il2cpp_array_object_header_size())
        : nullptr;
    auto* materialElements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(materials) +
        il2cpp_array_object_header_size());
    const int subMeshCount = std::max(
        1,
        DevMapCapture_ReadInt(Invoke(
            FindMethod(meshClass, "get_subMeshCount", 0),
            mesh,
            nullptr)));
    for (int subMesh = 0;
         subMesh < subMeshCount;
         ++subMesh)
    {
        void* triangleParameters[] = { &subMesh };
        auto* triangles = Invoke(
            FindMethod(meshClass, "GetTriangles", 1),
            mesh,
            triangleParameters);
        if (triangles == nullptr)
        {
            continue;
        }
        const uint32_t triangleIndexCount =
            il2cpp_array_length(triangles);
        if (triangleIndexCount < 3)
        {
            continue;
        }
        const auto* triangleData = reinterpret_cast<const int*>(
            reinterpret_cast<const char*>(triangles) +
            il2cpp_array_object_header_size());
        const auto materialIndex = static_cast<uint32_t>(
            std::min<int>(
                subMesh,
                static_cast<int>(materialCount) - 1));
        const auto material = DevMapCapture_BuildMaterial(
            materialElements[materialIndex]);
        if (material.material == nullptr)
        {
            continue;
        }
        for (uint32_t triangle = 0;
             triangle + 2 < triangleIndexCount;
             triangle += 3)
        {
            const int indices[3] = {
                triangleData[triangle + 0],
                triangleData[triangle + 1],
                triangleData[triangle + 2],
            };
            if (indices[0] < 0 ||
                indices[1] < 0 ||
                indices[2] < 0 ||
                static_cast<uint32_t>(indices[0]) >= vertexCount ||
                static_cast<uint32_t>(indices[1]) >= vertexCount ||
                static_cast<uint32_t>(indices[2]) >= vertexCount)
            {
                continue;
            }
            float world[3][3]{};
            float triangleUv[3][2]{};
            for (int vertex = 0; vertex < 3; ++vertex)
            {
                const int index = indices[vertex];
                const float localX = localVertices[index * 3 + 0];
                const float localY = localVertices[index * 3 + 1];
                const float localZ = localVertices[index * 3 + 2];
                world[vertex][0] =
                    matrix[0] * localX +
                    matrix[4] * localY +
                    matrix[8] * localZ +
                    matrix[12];
                world[vertex][1] =
                    matrix[1] * localX +
                    matrix[5] * localY +
                    matrix[9] * localZ +
                    matrix[13];
                world[vertex][2] =
                    matrix[2] * localX +
                    matrix[6] * localY +
                    matrix[10] * localZ +
                    matrix[14];
                if (localUvs != nullptr &&
                    static_cast<uint32_t>(index) < uvCount)
                {
                    triangleUv[vertex][0] =
                        localUvs[index * 2 + 0];
                    triangleUv[vertex][1] =
                        localUvs[index * 2 + 1];
                }
            }
            if (diagnosticTriangles < 16)
            {
                const float centerX =
                    (world[0][0] + world[1][0] + world[2][0]) / 3.0f;
                const float centerY =
                    (world[0][1] + world[1][1] + world[2][1]) / 3.0f;
                const float centerZ =
                    (world[0][2] + world[1][2] + world[2][2]) / 3.0f;
                const int centerPixelX = static_cast<int>(std::clamp(
                    (centerX - g_devMapBounds.minX) /
                        (g_devMapBounds.maxX - g_devMapBounds.minX) *
                        kDevMapCaptureSize,
                    0.0f,
                    static_cast<float>(kDevMapCaptureSize - 1)));
                const int centerPixelY = static_cast<int>(std::clamp(
                    (g_devMapBounds.maxZ - centerZ) /
                        (g_devMapBounds.maxZ - g_devMapBounds.minZ) *
                        kDevMapCaptureSize,
                    0.0f,
                    static_cast<float>(kDevMapCaptureSize - 1)));
                const size_t centerIndex =
                    static_cast<size_t>(centerPixelY) *
                        kDevMapCaptureSize +
                    static_cast<size_t>(centerPixelX);
                LogInfo(LogCategory_Scene, L"Scene", 
                    L"Visual triangle sample[%d]: renderer=%s world=(%.2f,%.2f,%.2f) pixel=(%d,%d) walkable=%d groundY=%.2f.",
                    diagnosticTriangles + 1,
                    objectName.c_str(),
                    centerX,
                    centerY,
                    centerZ,
                    centerPixelX,
                    centerPixelY,
                    g_devMapWalkable[centerIndex] ? 1 : 0,
                    g_devMapHeights[centerIndex]);
                ++diagnosticTriangles;
            }
            ++g_devMapVisualTriangles;
            DevMapCapture_RasterizeWorldTriangle(
                world,
                triangleUv,
                material);
        }
    }
}

bool DevMapCapture_BeginVisualRasterization()
{
    auto* objectClass = FindClass("UnityEngine", "Object");
    auto* rendererClass = FindClass("UnityEngine", "Renderer");
    if (objectClass == nullptr || rendererClass == nullptr)
    {
        return false;
    }
    il2cpp_runtime_class_init(rendererClass);
    auto* rendererType = il2cpp_type_get_object(
        il2cpp_class_get_type(rendererClass));
    bool includeInactive = false;
    void* parameters[] = { rendererType, &includeInactive };
    g_devMapRendererArray = Invoke(
        FindMethod(objectClass, "FindObjectsOfType", 2),
        nullptr,
        parameters);
    if (g_devMapRendererArray == nullptr)
    {
        return false;
    }
    if (il2cpp_gchandle_new != nullptr)
    {
        g_devMapRendererArrayHandle =
            il2cpp_gchandle_new(g_devMapRendererArray, false);
    }
    g_devMapRendererCount = static_cast<int>(
        il2cpp_array_length(g_devMapRendererArray));
    g_devMapRendererCursor = 0;
    const size_t cellCount =
        static_cast<size_t>(kDevMapCaptureSize) *
        kDevMapCaptureSize;
    g_devMapSurfaceScores.assign(cellCount, 1.0e20f);
    g_devMapVisualHeights.assign(cellCount, 0.0f);
    g_devMapVisualValid.assign(cellCount, 0);
    LogInfo(LogCategory_Scene, L"Scene", 
        L"Colored Park visual rasterization started: renderers=%d.",
        g_devMapRendererCount);
    return true;
}

void DevMapCapture_ProcessVisualRenderers()
{
    if (g_devMapRendererArray == nullptr)
    {
        return;
    }
    auto* elements = reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(g_devMapRendererArray) +
        il2cpp_array_object_header_size());
    const int end = std::min(
        g_devMapRendererCount,
        g_devMapRendererCursor + kDevMapRenderersPerFrame);
    for (; g_devMapRendererCursor < end; ++g_devMapRendererCursor)
    {
        DevMapCapture_RasterizeRenderer(
            elements[g_devMapRendererCursor]);
    }
}

bool DevMapCapture_WriteBmp(const std::wstring& path)
{
    const DWORD rowBytes =
        static_cast<DWORD>(kDevMapCaptureSize * 3);
    const DWORD pixelBytes =
        rowBytes * static_cast<DWORD>(kDevMapCaptureSize);
    BITMAPFILEHEADER fileHeader{};
    BITMAPINFOHEADER infoHeader{};
    fileHeader.bfType = 0x4D42;
    fileHeader.bfOffBits =
        sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    fileHeader.bfSize = fileHeader.bfOffBits + pixelBytes;
    infoHeader.biSize = sizeof(BITMAPINFOHEADER);
    infoHeader.biWidth = kDevMapCaptureSize;
    infoHeader.biHeight = kDevMapCaptureSize;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24;
    infoHeader.biCompression = BI_RGB;
    infoHeader.biSizeImage = pixelBytes;

    HANDLE file = CreateFileW(
        path.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    DWORD written = 0;
    bool ok =
        WriteFile(file, &fileHeader, sizeof(fileHeader), &written, nullptr) &&
        written == sizeof(fileHeader) &&
        WriteFile(file, &infoHeader, sizeof(infoHeader), &written, nullptr) &&
        written == sizeof(infoHeader);
    if (ok)
    {
        for (int imageY = kDevMapCaptureSize - 1;
             imageY >= 0 && ok;
             --imageY)
        {
            const auto* row =
                g_devMapPixels.data() +
                static_cast<size_t>(imageY) *
                    kDevMapCaptureSize * 3;
            static std::vector<uint8_t> bgrRow(rowBytes);
            for (int imageX = 0; imageX < kDevMapCaptureSize; ++imageX)
            {
                bgrRow[static_cast<size_t>(imageX) * 3 + 0] =
                    row[static_cast<size_t>(imageX) * 3 + 2];
                bgrRow[static_cast<size_t>(imageX) * 3 + 1] =
                    row[static_cast<size_t>(imageX) * 3 + 1];
                bgrRow[static_cast<size_t>(imageX) * 3 + 2] =
                    row[static_cast<size_t>(imageX) * 3 + 0];
            }
            ok = WriteFile(
                     file,
                     bgrRow.data(),
                     rowBytes,
                     &written,
                     nullptr) != FALSE &&
                 written == rowBytes;
        }
    }
    CloseHandle(file);
    return ok;
}

bool DevMapCapture_CaptureIsolatedTerrain()
{
    auto* rendererClass = FindClass("UnityEngine", "Renderer");
    auto* objectClass = FindClass("UnityEngine", "Object");
    auto* terrainClass = FindClass("UnityEngine", "Terrain");
    auto* cameraClass = FindClass("UnityEngine", "Camera");
    auto* textureClass = FindClass("UnityEngine", "Texture2D");
    auto* renderTextureClass = FindClass("UnityEngine", "RenderTexture");
    auto* imageConversionClass =
        FindClass("UnityEngine", "ImageConversion");
    if (rendererClass == nullptr ||
        objectClass == nullptr ||
        terrainClass == nullptr ||
        cameraClass == nullptr ||
        textureClass == nullptr ||
        renderTextureClass == nullptr ||
        imageConversionClass == nullptr)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"Colored map terrain capture could not resolve Unity classes.");
        return false;
    }

    auto* terrains = Invoke(
        FindMethod(terrainClass, "get_activeTerrains", 0),
        nullptr,
        nullptr);
    if (terrains == nullptr || il2cpp_array_length(terrains) == 0)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"Colored map terrain capture found no active Terrain.");
        return false;
    }
    auto* terrain = *reinterpret_cast<Il2CppObject**>(
        reinterpret_cast<char*>(terrains) +
        il2cpp_array_object_header_size());
    auto* terrainGameObject = GetComponentGameObject(terrain);
    auto* gameObjectClass = FindClass("UnityEngine", "GameObject");
    int terrainLayer = DevMapCapture_ReadInt(Invoke(
        FindMethod(gameObjectClass, "get_layer", 0),
        terrainGameObject,
        nullptr));

    il2cpp_runtime_class_init(rendererClass);
    auto* rendererType = il2cpp_type_get_object(
        il2cpp_class_get_type(rendererClass));
    bool includeInactive = true;
    void* findParameters[] = { rendererType, &includeInactive };
    auto* renderers = Invoke(
        FindMethod(objectClass, "FindObjectsOfType", 2),
        nullptr,
        findParameters);
    std::vector<std::pair<Il2CppObject*, bool>> rendererStates;
    std::string rendererInventory =
        "name,layer,enabled,centerX,centerY,centerZ,extentX,extentY,extentZ\r\n";
    if (renderers != nullptr)
    {
        const auto count = il2cpp_array_length(renderers);
        auto* elements = reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(renderers) +
            il2cpp_array_object_header_size());
        rendererStates.reserve(count);
        for (uint32_t index = 0; index < count; ++index)
        {
            auto* renderer = elements[index];
            if (renderer == nullptr)
            {
                continue;
            }
            const bool enabled = DevMapCapture_ReadBool(Invoke(
                FindMethod(rendererClass, "get_enabled", 0),
                renderer,
                nullptr));
            rendererStates.emplace_back(renderer, enabled);
            auto* rendererGameObject = GetComponentGameObject(renderer);
            const int layer = DevMapCapture_ReadInt(Invoke(
                FindMethod(gameObjectClass, "get_layer", 0),
                rendererGameObject,
                nullptr));
            auto* boundsValue = Invoke(
                FindMethod(rendererClass, "get_bounds", 0),
                renderer,
                nullptr);
            float centerX = 0.0f;
            float centerY = 0.0f;
            float centerZ = 0.0f;
            float extentX = 0.0f;
            float extentY = 0.0f;
            float extentZ = 0.0f;
            if (boundsValue != nullptr && il2cpp_object_unbox != nullptr)
            {
                const auto* bounds =
                    static_cast<float*>(il2cpp_object_unbox(boundsValue));
                centerX = bounds[0];
                centerY = bounds[1];
                centerZ = bounds[2];
                extentX = bounds[3];
                extentY = bounds[4];
                extentZ = bounds[5];
            }
            char line[1024]{};
            std::snprintf(
                line,
                sizeof(line),
                "%s,%d,%d,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\r\n",
                WideToUtf8(GetObjectName(rendererGameObject)).c_str(),
                layer,
                enabled ? 1 : 0,
                centerX,
                centerY,
                centerZ,
                extentX,
                extentY,
                extentZ);
            rendererInventory += line;
            bool disabled = false;
            void* enabledParameters[] = { &disabled };
            Invoke(
                FindMethod(rendererClass, "set_enabled", 1),
                renderer,
                enabledParameters);
        }
    }
    const std::wstring inventoryPath =
        GetEscapeFromManakaSubdirectory(L"MapsDev") +
        L"\\park_renderer_inventory.csv";
    HANDLE inventoryFile = CreateFileW(
        inventoryPath.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (inventoryFile != INVALID_HANDLE_VALUE)
    {
        DWORD written = 0;
        WriteFile(
            inventoryFile,
            rendererInventory.data(),
            static_cast<DWORD>(rendererInventory.size()),
            &written,
            nullptr);
        CloseHandle(inventoryFile);
    }

    const bool ownsCamera =
        !ExtendedMode_IsUnityObjectAlive(g_extendedCamera);
    auto* cameraObject = ownsCamera
        ? CreateWorldGameObject("EFM_DevTerrainCaptureCamera")
        : GetComponentGameObject(g_extendedCamera);
    auto* camera = ownsCamera && cameraObject != nullptr
        ? AddSceneComponentToGameObject(
            cameraObject,
            "UnityEngine",
            "Camera")
        : g_extendedCamera;
    float previousCameraPosition[3]{};
    float previousCameraRotation[4]{0.0f, 0.0f, 0.0f, 1.0f};
    int previousClearFlags = 1;
    int previousCullingMask = -1;
    int previousCameraDepth = 0;
    float previousBackgroundColor[4]{0.0f, 0.0f, 0.0f, 1.0f};
    float previousNearClip = 0.3f;
    float previousFarClip = 1000.0f;
    bool previousOrthographic = false;
    float previousOrthographicSize = 5.0f;
    float previousAspect = 1.0f;
    Il2CppObject* previousTargetTexture = nullptr;
    if (camera != nullptr && !ownsCamera)
    {
        auto* transform = GetWorldTransform(cameraObject);
        auto* positionValue = Invoke(
            FindMethod(FindClass("UnityEngine", "Transform"), "get_position", 0),
            transform,
            nullptr);
        auto* rotationValue = Invoke(
            FindMethod(FindClass("UnityEngine", "Transform"), "get_rotation", 0),
            transform,
            nullptr);
        if (positionValue != nullptr && il2cpp_object_unbox != nullptr)
        {
            std::copy(
                static_cast<float*>(il2cpp_object_unbox(positionValue)),
                static_cast<float*>(il2cpp_object_unbox(positionValue)) + 3,
                previousCameraPosition);
        }
        if (rotationValue != nullptr && il2cpp_object_unbox != nullptr)
        {
            std::copy(
                static_cast<float*>(il2cpp_object_unbox(rotationValue)),
                static_cast<float*>(il2cpp_object_unbox(rotationValue)) + 4,
                previousCameraRotation);
        }
        previousClearFlags = DevMapCapture_ReadInt(Invoke(
            FindMethod(cameraClass, "get_clearFlags", 0),
            camera,
            nullptr));
        previousCullingMask = DevMapCapture_ReadInt(Invoke(
            FindMethod(cameraClass, "get_cullingMask", 0),
            camera,
            nullptr));
        previousCameraDepth = DevMapCapture_ReadInt(Invoke(
            FindMethod(cameraClass, "get_depth", 0),
            camera,
            nullptr));
        previousNearClip = DevMapCapture_ReadFloat(Invoke(
            FindMethod(cameraClass, "get_nearClipPlane", 0),
            camera,
            nullptr));
        previousFarClip = DevMapCapture_ReadFloat(Invoke(
            FindMethod(cameraClass, "get_farClipPlane", 0),
            camera,
            nullptr));
        previousOrthographic = DevMapCapture_ReadBool(Invoke(
            FindMethod(cameraClass, "get_orthographic", 0),
            camera,
            nullptr));
        previousOrthographicSize = DevMapCapture_ReadFloat(Invoke(
            FindMethod(cameraClass, "get_orthographicSize", 0),
            camera,
            nullptr));
        previousAspect = DevMapCapture_ReadFloat(Invoke(
            FindMethod(cameraClass, "get_aspect", 0),
            camera,
            nullptr));
        auto* backgroundValue = Invoke(
            FindMethod(cameraClass, "get_backgroundColor", 0),
            camera,
            nullptr);
        if (backgroundValue != nullptr && il2cpp_object_unbox != nullptr)
        {
            std::copy(
                static_cast<float*>(il2cpp_object_unbox(backgroundValue)),
                static_cast<float*>(il2cpp_object_unbox(backgroundValue)) + 4,
                previousBackgroundColor);
        }
        previousTargetTexture = Invoke(
            FindMethod(cameraClass, "get_targetTexture", 0),
            camera,
            nullptr);
    }
    auto* renderTexture = il2cpp_object_new(renderTextureClass);
    int size = kDevMapCaptureSize;
    int depth = 24;
    void* renderTextureParameters[] = { &size, &size, &depth };
    Invoke(
        FindMethod(renderTextureClass, ".ctor", 3),
        renderTexture,
        renderTextureParameters);
    Invoke(
        FindMethod(renderTextureClass, "Create", 0),
        renderTexture,
        nullptr);

    bool captured = false;
    if (camera != nullptr && renderTexture != nullptr)
    {
        auto* transform = GetWorldTransform(cameraObject);
        if (!ownsCamera)
        {
            bool cameraEnabled = true;
            void* enabledParameters[] = { &cameraEnabled };
            Invoke(
                FindMethod(FindClass("UnityEngine", "Behaviour"), "set_enabled", 1),
                camera,
                enabledParameters);
        }
        ExtendedMode_SetTransformPosition(
            transform,
            (g_devMapBounds.minX + g_devMapBounds.maxX) * 0.5f,
            g_devMapBounds.maxY + 50.0f,
            (g_devMapBounds.minZ + g_devMapBounds.maxZ) * 0.5f);
        SetTransformEuler(transform, 90.0f, 0.0f, 0.0f);

        int clearFlags = 2; // CameraClearFlags.SolidColor
        float backgroundColor[] = { 0.93f, 0.12f, 0.86f, 1.0f };
        float nearClip = 0.1f;
        float farClip = 200.0f;
        bool orthographic = true;
        float orthographicSize =
            (g_devMapBounds.maxZ - g_devMapBounds.minZ) * 0.5f;
        float aspect = 1.0f;
        // Normal renderers are disabled for this one-frame isolation pass.
        // Keep every layer visible so Unity's Terrain subsystem and any terrain
        // detail renderers remain eligible.
        int cullingMask = -1;
        int cameraDepth = -100;
        void* clearParameters[] = { &clearFlags };
        void* colorParameters[] = { backgroundColor };
        void* nearParameters[] = { &nearClip };
        void* farParameters[] = { &farClip };
        void* orthoParameters[] = { &orthographic };
        void* sizeParameters[] = { &orthographicSize };
        void* aspectParameters[] = { &aspect };
        void* maskParameters[] = { &cullingMask };
        void* depthParameters[] = { &cameraDepth };
        void* targetParameters[] = { renderTexture };
        Invoke(
            FindMethod(cameraClass, "set_clearFlags", 1),
            camera,
            clearParameters);
        Invoke(
            FindMethod(cameraClass, "set_backgroundColor", 1),
            camera,
            colorParameters);
        Invoke(
            FindMethod(cameraClass, "set_nearClipPlane", 1),
            camera,
            nearParameters);
        Invoke(
            FindMethod(cameraClass, "set_farClipPlane", 1),
            camera,
            farParameters);
        Invoke(
            FindMethod(cameraClass, "set_orthographic", 1),
            camera,
            orthoParameters);
        Invoke(
            FindMethod(cameraClass, "set_orthographicSize", 1),
            camera,
            sizeParameters);
        Invoke(
            FindMethod(cameraClass, "set_aspect", 1),
            camera,
            aspectParameters);
        Invoke(
            FindMethod(cameraClass, "set_cullingMask", 1),
            camera,
            maskParameters);
        Invoke(
            FindMethod(cameraClass, "set_depth", 1),
            camera,
            depthParameters);
        Invoke(
            FindMethod(cameraClass, "set_targetTexture", 1),
            camera,
            targetParameters);
        Invoke(
            FindMethod(cameraClass, "Render", 0),
            camera,
            nullptr);

        auto* previousActive = Invoke(
            FindMethod(renderTextureClass, "get_active", 0),
            nullptr,
            nullptr);
        auto* setActive = FindMethod(
            renderTextureClass,
            "set_active",
            1);
        void* activeParameters[] = { renderTexture };
        Invoke(setActive, nullptr, activeParameters);
        auto* texture = il2cpp_object_new(textureClass);
        int textureFormat = 4; // TextureFormat.RGBA32
        bool mipChain = false;
        void* textureParameters[] = {
            &size,
            &size,
            &textureFormat,
            &mipChain,
        };
        Invoke(
            FindMethod(textureClass, ".ctor", 4),
            texture,
            textureParameters);
        float rect[] = {
            0.0f,
            0.0f,
            static_cast<float>(size),
            static_cast<float>(size),
        };
        int destX = 0;
        int destY = 0;
        bool recalculateMipMaps = false;
        void* readParameters[] = {
            rect,
            &destX,
            &destY,
            &recalculateMipMaps,
        };
        Invoke(
            FindMethod(textureClass, "ReadPixels", 4),
            texture,
            readParameters);
        Invoke(FindMethod(textureClass, "Apply", 0), texture, nullptr);
        void* restoreParameters[] = { previousActive };
        Invoke(setActive, nullptr, restoreParameters);

        auto* png = Invoke(
            FindMethod(imageConversionClass, "EncodeToPNG", 1),
            nullptr,
            (void*[]) { texture });
        if (png != nullptr)
        {
            const std::wstring directory =
                GetEscapeFromManakaSubdirectory(L"MapsDev");
            CreateDirectoryW(directory.c_str(), nullptr);
            const std::wstring path =
                directory + L"\\park_terrain_isolated_2560.png";
            const DWORD length = il2cpp_array_length(png);
            auto* bytes = reinterpret_cast<unsigned char*>(
                reinterpret_cast<char*>(png) +
                il2cpp_array_object_header_size());
            HANDLE file = CreateFileW(
                path.c_str(),
                GENERIC_WRITE,
                0,
                nullptr,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                nullptr);
            if (file != INVALID_HANDLE_VALUE)
            {
                DWORD written = 0;
                captured = WriteFile(
                    file,
                    bytes,
                    length,
                    &written,
                    nullptr) != FALSE &&
                    written == length;
                CloseHandle(file);
            }
        }
        if (texture != nullptr)
        {
            void* destroyParameters[] = { texture };
            Invoke(
                FindMethod(objectClass, "Destroy", 1),
                nullptr,
                destroyParameters);
        }
        if (!ownsCamera)
        {
            void* positionParameters[] = { previousCameraPosition };
            Invoke(
                FindMethod(
                    FindClass("UnityEngine", "Transform"),
                    "set_position",
                    1),
                transform,
                positionParameters);
            void* rotationParameters[] = { previousCameraRotation };
            Invoke(
                FindMethod(
                    FindClass("UnityEngine", "Transform"),
                    "set_rotation",
                    1),
                transform,
                rotationParameters);
            void* restoreIntParameters[] = { &previousClearFlags };
            Invoke(
                FindMethod(cameraClass, "set_clearFlags", 1),
                camera,
                restoreIntParameters);
            restoreIntParameters[0] = &previousCullingMask;
            Invoke(
                FindMethod(cameraClass, "set_cullingMask", 1),
                camera,
                restoreIntParameters);
            restoreIntParameters[0] = &previousCameraDepth;
            Invoke(
                FindMethod(cameraClass, "set_depth", 1),
                camera,
                restoreIntParameters);
            void* restoreFloatParameters[] = { &previousNearClip };
            Invoke(
                FindMethod(cameraClass, "set_nearClipPlane", 1),
                camera,
                restoreFloatParameters);
            restoreFloatParameters[0] = &previousFarClip;
            Invoke(
                FindMethod(cameraClass, "set_farClipPlane", 1),
                camera,
                restoreFloatParameters);
            restoreFloatParameters[0] = &previousOrthographicSize;
            Invoke(
                FindMethod(cameraClass, "set_orthographicSize", 1),
                camera,
                restoreFloatParameters);
            restoreFloatParameters[0] = &previousAspect;
            Invoke(
                FindMethod(cameraClass, "set_aspect", 1),
                camera,
                restoreFloatParameters);
            void* restoreBoolParameters[] = { &previousOrthographic };
            Invoke(
                FindMethod(cameraClass, "set_orthographic", 1),
                camera,
                restoreBoolParameters);
            void* restoreColorParameters[] = { previousBackgroundColor };
            Invoke(
                FindMethod(cameraClass, "set_backgroundColor", 1),
                camera,
                restoreColorParameters);
            void* restoreTargetParameters[] = { previousTargetTexture };
            Invoke(
                FindMethod(cameraClass, "set_targetTexture", 1),
                camera,
                restoreTargetParameters);
        }
    }

    if (renderTexture != nullptr)
    {
        void* destroyParameters[] = { renderTexture };
        Invoke(
            FindMethod(objectClass, "Destroy", 1),
            nullptr,
            destroyParameters);
    }
    if (ownsCamera && cameraObject != nullptr)
    {
        void* destroyParameters[] = { cameraObject };
        Invoke(
            FindMethod(objectClass, "Destroy", 1),
            nullptr,
            destroyParameters);
    }
    for (const auto& state : rendererStates)
    {
        bool enabled = state.second;
        void* enabledParameters[] = { &enabled };
        Invoke(
            FindMethod(rendererClass, "set_enabled", 1),
            state.first,
            enabledParameters);
    }
    LogInfo(LogCategory_Scene, L"Scene", 
        L"Colored map isolated Terrain capture: terrainLayer=%d restoredRenderers=%u success=%d.",
        terrainLayer,
        static_cast<unsigned int>(rendererStates.size()),
        captured ? 1 : 0);
    return captured;
}

bool DevMapCapture_BeginIsolatedTerrainRender()
{
    auto* rendererClass = FindClass("UnityEngine", "Renderer");
    auto* objectClass = FindClass("UnityEngine", "Object");
    auto* cameraClass = FindClass("UnityEngine", "Camera");
    auto* renderTextureClass = FindClass("UnityEngine", "RenderTexture");
    if (rendererClass == nullptr ||
        objectClass == nullptr ||
        cameraClass == nullptr ||
        renderTextureClass == nullptr)
    {
        return false;
    }

    il2cpp_runtime_class_init(rendererClass);
    auto* rendererType = il2cpp_type_get_object(
        il2cpp_class_get_type(rendererClass));
    bool includeInactive = true;
    void* findParameters[] = { rendererType, &includeInactive };
    auto* renderers = Invoke(
        FindMethod(objectClass, "FindObjectsOfType", 2),
        nullptr,
        findParameters);
    g_devMapTerrainRendererStates.clear();
    if (renderers != nullptr)
    {
        const auto count = il2cpp_array_length(renderers);
        auto* elements = reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<char*>(renderers) +
            il2cpp_array_object_header_size());
        g_devMapTerrainRendererStates.reserve(count);
        for (uint32_t index = 0; index < count; ++index)
        {
            auto* renderer = elements[index];
            if (renderer == nullptr)
            {
                continue;
            }
            const bool enabled = DevMapCapture_ReadBool(Invoke(
                FindMethod(rendererClass, "get_enabled", 0),
                renderer,
                nullptr));
            g_devMapTerrainRendererStates.emplace_back(renderer, enabled);
            bool disabled = false;
            void* enabledParameters[] = { &disabled };
            Invoke(
                FindMethod(rendererClass, "set_enabled", 1),
                renderer,
                enabledParameters);
        }
    }

    g_devMapTerrainCameraObject =
        CreateWorldGameObject("EFM_DevTerrainCaptureCamera");
    SetWorldGameObjectActive(g_devMapTerrainCameraObject, true);
    g_devMapTerrainCamera = AddSceneComponentToGameObject(
        g_devMapTerrainCameraObject,
        "UnityEngine",
        "Camera");
    g_devMapTerrainRenderTexture =
        il2cpp_object_new(renderTextureClass);
    int size = kDevMapCaptureSize;
    int depth = 24;
    void* renderTextureParameters[] = { &size, &size, &depth };
    Invoke(
        FindMethod(renderTextureClass, ".ctor", 3),
        g_devMapTerrainRenderTexture,
        renderTextureParameters);
    Invoke(
        FindMethod(renderTextureClass, "Create", 0),
        g_devMapTerrainRenderTexture,
        nullptr);
    g_devMapTerrainPreviousActiveTexture = Invoke(
        FindMethod(renderTextureClass, "get_active", 0),
        nullptr,
        nullptr);

    if (g_devMapTerrainCamera == nullptr ||
        g_devMapTerrainRenderTexture == nullptr)
    {
        return false;
    }
    auto* transform = GetWorldTransform(g_devMapTerrainCameraObject);
    ExtendedMode_SetTransformPosition(
        transform,
        (g_devMapBounds.minX + g_devMapBounds.maxX) * 0.5f,
        g_devMapBounds.maxY + 50.0f,
        (g_devMapBounds.minZ + g_devMapBounds.maxZ) * 0.5f);
    SetTransformEuler(transform, 90.0f, 0.0f, 0.0f);

    int clearFlags = 2;
    float backgroundColor[] = { 0.93f, 0.12f, 0.86f, 1.0f };
    float nearClip = 0.1f;
    float farClip = 200.0f;
    bool orthographic = true;
    float orthographicSize =
        (g_devMapBounds.maxZ - g_devMapBounds.minZ) * 0.5f;
    float aspect = 1.0f;
    int cullingMask = -1;
    int cameraDepth = -100;
    void* clearParameters[] = { &clearFlags };
    void* colorParameters[] = { backgroundColor };
    void* nearParameters[] = { &nearClip };
    void* farParameters[] = { &farClip };
    void* orthoParameters[] = { &orthographic };
    void* sizeParameters[] = { &orthographicSize };
    void* aspectParameters[] = { &aspect };
    void* maskParameters[] = { &cullingMask };
    void* depthParameters[] = { &cameraDepth };
    void* targetParameters[] = { g_devMapTerrainRenderTexture };
    Invoke(
        FindMethod(cameraClass, "set_clearFlags", 1),
        g_devMapTerrainCamera,
        clearParameters);
    Invoke(
        FindMethod(cameraClass, "set_backgroundColor", 1),
        g_devMapTerrainCamera,
        colorParameters);
    Invoke(
        FindMethod(cameraClass, "set_nearClipPlane", 1),
        g_devMapTerrainCamera,
        nearParameters);
    Invoke(
        FindMethod(cameraClass, "set_farClipPlane", 1),
        g_devMapTerrainCamera,
        farParameters);
    Invoke(
        FindMethod(cameraClass, "set_orthographic", 1),
        g_devMapTerrainCamera,
        orthoParameters);
    Invoke(
        FindMethod(cameraClass, "set_orthographicSize", 1),
        g_devMapTerrainCamera,
        sizeParameters);
    Invoke(
        FindMethod(cameraClass, "set_aspect", 1),
        g_devMapTerrainCamera,
        aspectParameters);
    Invoke(
        FindMethod(cameraClass, "set_cullingMask", 1),
        g_devMapTerrainCamera,
        maskParameters);
    Invoke(
        FindMethod(cameraClass, "set_depth", 1),
        g_devMapTerrainCamera,
        depthParameters);
    Invoke(
        FindMethod(cameraClass, "set_targetTexture", 1),
        g_devMapTerrainCamera,
        targetParameters);
    g_devMapTerrainRenderFrames = 4;
    LogInfo(LogCategory_Scene, L"Scene", 
        L"Colored map isolated Terrain render armed: disabledRenderers=%u.",
        static_cast<unsigned int>(
            g_devMapTerrainRendererStates.size()));
    return true;
}

void DevMapCapture_RestoreAfterTerrainRender()
{
    auto* rendererClass = FindClass("UnityEngine", "Renderer");
    for (const auto& state : g_devMapTerrainRendererStates)
    {
        bool enabled = state.second;
        void* enabledParameters[] = { &enabled };
        Invoke(
            FindMethod(rendererClass, "set_enabled", 1),
            state.first,
            enabledParameters);
    }
    g_devMapTerrainRendererStates.clear();
    g_devMapTerrainCameraObject = nullptr;
    g_devMapTerrainCamera = nullptr;
    g_devMapTerrainRenderTexture = nullptr;
    g_devMapTerrainPreviousActiveTexture = nullptr;
}

bool DevMapCapture_FinishIsolatedTerrainRender()
{
    auto* textureClass = FindClass("UnityEngine", "Texture2D");
    auto* renderTextureClass = FindClass("UnityEngine", "RenderTexture");
    auto* imageConversionClass =
        FindClass("UnityEngine", "ImageConversion");
    auto* objectClass = FindClass("UnityEngine", "Object");
    if (textureClass == nullptr ||
        renderTextureClass == nullptr ||
        imageConversionClass == nullptr ||
        g_devMapTerrainRenderTexture == nullptr)
    {
        DevMapCapture_RestoreAfterTerrainRender();
        return false;
    }

    auto* setActive = FindMethod(renderTextureClass, "set_active", 1);
    void* activeParameters[] = { g_devMapTerrainRenderTexture };
    Invoke(setActive, nullptr, activeParameters);
    auto* texture = il2cpp_object_new(textureClass);
    int width = kDevMapCaptureSize;
    int height = kDevMapCaptureSize;
    void* textureParameters[] = { &width, &height };
    Invoke(
        FindMethod(textureClass, ".ctor", 2),
        texture,
        textureParameters);
    float rect[] = {
        0.0f,
        0.0f,
        static_cast<float>(width),
        static_cast<float>(height),
    };
    int destX = 0;
    int destY = 0;
    bool recalculateMipMaps = false;
    void* readParameters[] = {
        rect,
        &destX,
        &destY,
        &recalculateMipMaps,
    };
    Invoke(
        FindMethod(textureClass, "ReadPixels", 4),
        texture,
        readParameters);
    Invoke(FindMethod(textureClass, "Apply", 0), texture, nullptr);
    void* restoreParameters[] = {
        g_devMapTerrainPreviousActiveTexture,
    };
    Invoke(setActive, nullptr, restoreParameters);

    bool written = false;
    void* encodeParameters[] = { texture };
    auto* png = Invoke(
        FindMethod(imageConversionClass, "EncodeToPNG", 1),
        nullptr,
        encodeParameters);
    if (png != nullptr)
    {
        const std::wstring directory =
            GetEscapeFromManakaSubdirectory(L"MapsDev");
        CreateDirectoryW(directory.c_str(), nullptr);
        const std::wstring path =
            directory + L"\\park_terrain_isolated_2560.png";
        const DWORD length = il2cpp_array_length(png);
        auto* bytes = reinterpret_cast<unsigned char*>(
            reinterpret_cast<char*>(png) +
            il2cpp_array_object_header_size());
        HANDLE file = CreateFileW(
            path.c_str(),
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);
        if (file != INVALID_HANDLE_VALUE)
        {
            DWORD writtenBytes = 0;
            written = WriteFile(
                file,
                bytes,
                length,
                &writtenBytes,
                nullptr) != FALSE &&
                writtenBytes == length;
            CloseHandle(file);
        }
    }

    if (g_devMapTerrainCameraObject != nullptr)
    {
        void* destroyParameters[] = { g_devMapTerrainCameraObject };
        Invoke(
            FindMethod(objectClass, "Destroy", 1),
            nullptr,
            destroyParameters);
    }
    if (g_devMapTerrainRenderTexture != nullptr)
    {
        Invoke(
            FindMethod(renderTextureClass, "Release", 0),
            g_devMapTerrainRenderTexture,
            nullptr);
        void* destroyParameters[] = { g_devMapTerrainRenderTexture };
        Invoke(
            FindMethod(objectClass, "Destroy", 1),
            nullptr,
            destroyParameters);
    }
    if (texture != nullptr)
    {
        void* destroyParameters[] = { texture };
        Invoke(
            FindMethod(objectClass, "Destroy", 1),
            nullptr,
            destroyParameters);
    }
    DevMapCapture_RestoreAfterTerrainRender();
    LogInfo(LogCategory_Scene, L"Scene", 
        L"Colored map isolated Terrain delayed render capture success=%d.",
        written ? 1 : 0);
    return written;
}

void DevMapCapture_WriteMetadata()
{
    const std::wstring directory =
        GetEscapeFromManakaSubdirectory(L"MapsDev");
    CreateDirectoryW(directory.c_str(), nullptr);
    const std::wstring path =
        directory + L"\\park_collision_material_map_2560.json";
    char text[4096]{};
    std::snprintf(
        text,
        sizeof(text),
        "{\n"
        "  \"stage\": 14,\n"
        "  \"type\": \"collision_navmesh_geometry\",\n"
        "  \"image\": \"park_collision_material_map_2560.bmp\",\n"
        "  \"walkableMask\": \"park_walkable_2560.r8\",\n"
        "  \"heightmap\": \"park_heightmap_2560.f32\",\n"
        "  \"gridWidth\": 2560,\n"
        "  \"gridHeight\": 2560,\n"
        "  \"minX\": %.6f,\n"
        "  \"maxX\": %.6f,\n"
        "  \"minY\": %.6f,\n"
        "  \"maxY\": %.6f,\n"
        "  \"minZ\": %.6f,\n"
        "  \"maxZ\": %.6f,\n"
        "  \"imageNorthAxis\": \"+Z\",\n"
        "  \"walkableNormalYMin\": %.3f,\n"
        "  \"navMeshSnapDistance\": %.3f,\n"
        "  \"navAcceptedCells\": %lld,\n"
        "  \"capsuleQueries\": %lld,\n"
        "  \"capsuleBlocked\": %lld,\n"
        "  \"raycasts\": %lld,\n"
        "  \"frames\": %d\n"
        "}\n",
        g_devMapBounds.minX,
        g_devMapBounds.maxX,
        g_devMapBounds.minY,
        g_devMapBounds.maxY,
        g_devMapBounds.minZ,
        g_devMapBounds.maxZ,
        kDevMapWalkableNormalY,
        kDevMapNavMeshSnapDistance,
        g_devMapNavAccepted,
        g_devMapCapsuleQueries,
        g_devMapCapsuleBlocked,
        g_devMapRaycastCount,
        g_devMapFrames);
    HANDLE file = CreateFileW(
        path.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
        return;
    }
    DWORD written = 0;
    WriteFile(
        file,
        text,
        static_cast<DWORD>(std::strlen(text)),
        &written,
        nullptr);
    CloseHandle(file);
}

bool DevMapCapture_WriteSpatialData(const std::wstring& directory)
{
    const auto writeFile = [](
        const std::wstring& path,
        const void* data,
        DWORD size)
    {
        HANDLE file = CreateFileW(
            path.c_str(),
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);
        if (file == INVALID_HANDLE_VALUE)
        {
            return false;
        }
        DWORD written = 0;
        const bool ok =
            WriteFile(file, data, size, &written, nullptr) != FALSE &&
            written == size;
        CloseHandle(file);
        return ok;
    };

    const DWORD cellCount = static_cast<DWORD>(
        kDevMapCaptureSize * kDevMapCaptureSize);
    const bool walkableWritten = writeFile(
        directory + L"\\park_walkable_2560.r8",
        g_devMapWalkable.data(),
        cellCount);
    const bool heightsWritten = writeFile(
        directory + L"\\park_heightmap_2560.f32",
        g_devMapHeights.data(),
        cellCount * static_cast<DWORD>(sizeof(float)));
    return walkableWritten && heightsWritten;
}

void DevMapCapture_Finish()
{
    // Remove one-pixel NavMesh speckles and fill one-pixel holes. Unlike a
    // blanket blur, this keeps ramps and narrow walkable strips intact.
    std::vector<uint8_t> cleaned = g_devMapWalkable;
    const int width = kDevMapCaptureSize;
    const int height = kDevMapCaptureSize;
    for (int y = 1; y < height - 1; ++y)
    {
        for (int x = 1; x < width - 1; ++x)
        {
            const int index = y * width + x;
            int walkableNeighbors = 0;
            float minHeight = 1.0e20f;
            float maxHeight = -1.0e20f;
            for (int oy = -1; oy <= 1; ++oy)
            {
                for (int ox = -1; ox <= 1; ++ox)
                {
                    if (ox == 0 && oy == 0)
                    {
                        continue;
                    }
                    const int neighbor = (y + oy) * width + x + ox;
                    if (g_devMapWalkable[neighbor])
                    {
                        ++walkableNeighbors;
                        minHeight = std::min(
                            minHeight,
                            g_devMapHeights[neighbor]);
                        maxHeight = std::max(
                            maxHeight,
                            g_devMapHeights[neighbor]);
                    }
                }
            }
            if (g_devMapWalkable[index] && walkableNeighbors == 0)
            {
                cleaned[index] = 0;
            }
            else if (!g_devMapWalkable[index] &&
                     walkableNeighbors >= 7 &&
                     maxHeight - minHeight <= 0.35f)
            {
                cleaned[index] = 1;
                const int left = index - 1;
                const int right = index + 1;
                const int up = index - width;
                const int down = index + width;
                g_devMapHeights[index] =
                    (g_devMapHeights[left] +
                     g_devMapHeights[right] +
                     g_devMapHeights[up] +
                     g_devMapHeights[down]) * 0.25f;
            }
        }
    }
    g_devMapWalkable.swap(cleaned);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            const int index = y * width + x;
            auto* pixel =
                g_devMapPixels.data() +
                static_cast<size_t>(index) * 3;
            if (!g_devMapWalkable[index])
            {
                pixel[0] = 39;
                pixel[1] = 43;
                pixel[2] = 47;
            }
        }
    }

    const std::wstring directory =
        GetEscapeFromManakaSubdirectory(L"MapsDev");
    CreateDirectoryW(directory.c_str(), nullptr);
    const std::wstring path =
        directory + L"\\park_collision_material_map_2560.bmp";
    const bool written = DevMapCapture_WriteBmp(path);
    if (written)
    {
        const bool spatialDataWritten =
            DevMapCapture_WriteSpatialData(directory);
        DevMapCapture_WriteMetadata();
        LogInfo(LogCategory_Scene, L"Scene", 
            L"Colored Park map capture complete: %s cells=%lld renderers=%lld missingRenderers=%lld textureSamples=%lld fallbacks=%lld spatialData=%d frames=%d.",
            path.c_str(),
            g_devMapNavAccepted,
            g_devMapRendererResolved,
            g_devMapRendererMissing,
            g_devMapTextureSamples,
            g_devMapColorFallbacks,
            spatialDataWritten ? 1 : 0,
            g_devMapFrames);
    }
    else
    {
        LogInfo(LogCategory_Scene, L"Scene", L"Colored Park map capture failed to write %s.", path.c_str());
    }

    if (g_devMapHitArrayHandle != 0 && il2cpp_gchandle_free != nullptr)
    {
        il2cpp_gchandle_free(g_devMapHitArrayHandle);
    }
    g_devMapHitArrayHandle = 0;
    g_devMapHitArray = nullptr;
    if (g_devMapColliderArrayHandle != 0 &&
        il2cpp_gchandle_free != nullptr)
    {
        il2cpp_gchandle_free(g_devMapColliderArrayHandle);
    }
    g_devMapColliderArrayHandle = 0;
    g_devMapColliderArray = nullptr;
    if (g_devMapRendererArrayHandle != 0 &&
        il2cpp_gchandle_free != nullptr)
    {
        il2cpp_gchandle_free(g_devMapRendererArrayHandle);
    }
    g_devMapRendererArrayHandle = 0;
    g_devMapRendererArray = nullptr;
    for (auto& entry : g_devMapTextures)
    {
        if (entry.second.gcHandle != 0 && il2cpp_gchandle_free != nullptr)
        {
            il2cpp_gchandle_free(entry.second.gcHandle);
        }
        if (entry.second.readable != nullptr &&
            entry.second.gcHandle != 0)
        {
            void* destroyParameters[] = { entry.second.readable };
            Invoke(
                FindMethod(
                    FindClass("UnityEngine", "Object"),
                    "Destroy",
                    1),
                nullptr,
                destroyParameters);
        }
    }
    g_devMapTextures.clear();
    g_devMapPaints.clear();
    g_devMapColliders.clear();
    g_devMapPixels.clear();
    g_devMapPixels.shrink_to_fit();
    g_devMapHeights.clear();
    g_devMapHeights.shrink_to_fit();
    g_devMapWalkable.clear();
    g_devMapWalkable.shrink_to_fit();
    g_devMapSurfaceScores.clear();
    g_devMapSurfaceScores.shrink_to_fit();
    g_devMapVisualHeights.clear();
    g_devMapVisualHeights.shrink_to_fit();
    g_devMapVisualValid.clear();
    g_devMapVisualValid.shrink_to_fit();
    g_devMapState = DevMapCapture_Complete;
}

void DevMapCapture_BeginScan()
{
    auto* raycastHitClass = FindClass("UnityEngine", "RaycastHit");
    auto* colliderClass = FindClass("UnityEngine", "Collider");
    if (raycastHitClass == nullptr || colliderClass == nullptr)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"Colored map capture could not resolve physics result classes.");
        g_devMapState = DevMapCapture_Complete;
        return;
    }
    g_devMapHitArray = il2cpp_array_new(
        raycastHitClass,
        kDevMapRaycastCapacity);
    if (g_devMapHitArray == nullptr)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"Colored map capture could not allocate the RaycastHit buffer.");
        g_devMapState = DevMapCapture_Complete;
        return;
    }
    if (il2cpp_gchandle_new != nullptr)
    {
        g_devMapHitArrayHandle =
            il2cpp_gchandle_new(g_devMapHitArray, false);
    }
    g_devMapColliderArray = il2cpp_array_new(
        colliderClass,
        kDevMapColliderCapacity);
    if (g_devMapColliderArray == nullptr)
    {
        LogInfo(LogCategory_Scene, L"Scene", L"Colored map capture could not allocate the Collider buffer.");
        g_devMapState = DevMapCapture_Complete;
        return;
    }
    if (il2cpp_gchandle_new != nullptr)
    {
        g_devMapColliderArrayHandle =
            il2cpp_gchandle_new(g_devMapColliderArray, false);
    }

    const size_t cellCount =
        static_cast<size_t>(kDevMapCaptureSize) * kDevMapCaptureSize;
    g_devMapPixels.assign(cellCount * 3, 0);
    g_devMapHeights.assign(cellCount, 0.0f);
    g_devMapWalkable.assign(cellCount, 0);
    g_devMapCursor = 0;
    g_devMapFrames = 0;
    g_devMapRaycastCount = 0;
    g_devMapNavAccepted = 0;
    g_devMapRendererResolved = 0;
    g_devMapRendererMissing = 0;
    g_devMapTextureSamples = 0;
    g_devMapVisualUvSamples = 0;
    g_devMapColorFallbacks = 0;
    g_devMapCapsuleQueries = 0;
    g_devMapCapsuleBlocked = 0;
    g_devMapVisualRenderersVisited = 0;
    g_devMapVisualTriangles = 0;
    g_devMapVisualPixels = 0;
    g_devMapVisualSkippedUnreadable = 0;
    g_devMapVisualTrianglesUpward = 0;
    g_devMapVisualTriangleCandidates = 0;
    g_devMapVisualHeightRejected = 0;
    g_devMapVisualAlphaRejected = 0;
    g_devMapState = DevMapCapture_Scanning;
    LogInfo(LogCategory_Scene, L"Scene", 
        L"Colored Park map capture started: %d x %d, %d cells/frame.",
        kDevMapCaptureSize,
        kDevMapCaptureSize,
        kDevMapCellsPerFrame);
}

void DevMapCapture_Tick()
{
    DevMapCapture_Initialize();
    if (g_devMapState == DevMapCapture_Disabled ||
        g_devMapState == DevMapCapture_Complete)
    {
        return;
    }

    const int modeState = g_extendedModeState.load();
    if (g_devMapState == DevMapCapture_WaitForWarehouse)
    {
        if (modeState == ExtendedMode_Inactive &&
            g_titleInstance != nullptr &&
            ExtendedMode_IsUnityObjectAlive(g_titleInstance))
        {
            if (g_devMapAutoStartDelay < 0)
            {
                g_devMapAutoStartDelay = 600;
                LogInfo(LogCategory_Scene, L"Scene", L"Colored map capture is waiting for a stable title before autostart.");
            }
            else if (--g_devMapAutoStartDelay <= 0)
            {
                g_devMapAutoStartDelay = -1;
                LogInfo(LogCategory_Scene, L"Scene", L"Colored map capture autostart is invoking the stable warehouse flow.");
                ExtendedMode_Begin();
            }
        }
        if (modeState == ExtendedMode_Active)
        {
            g_extendedTargetStageType = g_devMapStageType;
            g_extendedTargetDaytime = true;
            ExtendedMode_BeginSelectedGameplay();
            g_devMapState = DevMapCapture_WaitForStage;
            LogInfo(LogCategory_Scene, L"Scene", L"Colored Park map capture requested Park deployment.");
        }
        return;
    }

    if (g_devMapState == DevMapCapture_WaitForStage)
    {
        if (modeState == ExtendedMode_Gameplay &&
            ExtendedMode_GetCurrentStageType() == g_devMapStageType)
        {
            g_devMapSettleFrames = 180;
            g_devMapState = DevMapCapture_WaitForSettle;
            LogInfo(LogCategory_Scene, L"Scene", L"Colored Park map capture is waiting for Park scene settling.");
        }
        return;
    }

    if (g_devMapState == DevMapCapture_WaitForSettle)
    {
        if (modeState != ExtendedMode_Gameplay ||
            ExtendedMode_GetCurrentStageType() != g_devMapStageType)
        {
            return;
        }
        if (--g_devMapSettleFrames <= 0)
        {
            DevMapCapture_BeginScan();
        }
        return;
    }

    if (g_devMapState == DevMapCapture_WaitForTerrainRender)
    {
        if (modeState != ExtendedMode_Gameplay ||
            ExtendedMode_GetCurrentStageType() != g_devMapStageType)
        {
            return;
        }
        if (--g_devMapTerrainRenderFrames <= 0)
        {
            DevMapCapture_FinishIsolatedTerrainRender();
            DevMapCapture_BeginScan();
        }
        return;
    }

    if (g_devMapState == DevMapCapture_Scanning)
    {
        if (modeState != ExtendedMode_Gameplay ||
            ExtendedMode_GetCurrentStageType() != g_devMapStageType)
        {
            return;
        }
        const int cellCount =
            kDevMapCaptureSize * kDevMapCaptureSize;
        const int end = std::min(
            cellCount,
            g_devMapCursor + kDevMapCellsPerFrame);
        for (; g_devMapCursor < end; ++g_devMapCursor)
        {
            const int imageY =
                g_devMapCursor / kDevMapCaptureSize;
            const int imageX =
                g_devMapCursor % kDevMapCaptureSize;
            auto* pixel =
                g_devMapPixels.data() +
                static_cast<size_t>(g_devMapCursor) * 3;
            pixel[0] = 39;
            pixel[1] = 43;
            pixel[2] = 47;
            float height = 0.0f;
            if (DevMapCapture_TryCell(
                    imageX,
                    imageY,
                    pixel,
                    height))
            {
                g_devMapWalkable[g_devMapCursor] = 1;
                g_devMapHeights[g_devMapCursor] = height;
            }
        }
        ++g_devMapFrames;
        if (g_devMapCursor >= cellCount)
        {
            g_devMapState = DevMapCapture_PostProcess;
            LogInfo(LogCategory_Scene, L"Scene", 
                L"Colored Park map ray pass complete: walkable=%lld capsuleQueries=%lld capsuleBlocked=%lld frames=%d.",
                g_devMapNavAccepted,
                g_devMapCapsuleQueries,
                g_devMapCapsuleBlocked,
                g_devMapFrames);
        }
        return;
    }

    if (g_devMapState == DevMapCapture_RasterizingVisuals)
    {
        if (modeState != ExtendedMode_Gameplay ||
            ExtendedMode_GetCurrentStageType() != g_devMapStageType)
        {
            return;
        }
        DevMapCapture_ProcessVisualRenderers();
        if (g_devMapRendererCursor >= g_devMapRendererCount)
        {
            LogInfo(LogCategory_Scene, L"Scene", 
                L"Colored Park visual rasterization complete: visited=%lld triangles=%lld upward=%lld candidates=%lld heightRejected=%lld alphaRejected=%lld coloredPixels=%lld unreadableRenderers=%lld.",
                g_devMapVisualRenderersVisited,
                g_devMapVisualTriangles,
                g_devMapVisualTrianglesUpward,
                g_devMapVisualTriangleCandidates,
                g_devMapVisualHeightRejected,
                g_devMapVisualAlphaRejected,
                g_devMapVisualPixels,
                g_devMapVisualSkippedUnreadable);
            g_devMapState = DevMapCapture_PostProcess;
        }
        return;
    }

    if (g_devMapState == DevMapCapture_PostProcess)
    {
        DevMapCapture_Finish();
    }
}
