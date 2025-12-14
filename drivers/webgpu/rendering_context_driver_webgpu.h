#pragma once

#ifdef __EMSCRIPTEN__
#include <webgpu/webgpu.h>
#else
// Mock handles for non-Emscripten builds to allow syntax checking
typedef struct WGPUInstanceImpl* WGPUInstance;
typedef struct WGPUSurfaceImpl* WGPUSurface;
typedef struct WGPUAdapterImpl* WGPUAdapter;
typedef struct WGPUDeviceImpl* WGPUDevice;
#endif

#include "servers/rendering/rendering_context_driver.h"

class RenderingDeviceDriverWebGPU;

class RenderingContextDriverWebGPU : public RenderingContextDriver {
	Device device;
	WGPUInstance instance = nullptr;
	WGPUSurface surface = nullptr;
	WGPUAdapter adapter = nullptr;
	WGPUDevice wgpu_device = nullptr;

public:
	virtual Error initialize() override;
	virtual const Device &device_get(uint32_t p_device_index) const override;
	virtual uint32_t device_get_count() const override;
	virtual bool device_supports_present(uint32_t p_device_index, SurfaceID p_surface) const override;
	virtual RenderingDeviceDriver *driver_create() override;
	virtual void driver_free(RenderingDeviceDriver *p_driver) override;
	virtual SurfaceID surface_create(const void *p_platform_data) override;
	virtual void surface_set_size(SurfaceID p_surface, uint32_t p_width, uint32_t p_height) override;
	virtual void surface_set_vsync_mode(SurfaceID p_surface, DisplayServer::VSyncMode p_vsync_mode) override;
	virtual DisplayServer::VSyncMode surface_get_vsync_mode(SurfaceID p_surface) const override;
	virtual uint32_t surface_get_width(SurfaceID p_surface) const override;
	virtual uint32_t surface_get_height(SurfaceID p_surface) const override;
	virtual void surface_set_needs_resize(SurfaceID p_surface, bool p_needs_resize) override;
	virtual bool surface_get_needs_resize(SurfaceID p_surface) const override;
	virtual void surface_destroy(SurfaceID p_surface) override;
	virtual bool is_debug_utils_enabled() const override;

	WGPUDevice get_wgpu_device() const { return wgpu_device; }
	void set_wgpu_device(WGPUDevice p_device) { wgpu_device = p_device; }

	RenderingContextDriverWebGPU();
	virtual ~RenderingContextDriverWebGPU();
};
