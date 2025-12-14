#include "rendering_shader_container_webgpu.h"

uint32_t RenderingShaderContainerWebGPU::_format() const {
	return 0; // TODO: Define a format ID for WebGPU
}

uint32_t RenderingShaderContainerWebGPU::_format_version() const {
	return 1;
}

bool RenderingShaderContainerWebGPU::_set_code_from_spirv(const ReflectShader &p_shader) {
	// TODO: Convert SPIR-V to WGSL here.
	// For now, just store dummy data or fail.
	return true;
}

RenderingShaderContainerWebGPU::RenderingShaderContainerWebGPU() {
}

Ref<RenderingShaderContainer> RenderingShaderContainerFormatWebGPU::create_container() const {
	return memnew(RenderingShaderContainerWebGPU);
}

RenderingDeviceCommons::ShaderLanguageVersion RenderingShaderContainerFormatWebGPU::get_shader_language_version() const {
	// WebGPU uses WGSL, but we need to return a value from the enum.
	// Maybe return VULKAN_1_0 as placeholder, or define a new one if possible (but enum is in common).
	return SHADER_LANGUAGE_VULKAN_VERSION_1_0;
}

RenderingDeviceCommons::ShaderSpirvVersion RenderingShaderContainerFormatWebGPU::get_shader_spirv_version() const {
	return SHADER_SPIRV_VERSION_1_0;
}
