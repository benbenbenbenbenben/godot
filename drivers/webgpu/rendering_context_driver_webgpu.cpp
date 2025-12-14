#include "rendering_context_driver_webgpu.h"
#include "rendering_device_driver_webgpu.h"
#include <cstdio>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif


static void on_device_request_ended(WGPUDeviceRequestStatus status, WGPUDevice device, char const * params, void * userdata) {
	RenderingContextDriverWebGPU *driver = (RenderingContextDriverWebGPU *)userdata;
	if (status == WGPUDeviceRequestStatus_Success) {
		driver->set_wgpu_device(device);
	} else {
		// Log error
		printf("WebGPU Device request failed: %s\n", params ? params : "Unknown error");
	}
}

static void on_adapter_request_ended(WGPUAdapterRequestStatus status, WGPUAdapter adapter, char const * params, void * userdata) {
	RenderingContextDriverWebGPU *driver = (RenderingContextDriverWebGPU *)userdata;
	if (status == WGPUAdapterRequestStatus_Success) {
		WGPUDeviceDescriptor desc = {};
		desc.usage = WGPUUsage_User; // Default usage
		// TODO: Set required features and limits
		
		wgpuAdapterRequestDevice(adapter, &desc, on_device_request_ended, userdata);
	} else {
		printf("WebGPU Adapter request failed: %s\n", params ? params : "Unknown error");
	}
}

Error RenderingContextDriverWebGPU::initialize() {
#ifdef __EMSCRIPTEN__
	// Initialize WebGPU instance
	WGPUInstanceDescriptor desc = {};
	instance = wgpuCreateInstance(&desc);
	
	if (!instance) {
		return ERR_CANT_CREATE;
	}

	WGPURequestAdapterOptions options = {};
	options.compatibleSurface = surface; // This might be null if called before surface_create? 
	// Godot calls driver->initialize() then surface_create()? 
	// DisplayServerWeb calls context->surface_create() then context->make_current() which uses driver.
	// Actually RenderingDevice creates driver then calls initialize.
	// We need surface to request compatible adapter usually.
	
	wgpuInstanceRequestAdapter(instance, &options, on_adapter_request_ended, this);
	
	// TODO: Wait for device to be ready. 
	// This requires ASYNCIFY in Emscripten to block here while the browser event loop processes the promise.
	// emscripten_sleep(0); or similar loop checking if device is set.
	
#endif
	return OK;
}

const RenderingContextDriver::Device &RenderingContextDriverWebGPU::device_get(uint32_t p_device_index) const {
	return device;
}

uint32_t RenderingContextDriverWebGPU::device_get_count() const {
	return 1;
}

bool RenderingContextDriverWebGPU::device_supports_present(uint32_t p_device_index, SurfaceID p_surface) const {
	return true;
}

RenderingDeviceDriver *RenderingContextDriverWebGPU::driver_create() {
	return memnew(RenderingDeviceDriverWebGPU(this));
}

void RenderingContextDriverWebGPU::driver_free(RenderingDeviceDriver *p_driver) {
	memdelete(p_driver);
}

RenderingContextDriver::SurfaceID RenderingContextDriverWebGPU::surface_create(const void *p_platform_data) {
#ifdef __EMSCRIPTEN__
	if (!instance) {
		return 0;
	}

	WGPUSurfaceDescriptorFromCanvasHTMLSelector canvasDesc = {};
	canvasDesc.chain.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
	canvasDesc.selector = "#canvas";
	
	WGPUSurfaceDescriptor surfaceDesc = {};
	surfaceDesc.nextInChain = &canvasDesc.chain;
	
	surface = wgpuInstanceCreateSurface(instance, &surfaceDesc);
	return (SurfaceID)surface;
#else
	return 1;
#endif
}

void RenderingContextDriverWebGPU::surface_set_size(SurfaceID p_surface, uint32_t p_width, uint32_t p_height) {
}

void RenderingContextDriverWebGPU::surface_set_vsync_mode(SurfaceID p_surface, DisplayServer::VSyncMode p_vsync_mode) {
}

DisplayServer::VSyncMode RenderingContextDriverWebGPU::surface_get_vsync_mode(SurfaceID p_surface) const {
	return DisplayServer::VSYNC_ENABLED;
}

uint32_t RenderingContextDriverWebGPU::surface_get_width(SurfaceID p_surface) const {
	return 1920; // TODO: Get actual size
}

uint32_t RenderingContextDriverWebGPU::surface_get_height(SurfaceID p_surface) const {
	return 1080; // TODO: Get actual size
}

void RenderingContextDriverWebGPU::surface_set_needs_resize(SurfaceID p_surface, bool p_needs_resize) {
}

bool RenderingContextDriverWebGPU::surface_get_needs_resize(SurfaceID p_surface) const {
	return false;
}

void RenderingContextDriverWebGPU::surface_destroy(SurfaceID p_surface) {
#ifdef __EMSCRIPTEN__
	if (p_surface == (SurfaceID)surface && surface) {
		wgpuSurfaceRelease(surface);
		surface = nullptr;
	}
#endif
}

bool RenderingContextDriverWebGPU::is_debug_utils_enabled() const {
	return false;
}

RenderingContextDriverWebGPU::RenderingContextDriverWebGPU() {
}

RenderingContextDriverWebGPU::~RenderingContextDriverWebGPU() {
#ifdef __EMSCRIPTEN__
	if (instance) {
		wgpuInstanceRelease(instance);
	}
#endif
}
