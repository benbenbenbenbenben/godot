#include "rendering_shader_container_webgpu.h"

uint32_t RenderingShaderContainerWebGPU::_format() const {
	return 0; // TODO: Define a format ID for WebGPU
}

uint32_t RenderingShaderContainerWebGPU::_format_version() const {
	return 1;
}

#include "thirdparty/spirv-cross/spirv_wgsl.hpp"

// ...

bool RenderingShaderContainerWebGPU::_set_code_from_spirv(const ReflectShader &p_shader) {
	wgsl_code.clear();
    
    for (const ReflectShaderStage &stage : p_shader.shader_stages) {
        // SPIR-V data
        const uint32_t *spirv_ptr = stage.spirv().ptr();
        size_t word_count = stage.spirv().size();
        
        if (!spirv_ptr || word_count == 0) continue;

        try {
            spirv_cross::CompilerWGSL compiler(spirv_ptr, word_count);
            
            spirv_cross::CompilerWGSL::Options options;
            // options.emit_push_constants_as_uniform_buffer = true; // Godot uses push constants? WebGPU fits them to limits?
            // Godot RDD usually handles push constants via uniform buffers or root constants.
            // WebGPU only supports "setPushConstants" (compat) or mapped to UBO.
            // Spirv-Cross default usually maps them.
            compiler.set_common_options(options);
            
            std::string source = compiler.compile();
            
            StageWGSL s;
            s.stage = stage.shader_stage;
            s.wgsl = String::utf8(source.c_str());
            
            // Extract Reflection
            spirv_cross::ShaderResources resources = compiler.get_shader_resources();
            
            // Helper to process resources
            auto process_resources = [&](const spirv_cross::SmallVector<spirv_cross::Resource> &res_list, bool is_buffer, bool is_tex, bool is_sampler, bool is_storage) {
                for (const auto &res : res_list) {
                    StageWGSL::Binding b = {};
                    b.set = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
                    b.binding = compiler.get_decoration(res.id, spv::DecorationBinding);
                    b.is_buffer = is_buffer;
                    b.is_texture = is_tex;
                    b.is_sampler = is_sampler;
                    b.is_storage_texture = is_storage;
                    
                    if (is_buffer) {
                         // Default uniform, check if storage?
                         b.buffer_type = WGPUBufferBindingType_Uniform; 
                         // Check decorations for readonly/std140/etc.
                         spv::ExecutionModel model = compiler.get_execution_model();
                         // Logic for storage buffer...
                         // If resource in 'storage_buffers', map to Storage.
                         // But we verify against specific lists.
                    }
                    if (is_tex) {
                        b.texture_sample_type = WGPUTextureSampleType_Float; // Detect?
                        b.texture_view_dim = WGPUTextureViewDimension_2D; // Detect dim
                    }
                    if (is_sampler) {
                        b.sampler_type = WGPUSamplerBindingType_Filtering; // Detect comparison?
                    }

                    s.bindings.push_back(b);
                }
            };
            
            process_resources(resources.uniform_buffers, true, false, false, false);
            process_resources(resources.storage_buffers, true, false, false, false);
            process_resources(resources.separate_images, false, true, false, false);
            process_resources(resources.separate_samplers, false, false, true, false);
            process_resources(resources.storage_images, false, false, false, true);

            // Combined Image Samplers (split)
            auto combined = compiler.get_combined_image_samplers();
            for (const auto &remap : combined) {
                // Determine the bindings of the split resources
                 uint32_t tex_set = compiler.get_decoration(remap.image_id, spv::DecorationDescriptorSet);
                 uint32_t tex_binding = compiler.get_decoration(remap.image_id, spv::DecorationBinding);
                 
                 uint32_t samp_set = compiler.get_decoration(remap.sampler_id, spv::DecorationDescriptorSet);
                 uint32_t samp_binding = compiler.get_decoration(remap.sampler_id, spv::DecorationBinding);
                 
                 // We need to record this link so when binding "Combined" at Godot Binding X,
                 // we bind Texture at tex_binding and Sampler at samp_binding.
                 // We add a special Binding entry or mark existing?
                 // Simple: Add both.
                 // Note: 'proces_resources' already added them if they appear in separate_images/samplers lists?
                 // Check if spirv-cross moves combined into separate lists when compiling to WGSL. Yes it does.
                 // So we just need to know they are linked if we want to support "SAMPLER_WITH_TEXTURE" uniform set creation.
                 // Assuming Godot sends Texture+Sampler as one item.
                 
                 // Find the Texture binding we just added and link it to Sampler binding?
                 for (int i=0; i<s.bindings.size(); i++) {
                     if (s.bindings[i].is_texture && s.bindings[i].set == tex_set && s.bindings[i].binding == tex_binding) {
                         s.bindings.write[i].mirror_binding = samp_binding;
                     }
                 }
            }

            wgsl_code.push_back(s);
        } catch (const std::exception &e) {
            ERR_PRINT("SPIR-V Cross translation failed: " + String(e.what()));
            return false;
        }
    }
    
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
