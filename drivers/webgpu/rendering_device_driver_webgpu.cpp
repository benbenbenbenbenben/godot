#include "rendering_device_driver_webgpu.h"
#include "rendering_context_driver_webgpu.h"
#include "rendering_shader_container_webgpu.h"
#include "servers/rendering/rendering_shader_container.h"

#ifdef __EMSCRIPTEN__
#include <webgpu/webgpu.h>
#endif

RenderingDeviceDriverWebGPU::RenderingDeviceDriverWebGPU(RenderingContextDriverWebGPU *p_context_driver) :
		context_driver(p_context_driver) {
}

RenderingDeviceDriverWebGPU::~RenderingDeviceDriverWebGPU() {
}

Error RenderingDeviceDriverWebGPU::initialize(uint32_t p_device_index, uint32_t p_frame_count) {
	return OK;
}

#ifdef __EMSCRIPTEN__
static WGPUBufferUsageFlags _get_buffer_usage(BitField<RenderingDeviceDriver::BufferUsageBits> p_usage) {
	WGPUBufferUsageFlags flags = WGPUBufferUsage_None;
	
	if (p_usage.has_flag(RenderingDeviceDriver::BUFFER_USAGE_TRANSFER_FROM_BIT)) {
		flags |= WGPUBufferUsage_CopySrc;
	}
	if (p_usage.has_flag(RenderingDeviceDriver::BUFFER_USAGE_TRANSFER_TO_BIT)) {
		flags |= WGPUBufferUsage_CopyDst;
	}
	if (p_usage.has_flag(RenderingDeviceDriver::BUFFER_USAGE_UNIFORM_BIT)) {
		flags |= WGPUBufferUsage_Uniform;
	}
	if (p_usage.has_flag(RenderingDeviceDriver::BUFFER_USAGE_STORAGE_BIT)) {
		flags |= WGPUBufferUsage_Storage;
	}
	if (p_usage.has_flag(RenderingDeviceDriver::BUFFER_USAGE_INDEX_BIT)) {
		flags |= WGPUBufferUsage_Index;
	}
	if (p_usage.has_flag(RenderingDeviceDriver::BUFFER_USAGE_VERTEX_BIT)) {
		flags |= WGPUBufferUsage_Vertex;
	}
	if (p_usage.has_flag(RenderingDeviceDriver::BUFFER_USAGE_INDIRECT_BIT)) {
		flags |= WGPUBufferUsage_Indirect;
	}
	
	return flags;
}
#endif

static WGPUTextureFormat _get_wgpu_texture_format(RenderingDeviceDriver::DataFormat p_format) {
	switch (p_format) {
		case RenderingDeviceDriver::DATA_FORMAT_R8_UNORM: return WGPUTextureFormat_R8Unorm;
		case RenderingDeviceDriver::DATA_FORMAT_R8_SNORM: return WGPUTextureFormat_R8Snorm;
		case RenderingDeviceDriver::DATA_FORMAT_R8_UINT: return WGPUTextureFormat_R8Uint;
		case RenderingDeviceDriver::DATA_FORMAT_R8_SINT: return WGPUTextureFormat_R8Sint;
		
		case RenderingDeviceDriver::DATA_FORMAT_R16_UINT: return WGPUTextureFormat_R16Uint;
		case RenderingDeviceDriver::DATA_FORMAT_R16_SINT: return WGPUTextureFormat_R16Sint;
		case RenderingDeviceDriver::DATA_FORMAT_R16_SFLOAT: return WGPUTextureFormat_R16Float;

		case RenderingDeviceDriver::DATA_FORMAT_R8G8_UNORM: return WGPUTextureFormat_RG8Unorm;
		case RenderingDeviceDriver::DATA_FORMAT_R8G8_SNORM: return WGPUTextureFormat_RG8Snorm;
		case RenderingDeviceDriver::DATA_FORMAT_R8G8_UINT: return WGPUTextureFormat_RG8Uint;
		case RenderingDeviceDriver::DATA_FORMAT_R8G8_SINT: return WGPUTextureFormat_RG8Sint;

		case RenderingDeviceDriver::DATA_FORMAT_R16G16_UINT: return WGPUTextureFormat_RG16Uint;
		case RenderingDeviceDriver::DATA_FORMAT_R16G16_SINT: return WGPUTextureFormat_RG16Sint;
		case RenderingDeviceDriver::DATA_FORMAT_R16G16_SFLOAT: return WGPUTextureFormat_RG16Float;

		case RenderingDeviceDriver::DATA_FORMAT_R32_UINT: return WGPUTextureFormat_R32Uint;
		case RenderingDeviceDriver::DATA_FORMAT_R32_SINT: return WGPUTextureFormat_R32Sint;
		case RenderingDeviceDriver::DATA_FORMAT_R32_SFLOAT: return WGPUTextureFormat_R32Float;

		case RenderingDeviceDriver::DATA_FORMAT_R32G32_UINT: return WGPUTextureFormat_RG32Uint;
		case RenderingDeviceDriver::DATA_FORMAT_R32G32_SINT: return WGPUTextureFormat_RG32Sint;
		case RenderingDeviceDriver::DATA_FORMAT_R32G32_SFLOAT: return WGPUTextureFormat_RG32Float;

		case RenderingDeviceDriver::DATA_FORMAT_R8G8B8A8_UNORM: return WGPUTextureFormat_RGBA8Unorm;
		case RenderingDeviceDriver::DATA_FORMAT_R8G8B8A8_SNORM: return WGPUTextureFormat_RGBA8Snorm;
		case RenderingDeviceDriver::DATA_FORMAT_R8G8B8A8_UINT: return WGPUTextureFormat_RGBA8Uint;
		case RenderingDeviceDriver::DATA_FORMAT_R8G8B8A8_SINT: return WGPUTextureFormat_RGBA8Sint;
		case RenderingDeviceDriver::DATA_FORMAT_R8G8B8A8_SRGB: return WGPUTextureFormat_RGBA8UnormSrgb;

		case RenderingDeviceDriver::DATA_FORMAT_B8G8R8A8_UNORM: return WGPUTextureFormat_BGRA8Unorm;
		case RenderingDeviceDriver::DATA_FORMAT_B8G8R8A8_SRGB: return WGPUTextureFormat_BGRA8UnormSrgb;

		case RenderingDeviceDriver::DATA_FORMAT_R16G16B16A16_UINT: return WGPUTextureFormat_RGBA16Uint;
		case RenderingDeviceDriver::DATA_FORMAT_R16G16B16A16_SINT: return WGPUTextureFormat_RGBA16Sint;
		case RenderingDeviceDriver::DATA_FORMAT_R16G16B16A16_SFLOAT: return WGPUTextureFormat_RGBA16Float;

		case RenderingDeviceDriver::DATA_FORMAT_R32G32B32A32_UINT: return WGPUTextureFormat_RGBA32Uint;
		case RenderingDeviceDriver::DATA_FORMAT_R32G32B32A32_SINT: return WGPUTextureFormat_RGBA32Sint;
		case RenderingDeviceDriver::DATA_FORMAT_R32G32B32A32_SFLOAT: return WGPUTextureFormat_RGBA32Float;

		case RenderingDeviceDriver::DATA_FORMAT_D32_SFLOAT: return WGPUTextureFormat_Depth32Float;
		case RenderingDeviceDriver::DATA_FORMAT_D16_UNORM: return WGPUTextureFormat_Depth16Unorm;
		case RenderingDeviceDriver::DATA_FORMAT_D24_UNORM_S8_UINT: return WGPUTextureFormat_Depth24PlusStencil8;
		
		// TODO: Add compressed formats (BC, ASTC if supported/enabled)
		default: return WGPUTextureFormat_Undefined;
	}
}

