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

#ifdef __EMSCRIPTEN__
static WGPUVertexFormat _get_wgpu_vertex_format(RenderingDeviceDriver::DataFormat p_format) {
    switch (p_format) {
        case RenderingDeviceDriver::DATA_FORMAT_R32_SFLOAT: return WGPUVertexFormat_Float32;
        case RenderingDeviceDriver::DATA_FORMAT_R32G32_SFLOAT: return WGPUVertexFormat_Float32x2;
        case RenderingDeviceDriver::DATA_FORMAT_R32G32B32_SFLOAT: return WGPUVertexFormat_Float32x3;
        case RenderingDeviceDriver::DATA_FORMAT_R32G32B32A32_SFLOAT: return WGPUVertexFormat_Float32x4;
        case RenderingDeviceDriver::DATA_FORMAT_R8G8B8A8_UNORM: return WGPUVertexFormat_Unorm8x4;
        case RenderingDeviceDriver::DATA_FORMAT_R8G8B8A8_SNORM: return WGPUVertexFormat_Snorm8x4;
        case RenderingDeviceDriver::DATA_FORMAT_R8G8B8A8_UINT: return WGPUVertexFormat_Uint8x4;
        case RenderingDeviceDriver::DATA_FORMAT_R8G8B8A8_SINT: return WGPUVertexFormat_Sint8x4;
        case RenderingDeviceDriver::DATA_FORMAT_R16G16_SFLOAT: return WGPUVertexFormat_Float16x2;
        case RenderingDeviceDriver::DATA_FORMAT_R16G16B16A16_SFLOAT: return WGPUVertexFormat_Float16x4;
        case RenderingDeviceDriver::DATA_FORMAT_R16G16_SINT: return WGPUVertexFormat_Sint16x2;
        case RenderingDeviceDriver::DATA_FORMAT_R16G16B16A16_SINT: return WGPUVertexFormat_Sint16x4;
        case RenderingDeviceDriver::DATA_FORMAT_R16G16_UINT: return WGPUVertexFormat_Uint16x2;
        case RenderingDeviceDriver::DATA_FORMAT_R16G16B16A16_UINT: return WGPUVertexFormat_Uint16x4;
        case RenderingDeviceDriver::DATA_FORMAT_R32_UINT: return WGPUVertexFormat_Uint32;
        case RenderingDeviceDriver::DATA_FORMAT_R32G32_UINT: return WGPUVertexFormat_Uint32x2;
        case RenderingDeviceDriver::DATA_FORMAT_R32G32B32_UINT: return WGPUVertexFormat_Uint32x3;
        case RenderingDeviceDriver::DATA_FORMAT_R32G32B32A32_UINT: return WGPUVertexFormat_Uint32x4;
        case RenderingDeviceDriver::DATA_FORMAT_R32_SINT: return WGPUVertexFormat_Sint32;
        case RenderingDeviceDriver::DATA_FORMAT_R32G32_SINT: return WGPUVertexFormat_Sint32x2;
        case RenderingDeviceDriver::DATA_FORMAT_R32G32B32_SINT: return WGPUVertexFormat_Sint32x3;
        case RenderingDeviceDriver::DATA_FORMAT_R32G32B32A32_SINT: return WGPUVertexFormat_Sint32x4;
        // ... Add more as needed
        default: return WGPUVertexFormat_Float32x4; // Fallback or Error
    }
}

static WGPUVertexStepMode _get_wgpu_vertex_step_mode(RenderingDeviceDriver::VertexFrequency p_freq) {
    switch(p_freq) {
        case RenderingDeviceDriver::VERTEX_FREQUENCY_VERTEX: return WGPUVertexStepMode_Vertex;
        case RenderingDeviceDriver::VERTEX_FREQUENCY_INSTANCE: return WGPUVertexStepMode_Instance;
        default: return WGPUVertexStepMode_Vertex;
    }
}

static WGPUPrimitiveTopology _get_wgpu_primitive_topology(RenderingDeviceDriver::RenderPrimitive p_primitive) {
    switch (p_primitive) {
        case RenderingDeviceDriver::RENDER_PRIMITIVE_POINTS: return WGPUPrimitiveTopology_PointList;
        case RenderingDeviceDriver::RENDER_PRIMITIVE_LINES: return WGPUPrimitiveTopology_LineList;
        case RenderingDeviceDriver::RENDER_PRIMITIVE_LINES_WITH_ADJACENCY: return WGPUPrimitiveTopology_LineList; // Not fully supported?
        case RenderingDeviceDriver::RENDER_PRIMITIVE_LINE_STRIPS: return WGPUPrimitiveTopology_LineStrip;
        case RenderingDeviceDriver::RENDER_PRIMITIVE_LINE_STRIPS_WITH_ADJACENCY: return WGPUPrimitiveTopology_LineStrip;
        case RenderingDeviceDriver::RENDER_PRIMITIVE_TRIANGLES: return WGPUPrimitiveTopology_TriangleList;
        case RenderingDeviceDriver::RENDER_PRIMITIVE_TRIANGLES_WITH_ADJACENCY: return WGPUPrimitiveTopology_TriangleList;
        case RenderingDeviceDriver::RENDER_PRIMITIVE_TRIANGLE_STRIPS: return WGPUPrimitiveTopology_TriangleStrip;
        case RenderingDeviceDriver::RENDER_PRIMITIVE_TRIANGLE_STRIPS_WITH_ADJACENCY: return WGPUPrimitiveTopology_TriangleStrip; // Fix or fallback
         // TODO: Adjacency and Patch list support in WebGPU? (Usually not supported)
        default: return WGPUPrimitiveTopology_TriangleList;
    }
}

static WGPUFrontFace _get_wgpu_front_face(RenderingDeviceDriver::PipelineRasterizationState::FrontFace p_front) {
    switch(p_front) {
        case RenderingDeviceDriver::PipelineRasterizationState::FRONT_FACE_CCW: return WGPUFrontFace_CCW;
        case RenderingDeviceDriver::PipelineRasterizationState::FRONT_FACE_CW: return WGPUFrontFace_CW;
        default: return WGPUFrontFace_CCW;
    }
}

static WGPUCullMode _get_wgpu_cull_mode(RenderingDeviceDriver::PipelineRasterizationState::CullMode p_mode) {
    switch(p_mode) {
        case RenderingDeviceDriver::PipelineRasterizationState::CULL_MODE_DISABLED: return WGPUCullMode_None;
        case RenderingDeviceDriver::PipelineRasterizationState::CULL_MODE_FRONT: return WGPUCullMode_Front;
        case RenderingDeviceDriver::PipelineRasterizationState::CULL_MODE_BACK: return WGPUCullMode_Back;
        default: return WGPUCullMode_None;
    }
}

