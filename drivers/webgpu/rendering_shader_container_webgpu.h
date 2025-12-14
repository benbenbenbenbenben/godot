#pragma once

#include "servers/rendering/rendering_shader_container.h"

class RenderingShaderContainerWebGPU : public RenderingShaderContainer {
	GDSOFTCLASS(RenderingShaderContainerWebGPU, RenderingShaderContainer);

protected:
	virtual uint32_t _format() const override;
	virtual uint32_t _format_version() const override;
	virtual bool _set_code_from_spirv(const ReflectShader &p_shader) override;

	struct StageWGSL {
		RDC::ShaderStage stage;
		String wgsl;
        
        struct Binding {
            uint32_t set = 0;
            uint32_t binding = 0;
            uint32_t mirror_binding = 0xFFFFFFFF; // For combined samplers, this tracks the secondary binding (e.g. sampler)
            WGPUBufferBindingType buffer_type = WGPUBufferBindingType_Undefined;
            WGPUTextureSampleType texture_sample_type = WGPUTextureSampleType_Float;
            WGPUTextureViewDimension texture_view_dim = WGPUTextureViewDimension_2D;
            WGPUStorageTextureAccess storage_access = WGPUStorageTextureAccess_Undefined;
            WGPUSamplerBindingType sampler_type = WGPUSamplerBindingType_Filtering;
            bool is_buffer = false;
            bool is_texture = false;
            bool is_sampler = false;
            bool is_storage_texture = false;
        };
        Vector<Binding> bindings;
	};
	Vector<StageWGSL> wgsl_code;

public:
	RenderingShaderContainerWebGPU();
	const Vector<StageWGSL> &get_wgsl_code() const { return wgsl_code; }
};

class RenderingShaderContainerFormatWebGPU : public RenderingShaderContainerFormat {
	GDSOFTCLASS(RenderingShaderContainerFormatWebGPU, RenderingShaderContainerFormat);

public:
	virtual Ref<RenderingShaderContainer> create_container() const override;
	virtual ShaderLanguageVersion get_shader_language_version() const override;
	virtual ShaderSpirvVersion get_shader_spirv_version() const override;
};