static WGPUTextureUsageFlags _get_wgpu_texture_usage(BitField<RenderingDeviceDriver::TextureUsageBits> p_usage) {
	WGPUTextureUsageFlags flags = WGPUTextureUsage_None;
	
	if (p_usage.has_flag(RenderingDeviceDriver::TEXTURE_USAGE_SAMPLING_BIT)) flags |= WGPUTextureUsage_TextureBinding;
	if (p_usage.has_flag(RenderingDeviceDriver::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT)) flags |= WGPUTextureUsage_RenderAttachment;
	if (p_usage.has_flag(RenderingDeviceDriver::TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) flags |= WGPUTextureUsage_RenderAttachment;
	if (p_usage.has_flag(RenderingDeviceDriver::TEXTURE_USAGE_STORAGE_BIT)) flags |= WGPUTextureUsage_StorageBinding;
	if (p_usage.has_flag(RenderingDeviceDriver::TEXTURE_USAGE_CAN_COPY_FROM_BIT)) flags |= WGPUTextureUsage_CopySrc;
	if (p_usage.has_flag(RenderingDeviceDriver::TEXTURE_USAGE_CAN_COPY_TO_BIT)) flags |= WGPUTextureUsage_CopyDst;
	
	return flags;
}

static WGPUTextureDimension _get_wgpu_texture_dimension(RenderingDeviceDriver::TextureType p_type) {
	switch (p_type) {
		case RenderingDeviceDriver::TEXTURE_TYPE_1D:
		case RenderingDeviceDriver::TEXTURE_TYPE_1D_ARRAY:
			return WGPUTextureDimension_1D;
		case RenderingDeviceDriver::TEXTURE_TYPE_2D:
		case RenderingDeviceDriver::TEXTURE_TYPE_2D_ARRAY:
		case RenderingDeviceDriver::TEXTURE_TYPE_CUBE:
		case RenderingDeviceDriver::TEXTURE_TYPE_CUBE_ARRAY:
			return WGPUTextureDimension_2D;
		case RenderingDeviceDriver::TEXTURE_TYPE_3D:
			return WGPUTextureDimension_3D;
		default:
			return WGPUTextureDimension_2D;
	}
}

static WGPUTextureViewDimension _get_wgpu_texture_view_dimension(RenderingDeviceDriver::TextureType p_type) {
	switch (p_type) {
		case RenderingDeviceDriver::TEXTURE_TYPE_1D: return WGPUTextureViewDimension_1D;
		case RenderingDeviceDriver::TEXTURE_TYPE_1D_ARRAY: return WGPUTextureViewDimension_1D; // WebGPU doesn't explicitly distinguish 1D Array view dim? Wrapper says WGPUTextureViewDimension_1D
		case RenderingDeviceDriver::TEXTURE_TYPE_2D: return WGPUTextureViewDimension_2D;
		case RenderingDeviceDriver::TEXTURE_TYPE_2D_ARRAY: return WGPUTextureViewDimension_2DArray;
		case RenderingDeviceDriver::TEXTURE_TYPE_CUBE: return WGPUTextureViewDimension_Cube;
		case RenderingDeviceDriver::TEXTURE_TYPE_CUBE_ARRAY: return WGPUTextureViewDimension_CubeArray;
		case RenderingDeviceDriver::TEXTURE_TYPE_3D: return WGPUTextureViewDimension_3D;
		default: return WGPUTextureViewDimension_2D;
	}
}

static uint32_t _get_wgpu_sample_count(RenderingDeviceDriver::TextureSamples p_samples) {
	switch (p_samples) {
		case RenderingDeviceDriver::TEXTURE_SAMPLES_1: return 1;
		case RenderingDeviceDriver::TEXTURE_SAMPLES_2: return 2; // WebGPU usually only supports 1 or 4?
		case RenderingDeviceDriver::TEXTURE_SAMPLES_4: return 4;
		case RenderingDeviceDriver::TEXTURE_SAMPLES_8: return 8; // Maybe not supported
		case RenderingDeviceDriver::TEXTURE_SAMPLES_16: return 16;
		default: return 1;
	}
}

RDD::BufferID RenderingDeviceDriverWebGPU::buffer_create(uint64_t p_size, BitField<BufferUsageBits> p_usage, MemoryAllocationType p_allocation_type, uint64_t p_frames_drawn) {
#ifdef __EMSCRIPTEN__
	WebGPUBuffer *buffer_state = buffer_allocator.alloc();
	if (!buffer_state) {
		return BufferID();
	}
	
	buffer_state->size = p_size;
	buffer_state->is_cpu_writable = (p_allocation_type == MEMORY_ALLOCATION_TYPE_CPU); // Simplified check, might need to check usage bits too.

	WGPUBufferDescriptor desc = {};
	desc.label = nullptr;
	desc.usage = _get_buffer_usage(p_usage);
	
	// If we are emulating mapping, we need CopyDst to be able to upload data via queueWriteBuffer
	if (buffer_state->is_cpu_writable) {
		desc.usage |= WGPUBufferUsage_CopyDst;
		buffer_state->cpu_staging_ptr = (uint8_t *)memalloc(p_size);
	} else {
		buffer_state->cpu_staging_ptr = nullptr;
	}

	desc.size = p_size;
	desc.mappedAtCreation = false; // We use our own staging buffer for sync mapping

	WGPUDevice device = context_driver->get_wgpu_device();
	if (!device) {
		if (buffer_state->cpu_staging_ptr) memfree(buffer_state->cpu_staging_ptr);
		buffer_allocator.free(buffer_state);
		return BufferID();
	}

	buffer_state->handle = wgpuDeviceCreateBuffer(device, &desc);
	
	return BufferID(buffer_state);
#else
	return BufferID();
#endif
}

bool RenderingDeviceDriverWebGPU::buffer_set_texel_format(BufferID p_buffer, DataFormat p_format) {
	return false;
}

void RenderingDeviceDriverWebGPU::buffer_free(BufferID p_buffer) {
#ifdef __EMSCRIPTEN__
	WebGPUBuffer *buffer_state = (WebGPUBuffer *)p_buffer.id;
	if (buffer_state) {
		if (buffer_state->handle) {
			wgpuBufferDestroy(buffer_state->handle);
			wgpuBufferRelease(buffer_state->handle);
		}
		if (buffer_state->cpu_staging_ptr) {
			memfree(buffer_state->cpu_staging_ptr);
		}
		buffer_allocator.free(buffer_state);
	}
#endif
}

uint64_t RenderingDeviceDriverWebGPU::buffer_get_allocation_size(BufferID p_buffer) {
#ifdef __EMSCRIPTEN__
	WebGPUBuffer *buffer_state = (WebGPUBuffer *)p_buffer.id;
	if (buffer_state) {
		return buffer_state->size;
	}
#endif
	return 0;
}

uint8_t *RenderingDeviceDriverWebGPU::buffer_map(BufferID p_buffer) {
#ifdef __EMSCRIPTEN__
	WebGPUBuffer *buffer_state = (WebGPUBuffer *)p_buffer.id;
	if (buffer_state && buffer_state->is_cpu_writable) {
		return buffer_state->cpu_staging_ptr;
	}
#endif
	return nullptr;
}

void RenderingDeviceDriverWebGPU::buffer_unmap(BufferID p_buffer) {
#ifdef __EMSCRIPTEN__
	WebGPUBuffer *buffer_state = (WebGPUBuffer *)p_buffer.id;
	if (buffer_state && buffer_state->is_cpu_writable && buffer_state->handle) {
		// Upload the data from staging pointer to the GPU buffer
		WGPUDevice device = context_driver->get_wgpu_device();
		WGPUQueue queue = wgpuDeviceGetQueue(device);
		wgpuQueueWriteBuffer(queue, buffer_state->handle, 0, buffer_state->cpu_staging_ptr, buffer_state->size);
		// Note: wgpuQueueWriteBuffer is synchronous-ish (copies data immediately), so it's safe.
	}
#endif
}

uint8_t *RenderingDeviceDriverWebGPU::buffer_persistent_map_advance(BufferID p_buffer, uint64_t p_frames_drawn) {
	return nullptr;
}

uint64_t RenderingDeviceDriverWebGPU::buffer_get_dynamic_offsets(Span<BufferID> p_buffers) {
	return 0;
}

uint64_t RenderingDeviceDriverWebGPU::buffer_get_device_address(BufferID p_buffer) {
	return 0;
}

RDD::TextureID RenderingDeviceDriverWebGPU::texture_create(const TextureFormat &p_format, const TextureView &p_view) {
#ifdef __EMSCRIPTEN__
	WebGPUTexture *tex = texture_allocator.alloc();
	if (!tex) return TextureID();

	tex->is_own_handle = true;
	tex->width = p_format.width;
	tex->height = p_format.height;
	tex->depth = p_format.depth;
	tex->mips = p_format.mipmaps;
	tex->layers = p_format.array_layers;
	tex->format = _get_wgpu_texture_format(p_format.format);
	
	WGPUTextureDescriptor desc = {};
	desc.usage = _get_wgpu_texture_usage(p_format.usage_bits);
	desc.dimension = _get_wgpu_texture_dimension(p_format.texture_type);
	desc.size.width = p_format.width;
	desc.size.height = p_format.height;
	desc.size.depthOrArrayLayers = (p_format.texture_type == TEXTURE_TYPE_3D) ? p_format.depth : p_format.array_layers;
	desc.format = tex->format;
	desc.mipLevelCount = p_format.mipmaps;
	desc.sampleCount = _get_wgpu_sample_count(p_format.samples); // Helper needed

    if (desc.format == WGPUTextureFormat_Undefined) {
        texture_allocator.free(tex);
        return TextureID();
    }
	
	tex->handle = wgpuDeviceCreateTexture(context_driver->get_wgpu_device(), &desc);
	
    if (!tex->handle) {
        texture_allocator.free(tex);
        return TextureID();
    }

	// Create default view
	WGPUTextureViewDescriptor view_desc = {};
	view_desc.format = tex->format;
	view_desc.dimension = _get_wgpu_texture_view_dimension(p_format.texture_type);
	view_desc.baseMipLevel = 0;
	view_desc.mipLevelCount = p_format.mipmaps;
	view_desc.baseArrayLayer = 0;
	view_desc.arrayLayerCount = p_format.array_layers;
    view_desc.aspect = WGPUTextureAspect_All; 

	tex->view = wgpuTextureCreateView(tex->handle, &view_desc);
	
	return TextureID(tex);
#else
	return TextureID();
#endif
}

RDD::TextureID RenderingDeviceDriverWebGPU::texture_create_from_extension(uint64_t p_native_texture, TextureType p_type, DataFormat p_format, uint32_t p_array_layers, bool p_depth_stencil, uint32_t p_mipmaps) {
	return TextureID();
}

RDD::TextureID RenderingDeviceDriverWebGPU::texture_create_shared(TextureID p_original_texture, const TextureView &p_view) {
    // TODO: Implement valid shared views (aliasing)
	return TextureID();
}

RDD::TextureID RenderingDeviceDriverWebGPU::texture_create_shared_from_slice(TextureID p_original_texture, const TextureView &p_view, TextureSliceType p_slice_type, uint32_t p_layer, uint32_t p_layers, uint32_t p_mipmap, uint32_t p_mipmaps) {
    // TODO: Implement valid shared views (aliasing)
	return TextureID();
}

void RenderingDeviceDriverWebGPU::texture_free(TextureID p_texture) {
#ifdef __EMSCRIPTEN__
	WebGPUTexture *tex = (WebGPUTexture *)p_texture.id;
    if (tex) {
        if (tex->view) {
            wgpuTextureViewRelease(tex->view);
        }
        if (tex->is_own_handle && tex->handle) {
            wgpuTextureDestroy(tex->handle);
            wgpuTextureRelease(tex->handle);
        } else if (tex->handle) {
            // Shared handle, just release reference? 
            // In WGPU release decrements refcount.
             wgpuTextureRelease(tex->handle);
        }
        texture_allocator.free(tex);
    }
#endif
}

uint64_t RenderingDeviceDriverWebGPU::texture_get_allocation_size(TextureID p_texture) {
	return 0;
}

void RenderingDeviceDriverWebGPU::texture_get_copyable_layout(TextureID p_texture, const TextureSubresource &p_subresource, TextureCopyableLayout *r_layout) {
}

Vector<uint8_t> RenderingDeviceDriverWebGPU::texture_get_data(TextureID p_texture, uint32_t p_layer) {
	return Vector<uint8_t>();
}

BitField<RDD::TextureUsageBits> RenderingDeviceDriverWebGPU::texture_get_usages_supported_by_format(DataFormat p_format, bool p_cpu_readable) {
	return BitField<TextureUsageBits>();
}

bool RenderingDeviceDriverWebGPU::texture_can_make_shared_with_format(TextureID p_texture, DataFormat p_format, bool &r_raw_reinterpretation) {
	return false;
}

#ifdef __EMSCRIPTEN__
static WGPUFilterMode _get_wgpu_filter_mode(RenderingDeviceDriver::SamplerFilter p_filter) {
    if (p_filter == RenderingDeviceDriver::SAMPLER_FILTER_LINEAR) return WGPUFilterMode_Linear;
    return WGPUFilterMode_Nearest;
}

static WGPUMipmapFilterMode _get_wgpu_mipmap_filter_mode(RenderingDeviceDriver::SamplerFilter p_filter) {
    if (p_filter == RenderingDeviceDriver::SAMPLER_FILTER_LINEAR) return WGPUMipmapFilterMode_Linear;
    return WGPUMipmapFilterMode_Nearest;
}

static WGPUAddressMode _get_wgpu_address_mode(RenderingDeviceDriver::SamplerRepeatMode p_mode) {
    switch(p_mode) {
        case RenderingDeviceDriver::SAMPLER_REPEAT_MODE_REPEAT: return WGPUAddressMode_Repeat;
        case RenderingDeviceDriver::SAMPLER_REPEAT_MODE_MIRRORED_REPEAT: return WGPUAddressMode_MirrorRepeat;
        case RenderingDeviceDriver::SAMPLER_REPEAT_MODE_CLAMP_TO_EDGE: return WGPUAddressMode_ClampToEdge;
        case RenderingDeviceDriver::SAMPLER_REPEAT_MODE_CLAMP_TO_BORDER: return WGPUAddressMode_ClampToEdge; // Not supported, fallback
        case RenderingDeviceDriver::SAMPLER_REPEAT_MODE_MIRROR_CLAMP_TO_EDGE: return WGPUAddressMode_ClampToEdge; // Not supported usually?
        default: return WGPUAddressMode_ClampToEdge;
    }
}

static WGPUCompareFunction _get_wgpu_compare_function(RenderingDeviceDriver::CompareOperator p_op) {
    switch (p_op) {
        case RenderingDeviceDriver::COMPARE_OP_NEVER: return WGPUCompareFunction_Never;
        case RenderingDeviceDriver::COMPARE_OP_LESS: return WGPUCompareFunction_Less;
        case RenderingDeviceDriver::COMPARE_OP_EQUAL: return WGPUCompareFunction_Equal;
        case RenderingDeviceDriver::COMPARE_OP_LESS_OR_EQUAL: return WGPUCompareFunction_LessEqual;
        case RenderingDeviceDriver::COMPARE_OP_GREATER: return WGPUCompareFunction_Greater;
        case RenderingDeviceDriver::COMPARE_OP_NOT_EQUAL: return WGPUCompareFunction_NotEqual;
        case RenderingDeviceDriver::COMPARE_OP_GREATER_OR_EQUAL: return WGPUCompareFunction_GreaterEqual;
        case RenderingDeviceDriver::COMPARE_OP_ALWAYS: return WGPUCompareFunction_Always;
        default: return WGPUCompareFunction_Always;
    }
}
#endif

RDD::SamplerID RenderingDeviceDriverWebGPU::sampler_create(const SamplerState &p_state) {
#ifdef __EMSCRIPTEN__
    WebGPUSampler *sampler = sampler_allocator.alloc();
    if (!sampler) return SamplerID();

    WGPUSamplerDescriptor desc = {};
    desc.magFilter = _get_wgpu_filter_mode(p_state.mag_filter);
    desc.minFilter = _get_wgpu_filter_mode(p_state.min_filter);
    desc.mipmapFilter = _get_wgpu_mipmap_filter_mode(p_state.mip_filter);
    desc.addressModeU = _get_wgpu_address_mode(p_state.repeat_u);
    desc.addressModeV = _get_wgpu_address_mode(p_state.repeat_v);
    desc.addressModeW = _get_wgpu_address_mode(p_state.repeat_w);
    desc.lodMinClamp = p_state.min_lod;
    desc.lodMaxClamp = p_state.max_lod;
    if (p_state.enable_compare) {
        desc.compare = _get_wgpu_compare_function(p_state.compare_op);
    } else {
        desc.compare = WGPUCompareFunction_Undefined;
    }
    desc.maxAnisotropy = p_state.use_anisotropy ? (uint16_t)p_state.anisotropy_max : 1;

    sampler->handle = wgpuDeviceCreateSampler(context_driver->get_wgpu_device(), &desc);
    return SamplerID(sampler);
#else
	return SamplerID();
#endif
}

void RenderingDeviceDriverWebGPU::sampler_free(SamplerID p_sampler) {
#ifdef __EMSCRIPTEN__
    WebGPUSampler *sampler = (WebGPUSampler *)p_sampler.id;
    if (sampler) {
        if (sampler->handle) {
            wgpuSamplerRelease(sampler->handle);
        }
        sampler_allocator.free(sampler);
    }
#endif
}

bool RenderingDeviceDriverWebGPU::sampler_is_format_supported_for_filter(DataFormat p_format, SamplerFilter p_filter) {
	return false;
}

RDD::VertexFormatID RenderingDeviceDriverWebGPU::vertex_format_create(Span<VertexAttribute> p_vertex_attribs, const VertexAttributeBindingsMap &p_vertex_bindings) {
	return VertexFormatID();
}

void RenderingDeviceDriverWebGPU::vertex_format_free(VertexFormatID p_vertex_format) {
}

void RenderingDeviceDriverWebGPU::command_pipeline_barrier(CommandBufferID p_cmd_buffer, BitField<PipelineStageBits> p_src_stages, BitField<PipelineStageBits> p_dst_stages, VectorView<MemoryAccessBarrier> p_memory_barriers, VectorView<BufferBarrier> p_buffer_barriers, VectorView<TextureBarrier> p_texture_barriers) {
}

RDD::FenceID RenderingDeviceDriverWebGPU::fence_create() {
	return FenceID();
}

Error RenderingDeviceDriverWebGPU::fence_wait(FenceID p_fence) {
	return OK;
}

void RenderingDeviceDriverWebGPU::fence_free(FenceID p_fence) {
}

RDD::SemaphoreID RenderingDeviceDriverWebGPU::semaphore_create() {
	return SemaphoreID();
}

void RenderingDeviceDriverWebGPU::semaphore_free(SemaphoreID p_semaphore) {
}

RDD::CommandQueueFamilyID RenderingDeviceDriverWebGPU::command_queue_family_get(BitField<CommandQueueFamilyBits> p_cmd_queue_family_bits, RenderingContextDriver::SurfaceID p_surface) {
	return CommandQueueFamilyID();
}

RDD::CommandQueueID RenderingDeviceDriverWebGPU::command_queue_create(CommandQueueFamilyID p_cmd_queue_family, bool p_identify_as_main_queue) {
	return CommandQueueID();
}

Error RenderingDeviceDriverWebGPU::command_queue_execute_and_present(CommandQueueID p_cmd_queue, VectorView<SemaphoreID> p_wait_semaphores, VectorView<CommandBufferID> p_cmd_buffers, VectorView<SemaphoreID> p_cmd_semaphores, FenceID p_cmd_fence, VectorView<SwapChainID> p_swap_chains) {
	return OK;
}

void RenderingDeviceDriverWebGPU::command_queue_free(CommandQueueID p_cmd_queue) {
}

RDD::CommandPoolID RenderingDeviceDriverWebGPU::command_pool_create(CommandQueueFamilyID p_cmd_queue_family, CommandBufferType p_cmd_buffer_type) {
	return CommandPoolID();
}

bool RenderingDeviceDriverWebGPU::command_pool_reset(CommandPoolID p_cmd_pool) {
	return true;
}

void RenderingDeviceDriverWebGPU::command_pool_free(CommandPoolID p_cmd_pool) {
}

RDD::CommandBufferID RenderingDeviceDriverWebGPU::command_buffer_create(CommandPoolID p_cmd_pool) {
	return CommandBufferID();
}

bool RenderingDeviceDriverWebGPU::command_buffer_begin(CommandBufferID p_cmd_buffer) {
	return true;
}

bool RenderingDeviceDriverWebGPU::command_buffer_begin_secondary(CommandBufferID p_cmd_buffer, RenderPassID p_render_pass, uint32_t p_subpass, FramebufferID p_framebuffer) {
	return true;
}

void RenderingDeviceDriverWebGPU::command_buffer_end(CommandBufferID p_cmd_buffer) {
}

void RenderingDeviceDriverWebGPU::command_buffer_execute_secondary(CommandBufferID p_cmd_buffer, VectorView<CommandBufferID> p_secondary_cmd_buffers) {
}

RDD::SwapChainID RenderingDeviceDriverWebGPU::swap_chain_create(RenderingContextDriver::SurfaceID p_surface) {
	return SwapChainID();
}

Error RenderingDeviceDriverWebGPU::swap_chain_resize(CommandQueueID p_cmd_queue, SwapChainID p_swap_chain, uint32_t p_desired_framebuffer_count) {
	return OK;
}

RDD::FramebufferID RenderingDeviceDriverWebGPU::swap_chain_acquire_framebuffer(CommandQueueID p_cmd_queue, SwapChainID p_swap_chain, bool &r_resize_required) {
	return FramebufferID();
}

RDD::RenderPassID RenderingDeviceDriverWebGPU::swap_chain_get_render_pass(SwapChainID p_swap_chain) {
	return RenderPassID();
}

RDD::DataFormat RenderingDeviceDriverWebGPU::swap_chain_get_format(SwapChainID p_swap_chain) {
	return DATA_FORMAT_R8G8B8A8_UNORM;
}

void RenderingDeviceDriverWebGPU::swap_chain_free(SwapChainID p_swap_chain) {
}

RDD::FramebufferID RenderingDeviceDriverWebGPU::framebuffer_create(RenderPassID p_render_pass, VectorView<TextureID> p_attachments, uint32_t p_width, uint32_t p_height) {
	return FramebufferID();
}

void RenderingDeviceDriverWebGPU::framebuffer_free(FramebufferID p_framebuffer) {
}

RDD::ShaderID RenderingDeviceDriverWebGPU::shader_create_from_container(const Ref<RenderingShaderContainer> &p_shader_container, const Vector<ImmutableSampler> &p_immutable_samplers) {
	return ShaderID();
}

void RenderingDeviceDriverWebGPU::shader_free(ShaderID p_shader) {
}

void RenderingDeviceDriverWebGPU::shader_destroy_modules(ShaderID p_shader) {
}

RDD::UniformSetID RenderingDeviceDriverWebGPU::uniform_set_create(VectorView<BoundUniform> p_uniforms, ShaderID p_shader, uint32_t p_set_index, int p_linear_pool_index) {
	return UniformSetID();
}

void RenderingDeviceDriverWebGPU::uniform_set_free(UniformSetID p_uniform_set) {
}

uint32_t RenderingDeviceDriverWebGPU::uniform_sets_get_dynamic_offsets(VectorView<UniformSetID> p_uniform_sets, ShaderID p_shader, uint32_t p_first_set_index, uint32_t p_set_count) const {
	return 0;
}

void RenderingDeviceDriverWebGPU::command_uniform_set_prepare_for_use(CommandBufferID p_cmd_buffer, UniformSetID p_uniform_set, ShaderID p_shader, uint32_t p_set_index) {
}

void RenderingDeviceDriverWebGPU::command_clear_buffer(CommandBufferID p_cmd_buffer, BufferID p_buffer, uint64_t p_offset, uint64_t p_size) {
}

void RenderingDeviceDriverWebGPU::command_copy_buffer(CommandBufferID p_cmd_buffer, BufferID p_src_buffer, BufferID p_dst_buffer, VectorView<BufferCopyRegion> p_regions) {
}

void RenderingDeviceDriverWebGPU::command_copy_texture(CommandBufferID p_cmd_buffer, TextureID p_src_texture, TextureLayout p_src_texture_layout, TextureID p_dst_texture, TextureLayout p_dst_texture_layout, VectorView<TextureCopyRegion> p_regions) {
}

void RenderingDeviceDriverWebGPU::command_resolve_texture(CommandBufferID p_cmd_buffer, TextureID p_src_texture, TextureLayout p_src_texture_layout, uint32_t p_src_layer, uint32_t p_src_mipmap, TextureID p_dst_texture, TextureLayout p_dst_texture_layout, uint32_t p_dst_layer, uint32_t p_dst_mipmap) {
}

void RenderingDeviceDriverWebGPU::command_clear_color_texture(CommandBufferID p_cmd_buffer, TextureID p_texture, TextureLayout p_texture_layout, const Color &p_color, const TextureSubresourceRange &p_subresources) {
}

void RenderingDeviceDriverWebGPU::command_copy_buffer_to_texture(CommandBufferID p_cmd_buffer, BufferID p_src_buffer, TextureID p_dst_texture, TextureLayout p_dst_texture_layout, VectorView<BufferTextureCopyRegion> p_regions) {
}

void RenderingDeviceDriverWebGPU::command_copy_texture_to_buffer(CommandBufferID p_cmd_buffer, TextureID p_src_texture, TextureLayout p_src_texture_layout, BufferID p_dst_buffer, VectorView<BufferTextureCopyRegion> p_regions) {
}

void RenderingDeviceDriverWebGPU::pipeline_free(PipelineID p_pipeline) {
}

void RenderingDeviceDriverWebGPU::command_bind_push_constants(CommandBufferID p_cmd_buffer, ShaderID p_shader, uint32_t p_first_index, VectorView<uint32_t> p_data) {
}

bool RenderingDeviceDriverWebGPU::pipeline_cache_create(const Vector<uint8_t> &p_data) {
	return true;
}

void RenderingDeviceDriverWebGPU::pipeline_cache_free() {
}

size_t RenderingDeviceDriverWebGPU::pipeline_cache_query_size() {
	return 0;
}

Vector<uint8_t> RenderingDeviceDriverWebGPU::pipeline_cache_serialize() {
	return Vector<uint8_t>();
}

RDD::RenderPassID RenderingDeviceDriverWebGPU::render_pass_create(VectorView<Attachment> p_attachments, VectorView<Subpass> p_subpasses, VectorView<SubpassDependency> p_subpass_dependencies, uint32_t p_view_count, AttachmentReference p_fragment_density_map_attachment) {
	return RenderPassID();
}

void RenderingDeviceDriverWebGPU::render_pass_free(RenderPassID p_render_pass) {
}

void RenderingDeviceDriverWebGPU::command_begin_render_pass(CommandBufferID p_cmd_buffer, RenderPassID p_render_pass, FramebufferID p_framebuffer, CommandBufferType p_cmd_buffer_type, const Rect2i &p_rect, VectorView<RenderPassClearValue> p_clear_values) {
}

void RenderingDeviceDriverWebGPU::command_end_render_pass(CommandBufferID p_cmd_buffer) {
}

void RenderingDeviceDriverWebGPU::command_next_render_subpass(CommandBufferID p_cmd_buffer, CommandBufferType p_cmd_buffer_type) {
}

void RenderingDeviceDriverWebGPU::command_render_set_viewport(CommandBufferID p_cmd_buffer, VectorView<Rect2i> p_viewports) {
}

void RenderingDeviceDriverWebGPU::command_render_set_scissor(CommandBufferID p_cmd_buffer, VectorView<Rect2i> p_scissors) {
}

void RenderingDeviceDriverWebGPU::command_render_clear_attachments(CommandBufferID p_cmd_buffer, VectorView<AttachmentClear> p_attachment_clears, VectorView<Rect2i> p_rects) {
}

void RenderingDeviceDriverWebGPU::command_bind_render_pipeline(CommandBufferID p_cmd_buffer, PipelineID p_pipeline) {
}

void RenderingDeviceDriverWebGPU::command_bind_render_uniform_sets(CommandBufferID p_cmd_buffer, VectorView<UniformSetID> p_uniform_sets, ShaderID p_shader, uint32_t p_first_set_index, uint32_t p_set_count, uint32_t p_dynamic_offsets) {
}

void RenderingDeviceDriverWebGPU::command_render_draw(CommandBufferID p_cmd_buffer, uint32_t p_vertex_count, uint32_t p_instance_count, uint32_t p_base_vertex, uint32_t p_first_instance) {
}

void RenderingDeviceDriverWebGPU::command_render_draw_indexed(CommandBufferID p_cmd_buffer, uint32_t p_index_count, uint32_t p_instance_count, uint32_t p_first_index, int32_t p_vertex_offset, uint32_t p_first_instance) {
}

void RenderingDeviceDriverWebGPU::command_render_draw_indexed_indirect(CommandBufferID p_cmd_buffer, BufferID p_indirect_buffer, uint64_t p_offset, uint32_t p_draw_count, uint32_t p_stride) {
}

void RenderingDeviceDriverWebGPU::command_render_draw_indexed_indirect_count(CommandBufferID p_cmd_buffer, BufferID p_indirect_buffer, uint64_t p_offset, BufferID p_count_buffer, uint64_t p_count_buffer_offset, uint32_t p_max_draw_count, uint32_t p_stride) {
}

void RenderingDeviceDriverWebGPU::command_render_draw_indirect(CommandBufferID p_cmd_buffer, BufferID p_indirect_buffer, uint64_t p_offset, uint32_t p_draw_count, uint32_t p_stride) {
}

void RenderingDeviceDriverWebGPU::command_render_draw_indirect_count(CommandBufferID p_cmd_buffer, BufferID p_indirect_buffer, uint64_t p_offset, BufferID p_count_buffer, uint64_t p_count_buffer_offset, uint32_t p_max_draw_count, uint32_t p_stride) {
}

void RenderingDeviceDriverWebGPU::command_render_bind_vertex_buffers(CommandBufferID p_cmd_buffer, uint32_t p_binding_count, const BufferID *p_buffers, const uint64_t *p_offsets, uint64_t p_dynamic_offsets) {
}

void RenderingDeviceDriverWebGPU::command_render_bind_index_buffer(CommandBufferID p_cmd_buffer, BufferID p_buffer, IndexBufferFormat p_format, uint64_t p_offset) {
}

void RenderingDeviceDriverWebGPU::command_render_set_blend_constants(CommandBufferID p_cmd_buffer, const Color &p_constants) {
}

void RenderingDeviceDriverWebGPU::command_render_set_line_width(CommandBufferID p_cmd_buffer, float p_width) {
}

RDD::PipelineID RenderingDeviceDriverWebGPU::render_pipeline_create(ShaderID p_shader, VertexFormatID p_vertex_format, RenderPrimitive p_render_primitive, PipelineRasterizationState p_rasterization_state, PipelineMultisampleState p_multisample_state, PipelineDepthStencilState p_depth_stencil_state, PipelineColorBlendState p_blend_state, VectorView<int32_t> p_color_attachments, BitField<PipelineDynamicStateFlags> p_dynamic_state, RenderPassID p_render_pass, uint32_t p_render_subpass, VectorView<PipelineSpecializationConstant> p_specialization_constants) {
	return PipelineID();
}

void RenderingDeviceDriverWebGPU::command_bind_compute_pipeline(CommandBufferID p_cmd_buffer, PipelineID p_pipeline) {
}

void RenderingDeviceDriverWebGPU::command_bind_compute_uniform_sets(CommandBufferID p_cmd_buffer, VectorView<UniformSetID> p_uniform_sets, ShaderID p_shader, uint32_t p_first_set_index, uint32_t p_set_count, uint32_t p_dynamic_offsets) {
}

void RenderingDeviceDriverWebGPU::command_compute_dispatch(CommandBufferID p_cmd_buffer, uint32_t p_x_groups, uint32_t p_y_groups, uint32_t p_z_groups) {
}

void RenderingDeviceDriverWebGPU::command_compute_dispatch_indirect(CommandBufferID p_cmd_buffer, BufferID p_indirect_buffer, uint64_t p_offset) {
}

RDD::PipelineID RenderingDeviceDriverWebGPU::compute_pipeline_create(ShaderID p_shader, VectorView<PipelineSpecializationConstant> p_specialization_constants) {
	return PipelineID();
}

RDD::QueryPoolID RenderingDeviceDriverWebGPU::timestamp_query_pool_create(uint32_t p_query_count) {
	return QueryPoolID();
}

void RenderingDeviceDriverWebGPU::timestamp_query_pool_free(QueryPoolID p_pool_id) {
}

void RenderingDeviceDriverWebGPU::timestamp_query_pool_get_results(QueryPoolID p_pool_id, uint32_t p_query_count, uint64_t *r_results) {
}

uint64_t RenderingDeviceDriverWebGPU::timestamp_query_result_to_time(uint64_t p_result) {
	return 0;
}

void RenderingDeviceDriverWebGPU::command_timestamp_query_pool_reset(CommandBufferID p_cmd_buffer, QueryPoolID p_pool_id, uint32_t p_query_count) {
}

void RenderingDeviceDriverWebGPU::command_timestamp_write(CommandBufferID p_cmd_buffer, QueryPoolID p_pool_id, uint32_t p_index) {
}

void RenderingDeviceDriverWebGPU::command_begin_label(CommandBufferID p_cmd_buffer, const char *p_label_name, const Color &p_color) {
}

void RenderingDeviceDriverWebGPU::command_end_label(CommandBufferID p_cmd_buffer) {
}

void RenderingDeviceDriverWebGPU::command_insert_breadcrumb(CommandBufferID p_cmd_buffer, uint32_t p_data) {
}

void RenderingDeviceDriverWebGPU::begin_segment(uint32_t p_frame_index, uint32_t p_frames_drawn) {
}

void RenderingDeviceDriverWebGPU::end_segment() {
}

void RenderingDeviceDriverWebGPU::set_object_name(ObjectType p_type, ID p_driver_id, const String &p_name) {
}

uint64_t RenderingDeviceDriverWebGPU::get_resource_native_handle(DriverResource p_type, ID p_driver_id) {
	return 0;
}

uint64_t RenderingDeviceDriverWebGPU::get_total_memory_used() {
	return 0;
}

uint64_t RenderingDeviceDriverWebGPU::get_lazily_memory_used() {
	return 0;
}

uint64_t RenderingDeviceDriverWebGPU::limit_get(Limit p_limit) {
	return 0;
}

bool RenderingDeviceDriverWebGPU::has_feature(Features p_feature) {
	return false;
}

const RDD::MultiviewCapabilities &RenderingDeviceDriverWebGPU::get_multiview_capabilities() {
	return multiview_capabilities;
}

const RDD::FragmentShadingRateCapabilities &RenderingDeviceDriverWebGPU::get_fragment_shading_rate_capabilities() {
	return fragment_shading_rate_capabilities;
}

const RDD::FragmentDensityMapCapabilities &RenderingDeviceDriverWebGPU::get_fragment_density_map_capabilities() {
	return fragment_density_map_capabilities;
}

String RenderingDeviceDriverWebGPU::get_api_name() const {
	return "WebGPU";
}

String RenderingDeviceDriverWebGPU::get_api_version() const {
	return "1.0";
}

String RenderingDeviceDriverWebGPU::get_pipeline_cache_uuid() const {
	return "webgpu_uuid";
}

const RDD::Capabilities &RenderingDeviceDriverWebGPU::get_capabilities() const {
	return capabilities;
}

const RenderingShaderContainerFormat &RenderingDeviceDriverWebGPU::get_shader_container_format() const {
	static RenderingShaderContainerFormatWebGPU format;
	return format;
}
