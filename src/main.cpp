#include "logger.h"
#include "ShaderSerializer.h"

#include <coreinit/bsp.h>
#include <coreinit/filesystem.h>
#include <coreinit/foreground.h>
#include <coreinit/memdefaultheap.h>
#include <coreinit/thread.h>
#include <coreinit/title.h>
#include <gx2/draw.h>
#include <gx2/mem.h>
#include <gx2/swap.h>
#include <gx2r/draw.h>
#include <gx2r/surface.h>
#include <nn/erreula.h>
#include <proc_ui/procui.h>
#include <sysapp/launch.h>

#include <whb/gfx.h>

#include <mocha/mocha.h>

// https://github.com/wiiu-env/SplashscreenModule/blob/d58e30a85f58de7e15a8535823a057feaeb44e56/source/gfx/SplashScreenDrawer.cpp#L33
constexpr uint8_t s_textureVertexShaderCompiled[] = {
        0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x8A, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
        0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
        0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
        0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
        0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
        0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
        0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
        0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
        0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
        0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
        0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E,
        0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x80, 0x89, 0x00, 0x40, 0x01, 0xC0, 0xC8, 0x0F, 0x00, 0x94,
        0x3C, 0xA0, 0x00, 0xC0, 0x08, 0x0B, 0x00, 0x94, 0x05, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x80,
        0x00, 0x00, 0x00, 0x80, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0F, 0xFF,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x61, 0x50, 0x6F, 0x73,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x61, 0x54, 0x65, 0x78, 0x43, 0x6F, 0x6F, 0x72,
        0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00};

