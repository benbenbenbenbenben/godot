#pragma once

#include "servers/rendering/rendering_shader_container.h"

class RenderingShaderContainerWebGPU : public RenderingShaderContainer {
	GDSOFTCLASS(RenderingShaderContainerWebGPU, RenderingShaderContainer);

protected:
	virtual uint32_t _format() const override;
	virtual uint32_t _format_version() const override;
	virtual bool _set_code_from_spirv(const ReflectShader &p_shader) override;

public:
	RenderingShaderContainerWebGPU();
};

class RenderingShaderContainerFormatWebGPU : public RenderingShaderContainerFormat {
	GDSOFTCLASS(RenderingShaderContainerFormatWebGPU, RenderingShaderContainerFormat);

public:
	virtual Ref<RenderingShaderContainer> create_container() const override;
	virtual ShaderLanguageVersion get_shader_language_version() const override;
	virtual ShaderSpirvVersion get_shader_spirv_version() const override;
};
