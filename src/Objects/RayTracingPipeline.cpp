#include "Golmon.hpp"
#include "Objects/RayTracingPipeline.hpp"
#include "Objects/Sync.hpp"
#include "Objects/Commands.hpp"

using namespace vulkan;

PFN_vkGetBufferDeviceAddressKHR RayTracingPipeline::vkGetBufferDeviceAddressKHR = nullptr;
PFN_vkCreateRayTracingPipelinesKHR RayTracingPipeline::vkCreateRayTracingPipelinesKHR = nullptr;
PFN_vkGetAccelerationStructureBuildSizesKHR RayTracingPipeline::vkGetAccelerationStructureBuildSizesKHR = nullptr;
PFN_vkCreateAccelerationStructureKHR RayTracingPipeline::vkCreateAccelerationStructureKHR = nullptr;
PFN_vkDestroyAccelerationStructureKHR RayTracingPipeline::pvkDestroyAccelerationStructureKHR = nullptr;
PFN_vkGetAccelerationStructureDeviceAddressKHR RayTracingPipeline::vkGetAccelerationStructureDeviceAddressKHR = nullptr;
PFN_vkCmdBuildAccelerationStructuresKHR RayTracingPipeline::vkCmdBuildAccelerationStructuresKHR = nullptr;
PFN_vkGetRayTracingShaderGroupHandlesKHR RayTracingPipeline::vkGetRayTracingShaderGroupHandlesKHR = nullptr;
PFN_vkCmdTraceRaysKHR RayTracingPipeline::vkCmdTraceRaysKHR = nullptr;

void RayTracingPipeline::load_raytracing_functions(void)
{
	vkGetBufferDeviceAddressKHR = GET_INSTANCE_PROC(Context::instance.ptr, PFN_vkGetBufferDeviceAddressKHR);
	vkCreateRayTracingPipelinesKHR = GET_INSTANCE_PROC(Context::instance.ptr, PFN_vkCreateRayTracingPipelinesKHR);
	vkGetAccelerationStructureBuildSizesKHR = GET_INSTANCE_PROC(Context::instance.ptr, PFN_vkGetAccelerationStructureBuildSizesKHR);
	vkCreateAccelerationStructureKHR = GET_INSTANCE_PROC(Context::instance.ptr, PFN_vkCreateAccelerationStructureKHR);
	pvkDestroyAccelerationStructureKHR = GET_INSTANCE_PROC(Context::instance.ptr, PFN_vkDestroyAccelerationStructureKHR);
	vkGetAccelerationStructureDeviceAddressKHR = GET_INSTANCE_PROC(Context::instance.ptr, PFN_vkGetAccelerationStructureDeviceAddressKHR);
	vkCmdBuildAccelerationStructuresKHR = GET_INSTANCE_PROC(Context::instance.ptr, PFN_vkCmdBuildAccelerationStructuresKHR);
	vkGetRayTracingShaderGroupHandlesKHR = GET_INSTANCE_PROC(Context::instance.ptr, PFN_vkGetRayTracingShaderGroupHandlesKHR);
	vkCmdTraceRaysKHR = GET_INSTANCE_PROC(Context::instance.ptr, PFN_vkCmdTraceRaysKHR);

	if (vkGetBufferDeviceAddressKHR && vkCreateRayTracingPipelinesKHR && vkGetAccelerationStructureBuildSizesKHR && 
		vkCreateAccelerationStructureKHR && pvkDestroyAccelerationStructureKHR && vkGetAccelerationStructureDeviceAddressKHR
		&& vkCmdBuildAccelerationStructuresKHR && vkGetRayTracingShaderGroupHandlesKHR && vkCmdTraceRaysKHR) return;
	throw std::runtime_error("failed to get raytracing functions");
}

AS::AS(void)
{

}

AS::~AS(void)
{

}