// https://github.com/wiiu-env/SplashscreenModule/blob/d58e30a85f58de7e15a8535823a057feaeb44e56/source/gfx/SplashScreenDrawer.cpp#L83
constexpr uint8_t s_texturePixelShaderCompiled[] = {
        0x00, 0x20, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x10, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x8A,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20,
        0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0xC0,
        0x88, 0x06, 0x20, 0x94, 0x10, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0D, 0xF0,
        0x00, 0x00, 0x80, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0F, 0xFF,
        0x00, 0x00, 0x00, 0x01, 0x69, 0x6E, 0x54, 0x65, 0x78, 0x74, 0x75, 0x72,
        0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// https://github.com/wiiu-env/SplashscreenModule/blob/d58e30a85f58de7e15a8535823a057feaeb44e56/source/gfx/SplashScreenDrawer.h#L21
const float positionData[8] = {
        -1.0f,
        -1.0f,
        1.0f,
        -1.0f,
        1.0f,
        1.0f,
        -1.0f,
        1.0f,
};

// https://github.com/wiiu-env/SplashscreenModule/blob/d58e30a85f58de7e15a8535823a057feaeb44e56/source/gfx/SplashScreenDrawer.h#L32
const float texCoords[8] = {
        0.0f,
        1.0f,
        1.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
};

// BSP results & permissions: https://github.com/NWPlayer123/IOSU/blob/master/ios_bsp/libraries/bsp_entity/include/bsp_entity_enum.h

// unused, alternative patch
void givePpcBspAllClientCredentials() {
    uint32_t active = 0;
    uint32_t permissions = 0;
    for (uint32_t address = 0xE60471FC; address >= 0xE6047104; address -= 8) {
        Mocha_IOSUKernelRead32(address, &active);
        // skip if not active
        if (active == 0)
            continue;
        Mocha_IOSUKernelRead32(address + 4, &permissions);
        if (permissions == 0xF00) { // BSP_PERMISSIONS_PPC_USER
            Mocha_IOSUKernelWrite32(address + 4, 0xFFFF); // BSP_PERMISSIONS_ALL
            break;
        }
    }
}

void giveEjectRequestPpcPermissions() {
    // entity & attribute struct: https://github.com/NWPlayer123/IOSU/blob/master/ios_bsp/libraries/bsp_entity/include/bsp_entity.h
    // SMC entity is at 0xE6040D94
    // SMC attributes array: 0xE6044364
    uint32_t permissions = 0;
    // +44 for 2nd attribute (which is EjectRequest), +8 for permissions
    Mocha_IOSUKernelRead32(0xE6044364 + 44 + 8, &permissions);
    // by default EjectRequest has perms 0xFF (BSP_PERMISSIONS_IOS)
    Mocha_IOSUKernelWrite32(0xE6044364 + 44 + 8, permissions | 0xF00); // BSP_PERMISSIONS_PPC_USER
}

void procUiSaveCallback() {
    OSSavesDone_ReadyToRelease();
}

// copied from https://github.com/cemu-project/Cemu/blob/57fe7a53f1c842c3bfb222d2a82666dc7315f01f/src/Cafe/IOSU/ODM/iosu_odm.cpp#L29
enum OdmState {
    ODM_STATE_NONE = 0,
    ODM_STATE_INITIAL = 1,
    ODM_STATE_AUTHENTICATION = 2,
    ODM_STATE_WAIT_FOR_DISC_READY = 3,
    ODM_STATE_CAFE_DISC = 4,
    ODM_STATE_RVL_DISC = 5,
    ODM_STATE_CLEANING_DISC = 6,
    ODM_STATE_INVALID_DISC = 8,
    ODM_STATE_DIRTY_DISC = 9,
    ODM_STATE_NO_DISC = 10,
    ODM_STATE_INVALID_DRIVE = 11,
    ODM_STATE_FATAL = 12,
    ODM_STATE_HARD_FATAL = 13,
    ODM_STATE_SHUTDOWN = 14,
};

// returns one of OdmState. Mocha not required
int32_t getDriveState() {
    IOSHandle odmHandle = IOS_Open("/dev/odm", IOS_OPEN_READ);
    if (odmHandle < 0) {
        return ODM_STATE_NONE;
    }
    alignas(0x40) uint32_t ioBuffer[0x40 / 4];
    ioBuffer[0] = 0;
    IOS_Ioctl(odmHandle, 0x4, ioBuffer, 0x14, ioBuffer, 0x20);
    IOS_Close(odmHandle);
    return ioBuffer[0];
}

// copied from https://github.com/wiiu-env/AromaUpdater/blob/9be5f01ca6b00a09fcaa5f6e46ca00a429539937/source/common.h#L20
inline bool runningFromMiiMaker() {
    return (OSGetTitleID() & 0xFFFFFFFFFFFFF0FFull) == 0x000500101004A000ull;
}

void quit() {
    if (runningFromMiiMaker()) {
        SYSRelaunchTitle(0, 0);
    } else {
        SYSLaunchMenu();
    }
}

int32_t main() {
    ProcUIInit(procUiSaveCallback);

    initLogging();

    bool quitting = false;
    const int32_t driveState = getDriveState();
    DEBUG_FUNCTION_LINE("Drive state: %d", driveState);

    Mocha_InitLibrary();
    giveEjectRequestPpcPermissions();
    uint32_t request = 1;
    bspWrite("SMC", 0, "EjectRequest", 4, &request);

    if (WHBGfxInit() == FALSE) {
        DEBUG_FUNCTION_LINE_ERR("Failed to initialise graphics");
    }

    // get app transition frames to display later
    GX2Texture *lastFrameTvTexture = (GX2Texture *) MEMAllocFromDefaultHeapEx(sizeof(GX2Texture), 64);
    if (lastFrameTvTexture) {
        if (!GX2GetLastFrame(GX2_SCAN_TARGET_TV, lastFrameTvTexture)) {
            MEMFreeToDefaultHeap(lastFrameTvTexture);
            lastFrameTvTexture = nullptr;
        }
    }
    GX2Texture *lastFrameDrcTexture = (GX2Texture *) MEMAllocFromDefaultHeapEx(sizeof(GX2Texture), 64);
    if (lastFrameDrcTexture) {
        if (!GX2GetLastFrame(GX2_SCAN_TARGET_DRC, lastFrameDrcTexture)) {
            MEMFreeToDefaultHeap(lastFrameDrcTexture);
            lastFrameDrcTexture = nullptr;
        }
    }

    // create shader group
    // https://github.com/wiiu-env/SplashscreenModule/blob/d58e30a85f58de7e15a8535823a057feaeb44e56/source/gfx/SplashScreenDrawer.cpp#L178-L192
    std::unique_ptr<GX2VertexShaderWrapper> vertexShaderWrapper = DeserializeVertexShader(s_textureVertexShaderCompiled);
    std::unique_ptr<GX2PixelShaderWrapper> pixelShaderWrapper = DeserializePixelShader(s_texturePixelShaderCompiled);
    WHBGfxShaderGroup shaderGroup = {};
    shaderGroup.vertexShader = vertexShaderWrapper->getVertexShader();
    shaderGroup.pixelShader = pixelShaderWrapper->getPixelShader();
    GX2Invalidate(GX2_INVALIDATE_MODE_CPU_SHADER, shaderGroup.vertexShader->program, shaderGroup.vertexShader->size);
    GX2Invalidate(GX2_INVALIDATE_MODE_CPU_SHADER, shaderGroup.pixelShader->program, shaderGroup.pixelShader->size);
    GX2SetShaderMode(GX2_SHADER_MODE_UNIFORM_BLOCK);
    WHBGfxInitShaderAttribute(&shaderGroup, "aPos", 0, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32);
    WHBGfxInitShaderAttribute(&shaderGroup, "aTexCoord", 1, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32);
    WHBGfxInitFetchShader(&shaderGroup);

    // upload vertex position
    // https://github.com/wiiu-env/SplashscreenModule/blob/d58e30a85f58de7e15a8535823a057feaeb44e56/source/gfx/SplashScreenDrawer.cpp#L195-L201
    GX2RBuffer positionBuffer = {};
    positionBuffer.flags = GX2R_RESOURCE_BIND_VERTEX_BUFFER | GX2R_RESOURCE_USAGE_CPU_READ | GX2R_RESOURCE_USAGE_CPU_WRITE | GX2R_RESOURCE_USAGE_GPU_READ;
    positionBuffer.elemSize = 2 * sizeof(float);
    positionBuffer.elemCount = 4;
    GX2RCreateBuffer(&positionBuffer);
    void *posUploadBuffer = GX2RLockBufferEx(&positionBuffer, GX2R_RESOURCE_BIND_NONE);
    memcpy(posUploadBuffer, positionData, positionBuffer.elemSize * positionBuffer.elemCount);
    GX2RUnlockBufferEx(&positionBuffer, GX2R_RESOURCE_BIND_NONE);

    // upload texture coords
    // https://github.com/wiiu-env/SplashscreenModule/blob/d58e30a85f58de7e15a8535823a057feaeb44e56/source/gfx/SplashScreenDrawer.cpp#L204-L212
    GX2RBuffer texCoordBuffer = {};
    texCoordBuffer.flags = GX2R_RESOURCE_BIND_VERTEX_BUFFER | GX2R_RESOURCE_USAGE_CPU_READ | GX2R_RESOURCE_USAGE_CPU_WRITE | GX2R_RESOURCE_USAGE_GPU_READ;
    texCoordBuffer.elemSize = 2 * sizeof(float);
    texCoordBuffer.elemCount = 4;
    GX2RCreateBuffer(&texCoordBuffer);
    void *coordsUploadBuffer = GX2RLockBufferEx(&texCoordBuffer, GX2R_RESOURCE_BIND_NONE);
    memcpy(coordsUploadBuffer, texCoords, texCoordBuffer.elemSize * texCoordBuffer.elemCount);
    GX2RUnlockBufferEx(&texCoordBuffer, GX2R_RESOURCE_BIND_NONE);

    GX2Sampler sampler = {};
    GX2InitSampler(&sampler, GX2_TEX_CLAMP_MODE_CLAMP, GX2_TEX_XY_FILTER_MODE_LINEAR);

    // create FSClient for erreula
    FSInit();
    FSClient *fsClient = (FSClient *) MEMAllocFromDefaultHeap(sizeof(FSClient));
    FSAddClient(fsClient, FS_ERROR_FLAG_NONE);
    // create erreula
    nn::erreula::CreateArg createArg{};
    createArg.region = nn::erreula::RegionType::Europe;
    createArg.language = nn::erreula::LangType::English;
    createArg.fsClient = fsClient;
    createArg.workMemory = MEMAllocFromDefaultHeap(nn::erreula::GetWorkMemorySize());
    if (!nn::erreula::Create(createArg)) {
        DEBUG_FUNCTION_LINE_ERR("Failed to create erreula. FSClient: %p, workMemory: %p", fsClient, createArg.workMemory);
    }

    OSTime quitTimer = OSGetSystemTime();

    // only display error viewer if no disc is inserted
    if (driveState == ODM_STATE_NO_DISC) {
        // show the error viewer
        nn::erreula::AppearArg appearArg{};
        appearArg.errorArg.errorType = nn::erreula::ErrorType::Message;
        appearArg.errorArg.renderTarget = nn::erreula::RenderTarget::Both;
        appearArg.errorArg.errorMessage = u"No disc inserted";
        nn::erreula::AppearErrorViewer(appearArg);

        // wait for 2 seconds before quitting
        quitTimer += OSSecondsToTicks(2);
    }

    ProcUIStatus status = PROCUI_STATUS_IN_FOREGROUND;
    while ((status = ProcUIProcessMessages(TRUE)) != PROCUI_STATUS_EXITING) {
        if (status == PROCUI_STATUS_RELEASE_FOREGROUND) {
            ProcUIDrawDoneRelease();
            continue;
        } else if (status != PROCUI_STATUS_IN_FOREGROUND) {
            continue;
        }

        // have to call Calc (even with no controllerInfo) to update the error viewer state
        nn::erreula::ControllerInfo controllerInfo{};
        nn::erreula::Calc(controllerInfo);

        WHBGfxBeginRender();
        WHBGfxBeginRenderTV();
        WHBGfxClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        if (lastFrameTvTexture) {
            // fix displaying the SRGB last frame texture
            GX2ColorBuffer *colourBuffer = WHBGfxGetTVColourBuffer();
            GX2SurfaceFormat savedFormat = colourBuffer->surface.format;
            colourBuffer->surface.format = static_cast<GX2SurfaceFormat>(savedFormat | 0x00000400);
            GX2InitColorBufferRegs(colourBuffer);
            GX2SetColorBuffer(colourBuffer, GX2_RENDER_TARGET_0);

            GX2SetFetchShader(&shaderGroup.fetchShader);
            GX2SetVertexShader(shaderGroup.vertexShader);
            GX2SetPixelShader(shaderGroup.pixelShader);
            GX2SetShaderMode(GX2_SHADER_MODE_UNIFORM_BLOCK);
            GX2RSetAttributeBuffer(&positionBuffer, 0, positionBuffer.elemSize, 0);
            GX2RSetAttributeBuffer(&texCoordBuffer, 1, texCoordBuffer.elemSize, 0);
            GX2SetPixelTexture(lastFrameTvTexture, shaderGroup.pixelShader->samplerVars[0].location);
            GX2SetPixelSampler(&sampler, shaderGroup.pixelShader->samplerVars[0].location);
            GX2DrawEx(GX2_PRIMITIVE_MODE_QUADS, 4, 0, 1);

            // undo fix
            colourBuffer->surface.format = savedFormat;
            GX2InitColorBufferRegs(colourBuffer);
            GX2SetColorBuffer(colourBuffer, GX2_RENDER_TARGET_0);
        }

        nn::erreula::DrawTV();

        WHBGfxFinishRenderTV();
        WHBGfxBeginRenderDRC();
        WHBGfxClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        if (lastFrameDrcTexture) {
            // fix displaying the SRGB last frame texture
            GX2ColorBuffer *colourBuffer = WHBGfxGetDRCColourBuffer();
            GX2SurfaceFormat savedFormat = colourBuffer->surface.format;
            colourBuffer->surface.format = static_cast<GX2SurfaceFormat>(savedFormat | 0x00000400);
            GX2InitColorBufferRegs(colourBuffer);
            GX2SetColorBuffer(colourBuffer, GX2_RENDER_TARGET_0);

            GX2SetFetchShader(&shaderGroup.fetchShader);
            GX2SetVertexShader(shaderGroup.vertexShader);
            GX2SetPixelShader(shaderGroup.pixelShader);
            GX2SetShaderMode(GX2_SHADER_MODE_UNIFORM_BLOCK);
            GX2RSetAttributeBuffer(&positionBuffer, 0, positionBuffer.elemSize, 0);
            GX2RSetAttributeBuffer(&texCoordBuffer, 1, texCoordBuffer.elemSize, 0);
            GX2SetPixelTexture(lastFrameDrcTexture, shaderGroup.pixelShader->samplerVars[0].location);
            GX2SetPixelSampler(&sampler, shaderGroup.pixelShader->samplerVars[0].location);
            GX2DrawEx(GX2_PRIMITIVE_MODE_QUADS, 4, 0, 1);

            // undo fix
            colourBuffer->surface.format = savedFormat;
            GX2InitColorBufferRegs(colourBuffer);
            GX2SetColorBuffer(colourBuffer, GX2_RENDER_TARGET_0);
        }

        nn::erreula::DrawDRC();

        WHBGfxFinishRenderDRC();
        WHBGfxFinishRender();

        if (!quitting && OSGetSystemTime() >= quitTimer) {
            quitting = true;
            quit();
        }
    }

    // clean up erreula and FSClient
    nn::erreula::Destroy();
    MEMFreeToDefaultHeap(createArg.workMemory);
    FSDelClient(fsClient, FS_ERROR_FLAG_NONE);
    MEMFreeToDefaultHeap(fsClient);
    FSShutdown();

    GX2RDestroyBufferEx(&positionBuffer, GX2R_RESOURCE_BIND_NONE);
    GX2RDestroyBufferEx(&texCoordBuffer, GX2R_RESOURCE_BIND_NONE);
    if (lastFrameTvTexture) {
        GX2RDestroySurfaceEx(&lastFrameTvTexture->surface, GX2R_RESOURCE_BIND_NONE);
        MEMFreeToDefaultHeap(lastFrameTvTexture);
    }
    if (lastFrameDrcTexture) {
        GX2RDestroySurfaceEx(&lastFrameDrcTexture->surface, GX2R_RESOURCE_BIND_NONE);
        MEMFreeToDefaultHeap(lastFrameDrcTexture);
    }
    WHBGfxShutdown();

    Mocha_DeInitLibrary();
    deinitLogging();

    ProcUIShutdown();
    return 0;
}