static WGPUCompareFunction _get_wgpu_compare_function(RenderingDeviceDriver::CompareOperator p_op) {
    switch(p_op) {
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

static WGPUStencilOperation _get_wgpu_stencil_operation(RenderingDeviceDriver::StencilOperation p_op) {
    switch(p_op) {
        case RenderingDeviceDriver::STENCIL_OP_KEEP: return WGPUStencilOperation_Keep;
        case RenderingDeviceDriver::STENCIL_OP_ZERO: return WGPUStencilOperation_Zero;
        case RenderingDeviceDriver::STENCIL_OP_REPLACE: return WGPUStencilOperation_Replace;
        case RenderingDeviceDriver::STENCIL_OP_INCREMENT_AND_CLAMP: return WGPUStencilOperation_IncrementClamp;
        case RenderingDeviceDriver::STENCIL_OP_DECREMENT_AND_CLAMP: return WGPUStencilOperation_DecrementClamp;
        case RenderingDeviceDriver::STENCIL_OP_INVERT: return WGPUStencilOperation_Invert;
        case RenderingDeviceDriver::STENCIL_OP_INCREMENT_AND_WRAP: return WGPUStencilOperation_IncrementWrap;
        case RenderingDeviceDriver::STENCIL_OP_DECREMENT_AND_WRAP: return WGPUStencilOperation_DecrementWrap;
        default: return WGPUStencilOperation_Keep;
    }
}

static WGPUBlendFactor _get_wgpu_blend_factor(RenderingDeviceDriver::BlendFactor p_factor) {
    switch(p_factor) {
        case RenderingDeviceDriver::BLEND_FACTOR_ZERO: return WGPUBlendFactor_Zero;
        case RenderingDeviceDriver::BLEND_FACTOR_ONE: return WGPUBlendFactor_One;
        case RenderingDeviceDriver::BLEND_FACTOR_SRC_COLOR: return WGPUBlendFactor_Src;
        case RenderingDeviceDriver::BLEND_FACTOR_ONE_MINUS_SRC_COLOR: return WGPUBlendFactor_OneMinusSrc;
        case RenderingDeviceDriver::BLEND_FACTOR_DST_COLOR: return WGPUBlendFactor_Dst;
        case RenderingDeviceDriver::BLEND_FACTOR_ONE_MINUS_DST_COLOR: return WGPUBlendFactor_OneMinusDst;
        case RenderingDeviceDriver::BLEND_FACTOR_SRC_ALPHA: return WGPUBlendFactor_SrcAlpha;
        case RenderingDeviceDriver::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA: return WGPUBlendFactor_OneMinusSrcAlpha;
        case RenderingDeviceDriver::BLEND_FACTOR_DST_ALPHA: return WGPUBlendFactor_DstAlpha;
        case RenderingDeviceDriver::BLEND_FACTOR_ONE_MINUS_DST_ALPHA: return WGPUBlendFactor_OneMinusDstAlpha;
        case RenderingDeviceDriver::BLEND_FACTOR_CONSTANT_COLOR: return WGPUBlendFactor_Constant;
        case RenderingDeviceDriver::BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR: return WGPUBlendFactor_OneMinusConstant;
        case RenderingDeviceDriver::BLEND_FACTOR_CONSTANT_ALPHA: return WGPUBlendFactor_Constant; // WebGPU merges constant color/alpha?
        case RenderingDeviceDriver::BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA: return WGPUBlendFactor_OneMinusConstant;
        case RenderingDeviceDriver::BLEND_FACTOR_SRC_ALPHA_SATURATE: return WGPUBlendFactor_SrcAlphaSaturated;
        case RenderingDeviceDriver::BLEND_FACTOR_SRC1_COLOR: return WGPUBlendFactor_Src1; // Dual source blending?
        case RenderingDeviceDriver::BLEND_FACTOR_ONE_MINUS_SRC1_COLOR: return WGPUBlendFactor_OneMinusSrc1;
        case RenderingDeviceDriver::BLEND_FACTOR_SRC1_ALPHA: return WGPUBlendFactor_Src1Alpha;
        case RenderingDeviceDriver::BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA: return WGPUBlendFactor_OneMinusSrc1Alpha;
        default: return WGPUBlendFactor_One;
    }
}

static WGPUBlendOperation _get_wgpu_blend_operation(RenderingDeviceDriver::BlendOperation p_op) {
    switch(p_op) {
        case RenderingDeviceDriver::BLEND_OP_ADD: return WGPUBlendOperation_Add;
        case RenderingDeviceDriver::BLEND_OP_SUBTRACT: return WGPUBlendOperation_Subtract;
        case RenderingDeviceDriver::BLEND_OP_REVERSE_SUBTRACT: return WGPUBlendOperation_ReverseSubtract;
        case RenderingDeviceDriver::BLEND_OP_MIN: return WGPUBlendOperation_Min;
        case RenderingDeviceDriver::BLEND_OP_MAX: return WGPUBlendOperation_Max;
        default: return WGPUBlendOperation_Add;
    }
}

static WGPUColorWriteMask _get_wgpu_color_write_mask(BitField<RenderingDeviceDriver::ColorChannelBits> p_mask) {
    uint32_t mask = 0;
    if (p_mask.has_flag(RenderingDeviceDriver::COLOR_CHANNEL_R_BIT)) mask |= WGPUColorWriteMask_Red;
    if (p_mask.has_flag(RenderingDeviceDriver::COLOR_CHANNEL_G_BIT)) mask |= WGPUColorWriteMask_Green;
    if (p_mask.has_flag(RenderingDeviceDriver::COLOR_CHANNEL_B_BIT)) mask |= WGPUColorWriteMask_Blue;
    if (p_mask.has_flag(RenderingDeviceDriver::COLOR_CHANNEL_A_BIT)) mask |= WGPUColorWriteMask_Alpha;
    return (WGPUColorWriteMask)mask;
}

static uint32_t _get_wgpu_sample_count(RenderingDeviceDriver::TextureSamples p_samples) {
    switch(p_samples) {
        case RenderingDeviceDriver::TEXTURE_SAMPLES_1: return 1;
        case RenderingDeviceDriver::TEXTURE_SAMPLES_2: return 2; // WebGPU supports 2? Usually 1 or 4.
        case RenderingDeviceDriver::TEXTURE_SAMPLES_4: return 4;
        case RenderingDeviceDriver::TEXTURE_SAMPLES_8: return 8; // Maybe?
        case RenderingDeviceDriver::TEXTURE_SAMPLES_16: return 16; // Maybe?
        case RenderingDeviceDriver::TEXTURE_SAMPLES_32: return 32; // Unlikely
        case RenderingDeviceDriver::TEXTURE_SAMPLES_64: return 64; // Unlikely
        default: return 1;
    }
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
#ifdef __EMSCRIPTEN__
    WebGPUVertexFormat *fmt = vertex_format_allocator.alloc();
    memnew_placement(fmt, WebGPUVertexFormat);

    // Find max binding index to resize layouts vector
    uint32_t max_binding = 0;
    for (const KeyValue<uint32_t, VertexAttributeBinding> &E : p_vertex_bindings) {
        if (E.key > max_binding) max_binding = E.key;
    }
    
    // Resize layouts to accommodate max_binding (slots)
    // We strictly use binding index as slot index for simplicity and standard mapping
    fmt->layouts.resize(max_binding + 1);
    
    // Fill layout info
    for (const KeyValue<uint32_t, VertexAttributeBinding> &E : p_vertex_bindings) {
        WebGPUVertexFormat::BufferLayout &layout = fmt->layouts.write[E.key];
        layout.array_stride = E.value.stride;
        layout.step_mode = _get_wgpu_vertex_step_mode(E.value.frequency);
    }
    
    // Fill attributes
    for (const VertexAttribute &att : p_vertex_attribs) {
        // Godot allows binding to be UINT32_MAX, meaning "use index of attribute in layout"?
        // See definition: "When set to UINT32_MAX, it uses the index of the attribute in the layout."
        // We assume valid binding for now or handle this edge case.
        if (att.binding == UINT32_MAX) continue; // TODO: Implementation?

        if (att.binding < fmt->layouts.size()) {
             WGPUVertexAttribute wgpu_att = {};
             wgpu_att.format = _get_wgpu_vertex_format(att.format);
             wgpu_att.offset = att.offset;
             wgpu_att.shaderLocation = att.location;
             fmt->layouts.write[att.binding].attributes.push_back(wgpu_att);
        }
    }
    
    // Create final WGPUVertexBufferLayouts
    for (int i=0; i < fmt->layouts.size(); i++) {
        WGPUVertexBufferLayout wgpu_layout = {};
        wgpu_layout.arrayStride = fmt->layouts[i].array_stride;
        wgpu_layout.stepMode = fmt->layouts[i].step_mode;
        wgpu_layout.attributeCount = fmt->layouts[i].attributes.size();
        // If no attributes, pointers might be null, which is fine
        wgpu_layout.attributes = fmt->layouts[i].attributes.ptr();
        fmt->buffers.push_back(wgpu_layout);
    }
    
    return VertexFormatID(fmt);
#else
	return VertexFormatID();
#endif
}

void RenderingDeviceDriverWebGPU::vertex_format_free(VertexFormatID p_vertex_format) {
#ifdef __EMSCRIPTEN__
    WebGPUVertexFormat *fmt = (WebGPUVertexFormat *)p_vertex_format.id;
    if (fmt) {
        fmt->~WebGPUVertexFormat();
        vertex_format_allocator.free(fmt);
    }
#endif
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
#ifdef __EMSCRIPTEN__
    WGPUDevice device = context_driver->get_wgpu_device();
    if (!device) return ERR_INVALID_PARAMETER;

    // 1. Submit command buffers
    if (p_cmd_buffers.size() > 0) {
        WGPUQueue queue = wgpuDeviceGetQueue(device); // Get default queue
        // TODO: Handle p_cmd_queue if we support multiple queues (WebGPU usually has one per device for now in standard)

        // Collect valid WGPUCommandBuffers
        LocalVector<WGPUCommandBuffer> buffers;
        for (uint32_t i = 0; i < p_cmd_buffers.size(); i++) {
            WebGPUCommandBuffer *cmd_buf = (WebGPUCommandBuffer *)p_cmd_buffers[i].id;
            if (cmd_buf && cmd_buf->buffer) {
                buffers.push_back(cmd_buf->buffer);
            }
        }
        
        if (buffers.size() > 0) {
            wgpuQueueSubmit(queue, buffers.size(), buffers.ptr());
        }

        wgpuQueueRelease(queue);
    }

    // 2. Present (if needed/supported)
    // WebGPU presentation is usually implicit or managed via surface configuration 
    // but wgpuSurfacePresent is available in some bindings.
    // For now we assume implicit presentation at end of frame or handled by browser compositor.
#endif
	return OK;
}

void RenderingDeviceDriverWebGPU::command_queue_free(CommandQueueID p_cmd_queue) {
}

RDD::CommandPoolID RenderingDeviceDriverWebGPU::command_pool_create(CommandQueueFamilyID p_cmd_queue_family, CommandBufferType p_cmd_buffer_type) {
#ifdef __EMSCRIPTEN__
    WebGPUCommandPool *pool = command_pool_allocator.alloc();
    memnew_placement(pool, WebGPUCommandPool);
    pool->type = p_cmd_buffer_type;
    return CommandPoolID(pool);
#else
	return CommandPoolID();
#endif
}

bool RenderingDeviceDriverWebGPU::command_pool_reset(CommandPoolID p_cmd_pool) {
    // WebGPU encoders are one-shot. Pool reset is no-op or just logical.
	return true;
}

void RenderingDeviceDriverWebGPU::command_pool_free(CommandPoolID p_cmd_pool) {
#ifdef __EMSCRIPTEN__
    WebGPUCommandPool *pool = (WebGPUCommandPool *)p_cmd_pool.id;
    if (pool) {
        pool->~WebGPUCommandPool();
        command_pool_allocator.free(pool);
    }
#endif
}

RDD::CommandBufferID RenderingDeviceDriverWebGPU::command_buffer_create(CommandPoolID p_cmd_pool) {
#ifdef __EMSCRIPTEN__
    WebGPUCommandPool *pool = (WebGPUCommandPool *)p_cmd_pool.id;
    WebGPUCommandBuffer *cmd = command_buffer_allocator.alloc();
    memnew_placement(cmd, WebGPUCommandBuffer);
    cmd->pool = pool;
    return CommandBufferID(cmd);
#else
	return CommandBufferID();
#endif
}

bool RenderingDeviceDriverWebGPU::command_buffer_begin(CommandBufferID p_cmd_buffer) {
#ifdef __EMSCRIPTEN__
    WebGPUCommandBuffer *cmd = (WebGPUCommandBuffer *)p_cmd_buffer.id;
    if (!cmd) return false;

    if (cmd->buffer) {
        wgpuCommandBufferRelease(cmd->buffer);
        cmd->buffer = nullptr;
    }
    if (cmd->encoder) {
        wgpuCommandEncoderRelease(cmd->encoder);
    }
    
    WGPUCommandEncoderDescriptor desc = {};
    desc.label = nullptr; 
    cmd->encoder = wgpuDeviceCreateCommandEncoder(context_driver->get_wgpu_device(), &desc);
    cmd->is_recording = true;
    
	return cmd->encoder != nullptr;
#else
	return false;
#endif
}

bool RenderingDeviceDriverWebGPU::command_buffer_begin_secondary(CommandBufferID p_cmd_buffer, RenderPassID p_render_pass, uint32_t p_subpass, FramebufferID p_framebuffer) {
    // WebGPU Bundles support secondary-like behavior, but for now we ignore or implement later.
	return false;
}

void RenderingDeviceDriverWebGPU::command_buffer_end(CommandBufferID p_cmd_buffer) {
#ifdef __EMSCRIPTEN__
    WebGPUCommandBuffer *cmd = (WebGPUCommandBuffer *)p_cmd_buffer.id;
    if (cmd && cmd->encoder && cmd->is_recording) {
        WGPUCommandBufferDescriptor desc = {};
        cmd->buffer = wgpuCommandEncoderFinish(cmd->encoder, &desc);
        
        // Encoder is now finished/invalid, release it?
        // wgpuCommandEncoderFinish consumes the encoder? No, usually produce buffer.
        // Docs: "The encoder is usable until finish is called"?
        // Native headers: "After calling finish, the encoder is invalid".
        // Wrapper might handle release? Or we must release?
        // Usually we release the encoder wrapper after finish.
        wgpuCommandEncoderRelease(cmd->encoder);
        cmd->encoder = nullptr;
        cmd->is_recording = false;
    }
#endif
}

void RenderingDeviceDriverWebGPU::command_buffer_execute_secondary(CommandBufferID p_cmd_buffer, VectorView<CommandBufferID> p_secondary_cmd_buffers) {
    // Bundles ...
}

RDD::SwapChainID RenderingDeviceDriverWebGPU::swap_chain_create(RenderingContextDriver::SurfaceID p_surface) {
#ifdef __EMSCRIPTEN__
	WebGPUSwapChain *sc = swap_chain_allocator.alloc();
    memnew_placement(sc, WebGPUSwapChain);
	sc->surface = p_surface;
	sc->format = WGPUTextureFormat_BGRA8Unorm; // TODO: wgpuSurfaceGetPreferredFormat(surface, adapter);
    
    // Configure surface
    WGPUSurfaceConfiguration config = {};
    config.device = context_driver->get_wgpu_device();
    config.format = sc->format;
    config.usage = WGPUTextureUsage_RenderAttachment;
    config.viewFormatCount = 0;
    config.viewFormats = nullptr;
    config.alphaMode = WGPUCompositeAlphaMode_Auto; 
    config.width = context_driver->surface_get_width(p_surface);
    config.height = context_driver->surface_get_height(p_surface);
    config.presentMode = WGPUPresentMode_Fifo;
    
    wgpuSurfaceConfigure((WGPUSurface)sc->surface, &config);
    
	return SwapChainID(sc);
#else
	return SwapChainID();
#endif
}

Error RenderingDeviceDriverWebGPU::swap_chain_resize(CommandQueueID p_cmd_queue, SwapChainID p_swap_chain, uint32_t p_desired_framebuffer_count) {
#ifdef __EMSCRIPTEN__
    WebGPUSwapChain *sc = (WebGPUSwapChain *)p_swap_chain.id;
    if (!sc) return ERR_INVALID_PARAMETER;
    
    // Reconfigure surface with new size
    // Note: Godot usually calls resize when window size changes, so context driver usage implies current window size.
    WGPUSurfaceConfiguration config = {};
    config.device = context_driver->get_wgpu_device();
    config.format = sc->format;
    config.usage = WGPUTextureUsage_RenderAttachment;
    config.viewFormatCount = 0;
    config.viewFormats = nullptr;
    config.alphaMode = WGPUCompositeAlphaMode_Auto; 
    config.width = context_driver->surface_get_width(sc->surface);
    config.height = context_driver->surface_get_height(sc->surface);
    config.presentMode = WGPUPresentMode_Fifo;

    wgpuSurfaceConfigure((WGPUSurface)sc->surface, &config);
    return OK;
#else
	return OK;
#endif
}

RDD::FramebufferID RenderingDeviceDriverWebGPU::swap_chain_acquire_framebuffer(CommandQueueID p_cmd_queue, SwapChainID p_swap_chain, bool &r_resize_required) {
#ifdef __EMSCRIPTEN__
    WebGPUSwapChain *sc = (WebGPUSwapChain *)p_swap_chain.id;
    if (!sc) return FramebufferID();

    r_resize_required = false;

    // Get current texture
    WGPUSurfaceTexture surfaceTexture;
    wgpuSurfaceGetCurrentTexture((WGPUSurface)sc->surface, &surfaceTexture);

    if (surfaceTexture.status == WGPUSurfaceGetCurrentTextureStatus_Outdated || 
        surfaceTexture.status == WGPUSurfaceGetCurrentTextureStatus_Lost) {
        r_resize_required = true;
        return FramebufferID();
    }
    
    if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
        return FramebufferID();
    }

    // Release old texture wrapper if any
    if (sc->current_texture) {
        // Cleaning up the previous frame's texture wrapper. 
        // Note: we don't destroy the handle because it was a surface texture (owned by browser/swapchain), 
        // effectively we just release our view/ref. is_own_handle should be false.
        texture_free(TextureID(sc->current_texture));
    }

    // Wrap new texture
    WebGPUTexture *tex = texture_allocator.alloc();
    tex->handle = surfaceTexture.texture;
    tex->is_own_handle = false; // Surface owned
    tex->width = context_driver->surface_get_width(sc->surface);
    tex->height = context_driver->surface_get_height(sc->surface);
    tex->depth = 1;
    tex->mips = 1;
    tex->layers = 1;
    tex->format = sc->format;
    
    // Create view
    WGPUTextureViewDescriptor view_desc = {};
    view_desc.format = tex->format;
    view_desc.dimension = WGPUTextureViewDimension_2D;
    view_desc.baseMipLevel = 0;
    view_desc.mipLevelCount = 1;
    view_desc.baseArrayLayer = 0;
    view_desc.arrayLayerCount = 1;
    view_desc.aspect = WGPUTextureAspect_All;
    tex->view = wgpuTextureCreateView(tex->handle, &view_desc);

    sc->current_texture = tex;
    TextureID tex_id = TextureID(tex);

    // Update or Create Framebuffer
    if (!sc->current_framebuffer_ptr) {
        // First time, create framebuffer
        WebGPUFramebuffer *fb = framebuffer_allocator.alloc();
        memnew_placement(fb, WebGPUFramebuffer);
        fb->width = tex->width;
        fb->height = tex->height;
        fb->attachments.push_back(tex_id);
        
        sc->current_framebuffer_ptr = fb;
        sc->current_framebuffer_id = FramebufferID(fb);
    } else {
        // Update existing framebuffer
        sc->current_framebuffer_ptr->width = tex->width;
        sc->current_framebuffer_ptr->height = tex->height;
        if (sc->current_framebuffer_ptr->attachments.size() > 0) {
            sc->current_framebuffer_ptr->attachments.write[0] = tex_id;
        } else {
            sc->current_framebuffer_ptr->attachments.push_back(tex_id);
        }
    }

    return sc->current_framebuffer_id;
#else
	return FramebufferID();
#endif
}

RDD::RenderPassID RenderingDeviceDriverWebGPU::swap_chain_get_render_pass(SwapChainID p_swap_chain) {
#ifdef __EMSCRIPTEN__
    WebGPUSwapChain *sc = (WebGPUSwapChain *)p_swap_chain.id;
    if (!sc) return RenderPassID();

    if (sc->default_render_pass.id == 0) {
        // Create a default render pass compatible with swapchain
        WebGPURenderPass *pass = render_pass_allocator.alloc();
        memnew_placement(pass, WebGPURenderPass);
        
        Attachment att;
        att.format = DATA_FORMAT_B8G8R8A8_UNORM; // Match swapchain BGRA8
        att.samples = TEXTURE_SAMPLES_1;
        att.load_op = ATTACHMENT_LOAD_OP_CLEAR;
        att.store_op = ATTACHMENT_STORE_OP_STORE;
        // Default values for others
        pass->attachments.push_back(att);
        
        Subpass subpass;
        subpass.input_references = {};
        subpass.color_references = { AttachmentReference{0, ATTACHMENT_LAYOUT_COLOR_ATTACHMENT_OPTIMAL} };
        subpass.depth_stencil_reference = AttachmentReference{ATTACHMENT_UNUSED, ATTACHMENT_LAYOUT_UNDEFINED};
        subpass.resolve_references = {};
        pass->subpasses.push_back(subpass);

        sc->default_render_pass = RenderPassID(pass);
    }
    return sc->default_render_pass;
#else
	return RenderPassID();
#endif
}

RDD::DataFormat RenderingDeviceDriverWebGPU::swap_chain_get_format(SwapChainID p_swap_chain) {
#ifdef __EMSCRIPTEN__
    WebGPUSwapChain *sc = (WebGPUSwapChain *)p_swap_chain.id;
    // Map WGPU format back to RDD? Or store RDD format?
    // BGRA8Unorm -> DATA_FORMAT_B8G8R8A8_UNORM
    return DATA_FORMAT_B8G8R8A8_UNORM; 
#else
	return DATA_FORMAT_R8G8B8A8_UNORM;
#endif
}

void RenderingDeviceDriverWebGPU::swap_chain_free(SwapChainID p_swap_chain) {
#ifdef __EMSCRIPTEN__
    WebGPUSwapChain *sc = (WebGPUSwapChain *)p_swap_chain.id;
    if (sc) {
        if (sc->current_texture) {
            texture_free(TextureID(sc->current_texture));
        }
        if (sc->current_framebuffer_ptr) {
             sc->current_framebuffer_ptr->~WebGPUFramebuffer();
             framebuffer_allocator.free(sc->current_framebuffer_ptr);
        }
        if (sc->default_render_pass.id != 0) {
            render_pass_free(sc->default_render_pass);
        }
        
        sc->~WebGPUSwapChain();
        swap_chain_allocator.free(sc);
    }
#endif
}

RDD::FramebufferID RenderingDeviceDriverWebGPU::framebuffer_create(RenderPassID p_render_pass, VectorView<TextureID> p_attachments, uint32_t p_width, uint32_t p_height) {
#ifdef __EMSCRIPTEN__
	WebGPUFramebuffer *fb = framebuffer_allocator.alloc();
    memnew_placement(fb, WebGPUFramebuffer);
    fb->width = p_width;
    fb->height = p_height;
    
    for (uint32_t i = 0; i < p_attachments.size(); i++) {
        fb->attachments.push_back(p_attachments[i]);
    }

	return FramebufferID(fb);
#else
	return FramebufferID();
#endif
}

void RenderingDeviceDriverWebGPU::framebuffer_free(FramebufferID p_framebuffer) {
#ifdef __EMSCRIPTEN__
    WebGPUFramebuffer *fb = (WebGPUFramebuffer *)p_framebuffer.id;
    if (fb) {
        fb->~WebGPUFramebuffer();
        framebuffer_allocator.free(fb);
    }
#endif
}

RDD::ShaderID RenderingDeviceDriverWebGPU::shader_create_from_container(const Ref<RenderingShaderContainer> &p_shader_container, const Vector<ImmutableSampler> &p_immutable_samplers) {
#ifdef __EMSCRIPTEN__
    Ref<RenderingShaderContainerWebGPU> webgpu_container = p_shader_container;
    if (webgpu_container.is_null()) {
        return ShaderID();
    }

    WebGPUShader *shader = shader_allocator.alloc();
    memnew_placement(shader, WebGPUShader);

    // Create Shader Modules
    const Vector<RenderingShaderContainerWebGPU::StageWGSL> &stages = webgpu_container->get_wgsl_code();
    
    // Map: Set -> BindingIndex -> Entry
    struct EntryInfo {
        WGPUBindGroupLayoutEntry entry;
        uint32_t mirror_binding = 0xFFFFFFFF;
    };
    HashMap<uint32_t, HashMap<uint32_t, EntryInfo>> layout_entries;

    for (const auto &stage : stages) {
        // Create Module
        WGPUShaderModuleDescriptor desc = {};
        WGPUShaderModuleWGSLDescriptor wgsl_desc = {};
        wgsl_desc.chain.next = nullptr;
        wgsl_desc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
        CharString source = s.wgsl.utf8();
        wgsl_desc.code = source.get_data();

        WGPUShaderModuleDescriptor desc = {};
        desc.nextInChain = (const WGPUChainedStruct*)&wgsl_desc;
        desc.label = nullptr; // TODO: Pass shader name?
        
        WGPUShaderModule mod = wgpuDeviceCreateShaderModule(context_driver->get_wgpu_device(), &desc);
        if (mod) {
            shader->modules[s.stage] = mod;
        } else {
            ERR_PRINT("Failed to create WebGPU shader module for stage " + itos(s.stage));
            // Cleanup?
        }
    }
    
    return ShaderID(shader);
#else
	return ShaderID();
#endif
}

void RenderingDeviceDriverWebGPU::shader_free(ShaderID p_shader) {
#ifdef __EMSCRIPTEN__
    WebGPUShader *shader = (WebGPUShader *)p_shader.id;
    if (shader) {
        for (const KeyValue<RenderingDeviceDriver::ShaderStage, WGPUShaderModule> &E : shader->modules) {
            wgpuShaderModuleRelease(E.value);
        }
        shader->~WebGPUShader();
        shader_allocator.free(shader);
    }
#endif
}

void RenderingDeviceDriverWebGPU::shader_destroy_modules(ShaderID p_shader) {
    // Similar to destroy logic, but sometimes kept separated from free?
    // In Godot RDD, destroy_modules usually destroys the platform resource but keeps ID?
    // For now we map it to free logic or just ignore if free handles it.
    // Spec says: "Destroy the shader modules but keep the shader object valid...".
    // We'll leave it empty or implement if needed.
}

RDD::UniformSetID RenderingDeviceDriverWebGPU::uniform_set_create(VectorView<BoundUniform> p_uniforms, ShaderID p_shader, uint32_t p_set_index, int p_linear_pool_index) {
#ifdef __EMSCRIPTEN__
    WebGPUShader *shader = (WebGPUShader *)p_shader.id;
    if (!shader) return UniformSetID();
    
    // Validate Set Index
    if (p_set_index >= shader->bind_group_layouts.size() || !shader->bind_group_layouts[p_set_index]) {
        ERR_PRINT("Uniform set index out of bounds or layout missing");
        return UniformSetID();
    }
    
    // Create Bind Group Entries
    LocalVector<WGPUBindGroupEntry> entries;
    const WebGPUShader::SetInfo &set_info = shader->reflection_info[p_set_index];
    
    for(uint32_t i=0; i<p_uniforms.size(); i++) {
        const BoundUniform &u = p_uniforms[i];
        if (u.ids.size() == 0) continue; // Should not happen?
        
        // Find mapping
        // In Godot, `u.binding` is the binding index in the shader.
        // We look up our reflection info to see if we have valid mapping.
        if (!set_info.bindings.has(u.binding)) {
             // Maybe unused in shader but passed by Godot?
             continue;
        }
        
        const WebGPUShader::BindingInfo &b_info = set_info.bindings[u.binding];
        
        // Primary Binding
        WGPUBindGroupEntry entry = {};
        entry.binding = b_info.wgpu_binding;
        
        ID resource_id = u.ids[0]; // Assuming array size 1 for now? Godot RDD supports arrays.
        // If array, we need multiple entries? No, WebGPU doesn't support binding arrays easily in one entry unless it's an array of resources.
        // Actually, `WGPUBindGroupEntry` has `buffer`, `textureView`, `sampler` pointers.
        // It does not support array of these.
        // Godot usually unrolls arrays or we only support single for now.
        // TODO: Support Arrays.
        
        // Determine type from uniform or reflection?
        // We use the ID type.
        switch(u.type) {
            case UNIFORM_TYPE_UNIFORM_BUFFER:
            case UNIFORM_TYPE_STORAGE_BUFFER:
            case UNIFORM_TYPE_UNIFORM_BUFFER_DYNAMIC:
            case UNIFORM_TYPE_STORAGE_BUFFER_DYNAMIC: {
                WebGPUBuffer *buf = (WebGPUBuffer *)resource_id.id;
                if (buf) {
                    entry.buffer = buf->handle;
                    entry.offset = 0; // Entire buffer or default? Godot doesn't pass offset/size here?
                    // Godot RDD `BoundUniform` has no range info. It implies whole buffer?
                    // Actually, if it's dynamic, offset is handled at bind time.
                    // If static, it uses whole buffer.
                    entry.size = wgpuBufferGetSize(buf->handle); // Entire size
                }
                break;
            }
            case UNIFORM_TYPE_TEXTURE:
            case UNIFORM_TYPE_IMAGE:
            case UNIFORM_TYPE_INPUT_ATTACHMENT: { // Input attachment maps to texture
                WebGPUTexture *tex = (WebGPUTexture *)resource_id.id;
                if (tex) {
                    // We need a view. Main view?
                    // Godot RDD passes TextureID. We might need a specific view.
                    // Assuming main view for now.
                    entry.textureView = tex->main_view; 
                }
                break;
            }
            case UNIFORM_TYPE_SAMPLER: {
                WebGPUSampler *samp = (WebGPUSampler *)resource_id.id;
                if (samp) {
                    entry.sampler = samp->handle;
                }
                break;
            }
            case UNIFORM_TYPE_SAMPLER_WITH_TEXTURE: {
                // Combined. We need to split.
                 // Resource ID is Texture? Godot passes TextureID?
                 // Wait, `BoundUniform.ids`?
                 // "If type is SAMPLER_WITH_TEXTURE, ids[0] is sampler, ids[1] is texture? Or pairs?"
                 // Godot docs/internals:
                 // "For SAMPLER_WITH_TEXTURE, ids contains: SamplerID, TextureID, SamplerID..."
                 
                 WebGPUTexture *tex = nullptr;
                 WebGPUSampler *samp = nullptr;
                 
                 if (u.ids.size() >= 2) {
                     samp = (WebGPUSampler *)u.ids[0].id;
                     tex = (WebGPUTexture *)u.ids[1].id;
                 }
                 
                 // Bind Texture
                 entry.binding = b_info.wgpu_binding; // Assumption: Texture is primary
                 if (tex) entry.textureView = tex->main_view;
                 entries.push_back(entry);
                 
                 // Bind Sampler
                 if (b_info.mirror_binding != 0xFFFFFFFF) {
                     WGPUBindGroupEntry samp_entry = {};
                     samp_entry.binding = b_info.mirror_binding;
                     if (samp) samp_entry.sampler = samp->handle;
                     entries.push_back(samp_entry);
                 }
                 continue; // We pushed both
            }
            default: break;
        }
        
        if (u.type != UNIFORM_TYPE_SAMPLER_WITH_TEXTURE) {
             entries.push_back(entry);
        }
    }

    WGPUBindGroupDescriptor desc = {};
    desc.label = nullptr;
    desc.layout = shader->bind_group_layouts[p_set_index];
    desc.entryCount = entries.size();
    desc.entries = entries.ptr();
    
    WGPUBindGroup group = wgpuDeviceCreateBindGroup(context_driver->get_wgpu_device(), &desc);
    
    if (group) {
        WebGPUUniformSet *set = uniform_set_allocator.alloc();
        memnew_placement(set, WebGPUUniformSet);
        set->bind_group = group;
        return UniformSetID(set);
    }

    return UniformSetID();
#else
	return UniformSetID();
#endif
}

void RenderingDeviceDriverWebGPU::uniform_set_free(UniformSetID p_uniform_set) {
#ifdef __EMSCRIPTEN__
    WebGPUUniformSet *set = (WebGPUUniformSet *)p_uniform_set.id;
    if (set) {
        if (set->bind_group) wgpuBindGroupRelease(set->bind_group);
        set->~WebGPUUniformSet();
        uniform_set_allocator.free(set);
    }
#endif
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
#ifdef __EMSCRIPTEN__
	WebGPURenderPass *pass = render_pass_allocator.alloc();
    memnew_placement(pass, WebGPURenderPass);
    
    for(uint32_t i = 0; i < p_attachments.size(); i++) {
        pass->attachments.push_back(p_attachments[i]);
    }
    for(uint32_t i = 0; i < p_subpasses.size(); i++) {
        pass->subpasses.push_back(p_subpasses[i]);
    }
    
	return RenderPassID(pass);
#else
	return RenderPassID();
#endif
}

void RenderingDeviceDriverWebGPU::render_pass_free(RenderPassID p_render_pass) {
#ifdef __EMSCRIPTEN__
	WebGPURenderPass *pass = (WebGPURenderPass *)p_render_pass.id;
    if (pass) {
        pass->~WebGPURenderPass();
        render_pass_allocator.free(pass);
    }
#endif
}

#ifdef __EMSCRIPTEN__
static WGPULoadOp _get_wgpu_load_op(RenderingDeviceDriver::AttachmentLoadOp p_op) {
    switch(p_op) {
        case RenderingDeviceDriver::ATTACHMENT_LOAD_OP_LOAD: return WGPULoadOp_Load;
        case RenderingDeviceDriver::ATTACHMENT_LOAD_OP_CLEAR: return WGPULoadOp_Clear;
        case RenderingDeviceDriver::ATTACHMENT_LOAD_OP_DONT_CARE: return WGPULoadOp_Load; // Or undefined? Default to Load for safety?
        default: return WGPULoadOp_Load;
    }
}

static WGPUStoreOp _get_wgpu_store_op(RenderingDeviceDriver::AttachmentStoreOp p_op) {
     switch(p_op) {
        case RenderingDeviceDriver::ATTACHMENT_STORE_OP_STORE: return WGPUStoreOp_Store;
        case RenderingDeviceDriver::ATTACHMENT_STORE_OP_DONT_CARE: return WGPUStoreOp_Discard;
        default: return WGPUStoreOp_Store;
    }
}
#endif

void RenderingDeviceDriverWebGPU::command_begin_render_pass(CommandBufferID p_cmd_buffer, RenderPassID p_render_pass, FramebufferID p_framebuffer, CommandBufferType p_cmd_buffer_type, const Rect2i &p_rect, VectorView<RenderPassClearValue> p_clear_values) {
#ifdef __EMSCRIPTEN__
    WebGPUCommandBuffer *cmd = (WebGPUCommandBuffer *)p_cmd_buffer.id;
    WebGPUFramebuffer *fb = (WebGPUFramebuffer *)p_framebuffer.id;
    WebGPURenderPass *pass = (WebGPURenderPass *)p_render_pass.id;

    if (!cmd || !cmd->encoder || !fb || !pass) return;

    WGPURenderPassDescriptor desc = {};
    LocalVector<WGPURenderPassColorAttachment> color_attachments;
    WGPURenderPassDepthStencilAttachment depth_attachment_desc = {};
    bool has_depth = false;

    // Iterate framebuffer attachments (matched with RenderPass attachment descriptions)
    for (uint32_t i = 0; i < fb->attachments.size(); i++) {
        // Find if this attachment is Depth or Color.
        // Usually we check format.
        WebGPUTexture *tex = (WebGPUTexture *)fb->attachments[i].id;
        if (!tex) continue;

        // Check if depth
        bool is_depth = (tex->format == WGPUTextureFormat_Depth32Float || 
                         tex->format == WGPUTextureFormat_Depth16Unorm || 
                         tex->format == WGPUTextureFormat_Depth24Plus || 
                         tex->format == WGPUTextureFormat_Depth24PlusStencil8);

        Attachment &att_desc = pass->attachments[i];

        if (is_depth) {
             has_depth = true;
             depth_attachment_desc.view = tex->view;
             depth_attachment_desc.depthLoadOp = _get_wgpu_load_op(att_desc.load_op);
             depth_attachment_desc.depthStoreOp = _get_wgpu_store_op(att_desc.store_op);
             depth_attachment_desc.depthClearValue = p_clear_values[i].depth_stencil.depth;
             
             // TODO: Stencil logic
             depth_attachment_desc.stencilLoadOp = WGPULoadOp_Load;
             depth_attachment_desc.stencilStoreOp = WGPUStoreOp_Store;

             desc.depthStencilAttachment = &depth_attachment_desc;
        } else {
             WGPURenderPassColorAttachment color_att = {};
             color_att.view = tex->view;
             color_att.resolveTarget = nullptr; // TODO
             color_att.loadOp = _get_wgpu_load_op(att_desc.load_op);
             color_att.storeOp = _get_wgpu_store_op(att_desc.store_op);
             
             Color c = p_clear_values[i].color;
             color_att.clearValue = { c.r, c.g, c.b, c.a };
             
             color_attachments.push_back(color_att);
        }
    }

    desc.colorAttachmentCount = color_attachments.size();
    desc.colorAttachments = color_attachments.ptr();
    
    cmd->render_encoder = wgpuCommandEncoderBeginRenderPass(cmd->encoder, &desc);
#endif
}

void RenderingDeviceDriverWebGPU::command_end_render_pass(CommandBufferID p_cmd_buffer) {
#ifdef __EMSCRIPTEN__
    WebGPUCommandBuffer *cmd = (WebGPUCommandBuffer *)p_cmd_buffer.id;
    if (cmd && cmd->render_encoder) {
        wgpuRenderPassEncoderEnd(cmd->render_encoder);
        wgpuRenderPassEncoderRelease(cmd->render_encoder);
        cmd->render_encoder = nullptr;
    }
#endif
}

void RenderingDeviceDriverWebGPU::command_next_render_subpass(CommandBufferID p_cmd_buffer, CommandBufferType p_cmd_buffer_type) {
}

void RenderingDeviceDriverWebGPU::command_render_set_viewport(CommandBufferID p_cmd_buffer, VectorView<Rect2i> p_viewports) {
}

void RenderingDeviceDriverWebGPU::command_render_set_scissor(CommandBufferID p_cmd_buffer, VectorView<Rect2i> p_scissors) {
    // wgpuRenderPassEncoderSetScissorRect(encoder, x, y, w, h)
}

void RenderingDeviceDriverWebGPU::command_render_clear_attachments(CommandBufferID p_cmd_buffer, VectorView<AttachmentClear> p_attachment_clears, VectorView<Rect2i> p_rects) {
}

void RenderingDeviceDriverWebGPU::command_buffer_free(CommandBufferID p_cmd_buffer) {
#ifdef __EMSCRIPTEN__
    WebGPUCommandBuffer *cmd = (WebGPUCommandBuffer *)p_cmd_buffer.id;
    if (cmd) {
        if (cmd->buffer) wgpuCommandBufferRelease(cmd->buffer);
        if (cmd->encoder) wgpuCommandEncoderRelease(cmd->encoder);
        if (cmd->render_encoder) wgpuRenderPassEncoderRelease(cmd->render_encoder);
        if (cmd->compute_encoder) wgpuComputePassEncoderRelease(cmd->compute_encoder);
        
        cmd->~WebGPUCommandBuffer();
        command_buffer_allocator.free(cmd);
    }
#endif
}

void RenderingDeviceDriverWebGPU::command_bind_render_pipeline(CommandBufferID p_cmd_buffer, PipelineID p_pipeline) {
#ifdef __EMSCRIPTEN__
    WebGPUCommandBuffer *cmd = (WebGPUCommandBuffer *)p_cmd_buffer.id;
    WebGPUPipeline *pipeline = (WebGPUPipeline *)p_pipeline.id;
    
    if (cmd && cmd->render_encoder && pipeline && pipeline->render_pipeline) {
        wgpuRenderPassEncoderSetPipeline(cmd->render_encoder, pipeline->render_pipeline);
    }
#endif
}

void RenderingDeviceDriverWebGPU::command_bind_render_uniform_sets(CommandBufferID p_cmd_buffer, VectorView<UniformSetID> p_uniform_sets, ShaderID p_shader, uint32_t p_first_set_index, uint32_t p_set_count, uint32_t p_dynamic_offsets) {
#ifdef __EMSCRIPTEN__
    WebGPUCommandBuffer *cmd = (WebGPUCommandBuffer *)p_cmd_buffer.id;
    if (cmd && cmd->render_encoder) {
        for (uint32_t i=0; i<p_set_count; i++) {
             if (i < p_uniform_sets.size()) {
                 WebGPUUniformSet *set = (WebGPUUniformSet *)p_uniform_sets[i].id;
                 if (set && set->bind_group) {
                     // Dynamic offsets logic handling needed if we decide to support it properly.
                     // The generic API passes a count? 
                     // Wait, `p_dynamic_offsets` is `uint32_t`. 
                     // In Vulkan backend it seems to use `uniform_sets_get_dynamic_offsets`?
                     wgpuRenderPassEncoderSetBindGroup(cmd->render_encoder, p_first_set_index + i, set->bind_group, 0, nullptr);
                 }
             }
        }
    }
#endif
}

void RenderingDeviceDriverWebGPU::command_render_draw(CommandBufferID p_cmd_buffer, uint32_t p_vertex_count, uint32_t p_instance_count, uint32_t p_first_vertex, uint32_t p_first_instance) {
#ifdef __EMSCRIPTEN__
    WebGPUCommandBuffer *cmd = (WebGPUCommandBuffer *)p_cmd_buffer.id;
    if (cmd && cmd->render_encoder) {
        wgpuRenderPassEncoderDraw(cmd->render_encoder, p_vertex_count, p_instance_count, p_first_vertex, p_first_instance);
    }
#endif
}

void RenderingDeviceDriverWebGPU::command_render_draw_indexed(CommandBufferID p_cmd_buffer, uint32_t p_index_count, uint32_t p_instance_count, uint32_t p_first_index, int32_t p_vertex_offset, uint32_t p_first_instance) {
#ifdef __EMSCRIPTEN__
    WebGPUCommandBuffer *cmd = (WebGPUCommandBuffer *)p_cmd_buffer.id;
    if (cmd && cmd->render_encoder) {
        wgpuRenderPassEncoderDrawIndexed(cmd->render_encoder, p_index_count, p_instance_count, p_first_index, p_vertex_offset, p_first_instance);
    }
#endif
}

void RenderingDeviceDriverWebGPU::command_render_draw_indexed_indirect(CommandBufferID p_cmd_buffer, BufferID p_indirect_buffer, uint64_t p_offset, uint32_t p_draw_count, uint32_t p_stride) {
#ifdef __EMSCRIPTEN__
    WebGPUCommandBuffer *cmd = (WebGPUCommandBuffer *)p_cmd_buffer.id;
    WebGPUBuffer *buf = (WebGPUBuffer *)p_indirect_buffer.id;
    if (cmd && cmd->render_encoder && buf) {
        // WebGPU usually supports only 1 draw per call unless MultiDrawIndirect feature?
        // Godot usually passes draw_count=1 for standard.
        // If draw_count > 1, loop? But stride must match?
        // wgpuRenderPassEncoderDrawIndexedIndirect(encoder, buffer, offset)
        wgpuRenderPassEncoderDrawIndexedIndirect(cmd->render_encoder, buf->handle, p_offset);
    }
#endif
}

void RenderingDeviceDriverWebGPU::command_render_draw_indexed_indirect_count(CommandBufferID p_cmd_buffer, BufferID p_indirect_buffer, uint64_t p_offset, BufferID p_count_buffer, uint64_t p_count_buffer_offset, uint32_t p_max_draw_count, uint32_t p_stride) {
}

void RenderingDeviceDriverWebGPU::command_render_draw_indirect(CommandBufferID p_cmd_buffer, BufferID p_indirect_buffer, uint64_t p_offset, uint32_t p_draw_count, uint32_t p_stride) {
#ifdef __EMSCRIPTEN__
    WebGPUCommandBuffer *cmd = (WebGPUCommandBuffer *)p_cmd_buffer.id;
    WebGPUBuffer *buf = (WebGPUBuffer *)p_indirect_buffer.id;
    if (cmd && cmd->render_encoder && buf) {
        wgpuRenderPassEncoderDrawIndirect(cmd->render_encoder, buf->handle, p_offset);
    }
#endif
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

RDD::PipelineID RenderingDeviceDriverWebGPU::render_pipeline_create(
		ShaderID p_shader,
		VertexFormatID p_vertex_format,
		RenderPrimitive p_render_primitive,
		PipelineRasterizationState p_rasterization_state,
		PipelineMultisampleState p_multisample_state,
		PipelineDepthStencilState p_depth_stencil_state,
		PipelineColorBlendState p_blend_state,
		VectorView<int32_t> p_color_attachments,
		BitField<PipelineDynamicStateFlags> p_dynamic_state,
		RenderPassID p_render_pass,
		uint32_t p_render_subpass,
		VectorView<PipelineSpecializationConstant> p_specialization_constants) {

#ifdef __EMSCRIPTEN__
    WebGPUShader *shader = (WebGPUShader *)p_shader.id;
    if (!shader) return PipelineID();

    WebGPUPipeline *pipeline = pipeline_allocator.alloc();
    memnew_placement(pipeline, WebGPUPipeline);

    WGPURenderPipelineDescriptor desc = {};
    desc.label = nullptr; 

    // 1. Vertex State
    WGPUVertexState vertex = {};
    if (shader->modules.has(SHADER_STAGE_VERTEX)) {
        vertex.module = shader->modules[SHADER_STAGE_VERTEX];
        vertex.entryPoint = "main";
    } else {
        // Error or fallback? Vertex stage is mandatory.
        ERR_PRINT("Vertex shader module missing");
        return PipelineID();
    }
    
    WebGPUVertexFormat *vf = (WebGPUVertexFormat *)p_vertex_format.id;
    if (vf) {
        vertex.bufferCount = vf->buffers.size();
        vertex.buffers = vf->buffers.ptr();
    }
    desc.vertex = vertex;

    // 2. Fragment State
    WGPUFragmentState fragment = {};
    bool has_fragment = shader->modules.has(SHADER_STAGE_FRAGMENT);
    if (has_fragment) {
        fragment.module = shader->modules[SHADER_STAGE_FRAGMENT];
        fragment.entryPoint = "main";
        
        LocalVector<WGPUColorTargetState> targets;
        WebGPURenderPass *pass = (WebGPURenderPass *)p_render_pass.id;

        // Iterate requested color attachments
        for (uint32_t i = 0; i < p_color_attachments.size(); i++) {
            int32_t attachment_idx = p_color_attachments[i];
            if (attachment_idx == RenderingDeviceCommons::ATTACHMENT_UNUSED) {
                 // TODO: sparse attachment support?
                 // WebGPU targets array is dense. If we have attachment 0 and 2, but not 1?
                 // Godot usually packs them?
                 // If p_color_attachments contains -1, what does it mean?
                 // It means "no draw to this output".
                 // WebGPU allows null targets/writeMask=0?
                 // If the shader outputs to location 1, we need binding 1 in targets?
                 // Actually `targets` array index corresponds to shader `@location(index)`.
                 // `p_color_attachments[i]` maps shader output `i` to RenderPass attachment `attachment_idx`.
                 
                 // NOTE: `p_color_attachments` size is the number of colored outputs from shader? 
                 // Or typically `targets` count matches.
                 
                WGPUColorTargetState target = {};
                // If unused, we might need a dummy format or WGPUTextureFormat_Undefined implies no write?
                // Spec: "If a target is nullptr..."
                // We'll handle unused by skipping or null?
                // For now assuming dense or implementing minimal logic.
                 target.format = WGPUTextureFormat_Undefined; 
                 target.writeMask = 0;
                 targets.push_back(target);
                 continue;
            }
            
            if (pass && attachment_idx >= 0 && (uint32_t)attachment_idx < pass->attachments.size()) {
                WGPUColorTargetState target = {};
                target.format = _get_wgpu_texture_format(pass->attachments[attachment_idx].format);
                
                // Blend State (per target)
                // Godot p_blend_state.attachments[i]
                if (i < p_blend_state.attachments.size()) {
                    const PipelineColorBlendState::Attachment &blend_att = p_blend_state.attachments[i];
                    if (blend_att.enable_blend) {
                         // We need a stable pointer for blend state.
                         // Use LocalVector or memory within this scope?
                         // WGPUColorTargetState has `blend` pointer.
                         // We can allocate WGPUBlendState on heap or stack if vector doesn't resize?
                         // Actually `targets` vector resizing invalidates internal pointers if we stored them?
                         // No, WGPUColorTargetState stores `const WGPUBlendState * blend`.
                         // We need `WGPUBlendState` to be alive when we call `createRenderPipeline`.
                         // So we need a parallel vector of WGPUBlendState.
                    }
                     target.writeMask = _get_wgpu_color_write_mask(blend_att.write_r, blend_att.write_g, blend_att.write_b, blend_att.write_a);
                } else {
                    target.writeMask = WGPUColorWriteMask_All;
                }
                targets.push_back(target);
            }
        }
        
        // Handle Blending Pointers
        // We need a stable storage for BlendState structs to point to.
        LocalVector<WGPUBlendState> blend_states;
        blend_states.resize(targets.size());
        
        for (int i=0; i<targets.size(); i++) {
             int32_t attachment_idx = p_color_attachments[i];
             if (attachment_idx != RenderingDeviceCommons::ATTACHMENT_UNUSED && i < p_blend_state.attachments.size()) {
                 const PipelineColorBlendState::Attachment &blend_att = p_blend_state.attachments[i];
                 if (blend_att.enable_blend) {
                     WGPUBlendState &bs = blend_states[i];
                     bs.color.operation = _get_wgpu_blend_operation(blend_att.color_blend_op);
                     bs.color.srcFactor = _get_wgpu_blend_factor(blend_att.src_color_blend_factor);
                     bs.color.dstFactor = _get_wgpu_blend_factor(blend_att.dst_color_blend_factor);
                     bs.alpha.operation = _get_wgpu_blend_operation(blend_att.alpha_blend_op);
                     bs.alpha.srcFactor = _get_wgpu_blend_factor(blend_att.src_alpha_blend_factor);
                     bs.alpha.dstFactor = _get_wgpu_blend_factor(blend_att.dst_alpha_blend_factor);
                     
                     targets[i].blend = &bs;
                 }
             }
        }
        
        fragment.targetCount = targets.size();
        fragment.targets = targets.ptr();
        desc.fragment = &fragment;
    }

    // 3. Primitive
    WGPUPrimitiveState primitive = {};
    primitive.topology = _get_wgpu_primitive_topology(p_render_primitive);
    primitive.frontFace = _get_wgpu_front_face(p_rasterization_state.front_face);
    primitive.cullMode = _get_wgpu_cull_mode(p_rasterization_state.cull_mode);
    // Strip Index Format?
    if (p_render_primitive == RENDER_PRIMITIVE_TRIANGLE_STRIPS || p_render_primitive == RENDER_PRIMITIVE_LINE_STRIPS) {
         // Godot pipeline creation doesn't specify index format (index buffer bind does).
         // WebGPU requires stripIndexFormat if topology is strip?
         // "If the topology is a strip topology... stripIndexFormat must be specified."
         // Godot sets index buffer format at bind time.
         // This is a mismatch. WebGPU pipeline needs to know if it's uint16 or uint32 for restart value (0xFFFF or 0xFFFFFFFF).
         // Godot `p_render_primitive` implies strip, but not index format.
         // Warning: We might need assume generic or restart is disabled?
         // Or default to Undefined and hope implementation allows?
         // Spec: "Required if topology is...".
         // We might need to guess Use 32-bit? Or allow pipeline variation?
         primitive.stripIndexFormat = WGPUIndexFormat_Undefined; // TODO: Check if this errors.
    }
    desc.primitive = primitive;

    // 4. Depth Stencil
    WGPUDepthStencilState depth_stencil = {};
    WebGPURenderPass *pass = (WebGPURenderPass *)p_render_pass.id;
    int32_t depth_att_idx = -1;
    // Find depth attachment index in subpass
    if (pass && p_render_subpass < pass->subpasses.size()) {
         uint32_t ref = pass->subpasses[p_render_subpass].depth_stencil_reference.attachment;
         if (ref != AttachmentReference::UNUSED) {
             depth_att_idx = (int32_t)ref;
         }
    }
    
    if (depth_att_idx >= 0 && depth_att_idx < pass->attachments.size()) {
        depth_stencil.format = _get_wgpu_texture_format(pass->attachments[depth_att_idx].format);
        depth_stencil.depthWriteEnabled = p_depth_stencil_state.enable_depth_write;
        depth_stencil.depthCompare = p_depth_stencil_state.enable_depth_test ? 
                                     _get_wgpu_compare_function(p_depth_stencil_state.depth_compare_operator) : 
                                     WGPUCompareFunction_Always;
        
        // Stencil
        WGPUStencilFaceState stencil_front = {};
        stencil_front.compare = _get_wgpu_compare_function(p_depth_stencil_state.front_op.compare);
        stencil_front.failOp = _get_wgpu_stencil_operation(p_depth_stencil_state.front_op.fail);
        stencil_front.depthFailOp = _get_wgpu_stencil_operation(p_depth_stencil_state.front_op.depth_fail);
        stencil_front.passOp = _get_wgpu_stencil_operation(p_depth_stencil_state.front_op.pass);
        
        WGPUStencilFaceState stencil_back = {};
        stencil_back.compare = _get_wgpu_compare_function(p_depth_stencil_state.back_op.compare);
        stencil_back.failOp = _get_wgpu_stencil_operation(p_depth_stencil_state.back_op.fail);
        stencil_back.depthFailOp = _get_wgpu_stencil_operation(p_depth_stencil_state.back_op.depth_fail);
        stencil_back.passOp = _get_wgpu_stencil_operation(p_depth_stencil_state.back_op.pass);
        
        depth_stencil.stencilFront = stencil_front;
        depth_stencil.stencilBack = stencil_back;
        depth_stencil.stencilReadMask = 0xFFFFFFFF; // TODO: get from state
        depth_stencil.stencilWriteMask = 0xFFFFFFFF; // TODO: get from state
        
        // Godot passes range? No, dynamic state.
        
        desc.depthStencil = &depth_stencil;
    }

    // 5. Multisample
    WGPUMultisampleState multisample = {};
    multisample.count = _get_wgpu_sample_count(p_multisample_state.sample_count); // Helper needed
    multisample.mask = 0xFFFFFFFF; // Godot sample mask?
    multisample.alphaToCoverageEnabled = p_multisample_state.enable_alpha_to_coverage;
    desc.multisample = multisample;

    // 6. Layout
    // Use "auto" layout by passing nullptr (if supported) or create explicit.
    // Emscripten/Dawn supports implicit layout if not provided?
    // "If layout is null, it's auto".
    desc.layout = nullptr; 

    pipeline->render_pipeline = wgpuDeviceCreateRenderPipeline(context_driver->get_wgpu_device(), &desc);
    
    if (!pipeline->render_pipeline) {
        ERR_PRINT("Failed to create WebGPU Render Pipeline");
        pipeline->~WebGPUPipeline();
        pipeline_allocator.free(pipeline);
        return PipelineID();
    }

    return PipelineID(pipeline);
#else
	return PipelineID();
#endif
}

void RenderingDeviceDriverWebGPU::pipeline_free(PipelineID p_pipeline) {
#ifdef __EMSCRIPTEN__
    WebGPUPipeline *pipeline = (WebGPUPipeline *)p_pipeline.id;
    if (pipeline) {
        if (pipeline->render_pipeline) wgpuRenderPipelineRelease(pipeline->render_pipeline);
        if (pipeline->compute_pipeline) wgpuComputePipelineRelease(pipeline->compute_pipeline);
        pipeline->~WebGPUPipeline();
        pipeline_allocator.free(pipeline);
    }
#endif
}

void RenderingDeviceDriverWebGPU::command_bind_compute_pipeline(CommandBufferID p_cmd_buffer, PipelineID p_pipeline) {
#ifdef __EMSCRIPTEN__
    WebGPUCommandBuffer *cmd = (WebGPUCommandBuffer *)p_cmd_buffer.id;
    WebGPUPipeline *pipeline = (WebGPUPipeline *)p_pipeline.id;
    
    if (cmd && cmd->compute_encoder && pipeline && pipeline->compute_pipeline) {
        wgpuComputePassEncoderSetPipeline(cmd->compute_encoder, pipeline->compute_pipeline);
    }
#endif
}

void RenderingDeviceDriverWebGPU::command_bind_compute_uniform_sets(CommandBufferID p_cmd_buffer, VectorView<UniformSetID> p_uniform_sets, ShaderID p_shader, uint32_t p_first_set_index, uint32_t p_set_count, uint32_t p_dynamic_offsets) {
     // TODO: Implement uniform sets first
}

void RenderingDeviceDriverWebGPU::command_compute_dispatch(CommandBufferID p_cmd_buffer, uint32_t p_x_groups, uint32_t p_y_groups, uint32_t p_z_groups) {
#ifdef __EMSCRIPTEN__
    WebGPUCommandBuffer *cmd = (WebGPUCommandBuffer *)p_cmd_buffer.id;
    if (cmd && cmd->compute_encoder) {
        wgpuComputePassEncoderDispatchWorkgroups(cmd->compute_encoder, p_x_groups, p_y_groups, p_z_groups);
    }
#endif
}

void RenderingDeviceDriverWebGPU::command_compute_dispatch_indirect(CommandBufferID p_cmd_buffer, BufferID p_indirect_buffer, uint64_t p_offset) {
#ifdef __EMSCRIPTEN__
    WebGPUCommandBuffer *cmd = (WebGPUCommandBuffer *)p_cmd_buffer.id;
    WebGPUBuffer *buf = (WebGPUBuffer *)p_indirect_buffer.id;
    if (cmd && cmd->compute_encoder && buf) {
        wgpuComputePassEncoderDispatchWorkgroupsIndirect(cmd->compute_encoder, buf->handle, p_offset);
    }
#endif
}

RDD::PipelineID RenderingDeviceDriverWebGPU::compute_pipeline_create(ShaderID p_shader, VectorView<PipelineSpecializationConstant> p_specialization_constants) {
#ifdef __EMSCRIPTEN__
    WebGPUShader *shader = (WebGPUShader *)p_shader.id;
    if (!shader) return PipelineID();

    WebGPUPipeline *pipeline = pipeline_allocator.alloc();
    memnew_placement(pipeline, WebGPUPipeline);

    WGPUComputePipelineDescriptor desc = {};
    desc.label = nullptr; 
    
    // Layout
    desc.layout = nullptr; // Auto

    // Compute Stage
    WGPUComputeState compute = {};
    if (shader->modules.has(SHADER_STAGE_COMPUTE)) {
        compute.module = shader->modules[SHADER_STAGE_COMPUTE];
        compute.entryPoint = "main";
        // TODO: constants (p_specialization_constants map to WGSL overridable constants?)
        // compute.constantCount = ...
        // compute.constants = ...
    } else {
        ERR_PRINT("Compute shader module missing");
        pipeline->~WebGPUPipeline();
        pipeline_allocator.free(pipeline);
        return PipelineID();
    }
    desc.compute = compute;

    pipeline->compute_pipeline = wgpuDeviceCreateComputePipeline(context_driver->get_wgpu_device(), &desc);
    
    if (!pipeline->compute_pipeline) {
        ERR_PRINT("Failed to create WebGPU Compute Pipeline");
        pipeline->~WebGPUPipeline();
        pipeline_allocator.free(pipeline);
        return PipelineID();
    }

    return PipelineID(pipeline);
#else
	return PipelineID();
#endif
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
    // Return safe defaults for now
    switch(p_limit) {
        case LIMIT_MAX_TEXTURE_SIZE: return 8192;
        case LIMIT_MAX_TEXTURE_ARRAY_LAYERS: return 256;
        case LIMIT_MAX_FRAMEBUFFER_WIDTH: return 8192;
        case LIMIT_MAX_FRAMEBUFFER_HEIGHT: return 8192;
        case LIMIT_MAX_UNIFORM_BUFFER_SIZE: return 65536;
        default: return 1;
    }
}

uint64_t RenderingDeviceDriverWebGPU::api_trait_get(ApiTrait p_trait) {
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
	static RenderingShaderContainerFormat format;
	return format;
}