void AS::init_as(VkAccelerationStructureTypeKHR type)
{
	as_buffer.init(
		build_sizes_info.accelerationStructureSize,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
	);

	VkAccelerationStructureCreateInfoKHR create_info{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
	create_info.buffer = as_buffer.ptr;
	create_info.size = as_buffer.original_size;
	create_info.type = type;

	if (RayTracingPipeline::vkCreateAccelerationStructureKHR(Context::device.ptr, &create_info, nullptr, &ptr) != VK_SUCCESS)
		throw std::runtime_error("failed to create AS");

	//std::cout << "AS constructed with size " << as_buffer.size << " (original size: " << as_buffer.original_size << ")" << std::endl;

	VkAccelerationStructureDeviceAddressInfoKHR address_info{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
	address_info.accelerationStructure = ptr;
	address = RayTracingPipeline::vkGetAccelerationStructureDeviceAddressKHR(Context::device.ptr, &address_info);

	scratch_buffer.init(
		build_sizes_info.buildScratchSize,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
	);

	build_geometry.dstAccelerationStructure = ptr;
	build_geometry.scratchData.deviceAddress = scratch_buffer.address;
	build_geometry.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;

	//std::cout << "scratch buffer created with size: " << scratch_buffer.size << "(original size: " << scratch_buffer.original_size << ")" << std::endl;
}

void AS::build(vulkan::CommandBuffer& cb)
{
	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> build_range_infos_ptr;
	vulkan::Fence f;
	f.init(false);

	for (auto& i : build_range_infos)
		build_range_infos_ptr.push_back(&i);

	cb.begin();

	RayTracingPipeline::vkCmdBuildAccelerationStructuresKHR(
		cb.ptr, 1,
		&build_geometry,
		build_range_infos_ptr.data()
	);

	cb.end();

	cb.submit_p(nullptr, nullptr, &f);
	f.wait();
}

BLAS::BLAS(void)
{

}

BLAS::~BLAS(void)
{

}

void BLAS::init(std::vector<vulkan::Buffer*> input_buffers)
{
	geometries.resize(input_buffers.size());
	build_range_infos.resize(input_buffers.size());
	blas_max_primitive.resize(input_buffers.size());

	//std::cout << "building BLAS" << std::endl;
	//std::cout << "input buffers: " << input_buffers.size() << std::endl;

	for (int i = 0; i < input_buffers.size(); i++) {
		uint32_t nb_triangles = static_cast<uint32_t>(input_buffers[i]->original_size / (sizeof(Vertex)));
		VkAccelerationStructureGeometryTrianglesDataKHR  triangles{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR };
		triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		triangles.vertexData.deviceAddress = input_buffers[i]->address;
		triangles.vertexStride = sizeof(Vertex);
		triangles.indexType = VK_INDEX_TYPE_NONE_KHR;
		triangles.maxVertex = nb_triangles;

		//std::cout << "input buffer [" << i << "]:" << std::endl << "size: " << input_buffers[i]->size << std::endl << "original size:" << input_buffers[i]->original_size;
		//std::cout << std::endl << "maxVertex:" << triangles.maxVertex << std::endl << "nb_triangles: " << nb_triangles << std::endl;

		geometries[i].sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geometries[i].geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR; 
		geometries[i].geometry.triangles = triangles; 
		geometries[i].flags = VK_GEOMETRY_OPAQUE_BIT_KHR; 

		blas_max_primitive[i] = nb_triangles;
		build_range_infos[i].primitiveCount = nb_triangles / 3;
	}

	build_geometry.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	build_geometry.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	build_geometry.geometryCount = static_cast<uint32_t>(geometries.size());
	build_geometry.pGeometries = geometries.data();

	RayTracingPipeline::vkGetAccelerationStructureBuildSizesKHR(
		Context::device.ptr, 
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, 
		&build_geometry, 
		blas_max_primitive.data(), 
		&build_sizes_info
	);

	init_as(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR);

}

TLAS::TLAS(void)
{

}

TLAS::~TLAS(void)
{

}

void TLAS::init(std::vector<BLAS> &blases)
{
	build_range_infos.push_back({ .primitiveCount = static_cast<uint32_t>(blases.size()) });
	instances.resize(blases.size());
	std::vector<uint32_t> max_primitives({static_cast<uint32_t>(blases.size())});

	instances_buffer.init(
		sizeof(VkAccelerationStructureInstanceKHR) * blases.size(),
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
	);
	instances_buffer.map();

	for (int i = 0; i < blases.size(); i++) {

		instances[i].transform = {
			.matrix = {
				{1.0, 0.0, 0.0, 0.0},
				{0.0, 1.0, 0.0, 0.0},
				{0.0, 0.0, 1.0, 0.0}
			}
		};
		instances[i].instanceCustomIndex = i;
		instances[i].mask = 0xFF;
		instances[i].instanceShaderBindingTableRecordOffset = 0;
		instances[i].flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		instances[i].accelerationStructureReference = blases[i].address;
	}

	instances_buffer.memcpy(instances.data(), instances.size() * sizeof(VkAccelerationStructureInstanceKHR));
	
	geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	geometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	geometry.geometry.instances.data.deviceAddress = instances_buffer.address;
	geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;

	build_geometry.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	build_geometry.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	build_geometry.geometryCount = static_cast<uint32_t>(1);
	build_geometry.pGeometries = &geometry;

	RayTracingPipeline::vkGetAccelerationStructureBuildSizesKHR(Context::device.ptr,
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&build_geometry,
		max_primitives.data(),
		&build_sizes_info
	);

	init_as(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR);
}

ShaderBindingTable::ShaderBindingTable(void)
{

}

ShaderBindingTable::~ShaderBindingTable(void)
{

}

void ShaderBindingTable::init(VkPipeline pipeline, uint32_t miss_count, uint32_t hit_count)
{
	uint32_t handle_count = 1 + miss_count + hit_count;
	uint32_t handle_size = Context::device.rtProps.shaderGroupHandleSize;
	uint32_t handle_alignment = Context::device.rtProps.shaderGroupHandleAlignment;
	uint32_t base_alignment = Context::device.rtProps.shaderGroupBaseAlignment;
	uint32_t handle_size_aligned = ALIGN(handle_size, handle_alignment);

	gen_region.stride = ALIGN(handle_size_aligned, Context::device.rtProps.shaderGroupBaseAlignment);
	gen_region.size = gen_region.stride;

	miss_region.stride = handle_size_aligned;
	miss_region.size = ALIGN(miss_count * handle_size_aligned, base_alignment);

	hit_region.stride = handle_size_aligned;
	hit_region.size = ALIGN(hit_count * handle_size_aligned, base_alignment);

	buffer.init(
		gen_region.size + miss_region.size + hit_region.size + call_region.size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
	);
	buffer.map();
	
	gen_region.deviceAddress = buffer.address;
	miss_region.deviceAddress = buffer.address + gen_region.size;
	hit_region.deviceAddress = buffer.address + gen_region.size + miss_region.size;

	uint32_t data_size = handle_count * handle_size;
	std::vector<uint8_t> handles(data_size);

	if (RayTracingPipeline::vkGetRayTracingShaderGroupHandlesKHR(Context::device.ptr, pipeline, 0, handle_count, data_size, handles.data()) != VK_SUCCESS)
		throw std::runtime_error("failed to get shader groups handles");

	auto getHandle = [&](int i) { return handles.data() + i * handle_size; };
	void* data = nullptr;
	int i = 0;

	//RAYGEN 
	data = buffer.data;
	memcpy(data, getHandle(i++), handle_size);

	//MISS
	data = (uint8_t *)buffer.data + gen_region.size;
	memcpy(data, getHandle(i++), handle_size);

	//HIT
	data = (uint8_t*)buffer.data + gen_region.size + miss_region.size;
	memcpy(data, getHandle(i++), handle_size);
}

RayTracingPipeline::RayTracingPipeline(void)
{

}

RayTracingPipeline::~RayTracingPipeline(void)
{
	
}

void RayTracingPipeline::init(void)
{
	VkPipelineLayoutCreateInfo pipeline_layout_create_info{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(layouts.size());
	pipeline_layout_create_info.pSetLayouts = layouts.data();

	if (vkCreatePipelineLayout(Context::device.ptr, &pipeline_layout_create_info, nullptr, &layout) != VK_SUCCESS)
		throw std::runtime_error("failed to create pipeline layout");

	VkRayTracingPipelineCreateInfoKHR create_info = {
		.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
		.pNext = NULL,
		.flags = 0,
		.stageCount = static_cast<uint32_t>(stages.size()),
		.pStages = stages.data(),
		.groupCount = static_cast<uint32_t>(groups.size()),
		.pGroups = groups.data(), //rayTracingShaderGroupCreateInfoList.data(),
		.maxPipelineRayRecursionDepth = 10,
		.pLibraryInfo = NULL,
		.pLibraryInterface = NULL,
		.pDynamicState = NULL,
		.layout = layout,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = 0 
	};

	if (RayTracingPipeline::vkCreateRayTracingPipelinesKHR(Context::device.ptr, nullptr, nullptr, 1, &create_info, nullptr, &ptr) != VK_SUCCESS)
		throw std::runtime_error("failed to create raytracing pipeline");
}