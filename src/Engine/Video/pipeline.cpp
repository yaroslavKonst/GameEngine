#include "pipeline.h"

#include "../Logger/logger.h"

Pipeline::Pipeline(InitInfo* initInfo)
{
	_device = initInfo->Device;
	_extent = initInfo->Extent;
	_colorAttachmentFormat = initInfo->ColorAttachmentFormat;
	_depthAttachmentFormat = initInfo->DepthAttachmentFormat;
	_msaaSamples = initInfo->MsaaSamples;
	_resolve = initInfo->ResolveImage;
	_color = initInfo->ColorImage;
	_depth = initInfo->DepthImage;
	_clearColorImage = initInfo->ClearColorImage;

	bool geometry = initInfo->GeometryShaderSize > 0;

	VkShaderModule vertShaderModule = CreateShaderModule(
		initInfo->VertexShaderCode,
		initInfo->VertexShaderSize);

	VkShaderModule fragShaderModule = CreateShaderModule(
		initInfo->FragmentShaderCode,
		initInfo->FragmentShaderSize);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	shaderStages.push_back(vertShaderStageInfo);
	shaderStages.push_back(fragShaderStageInfo);

	VkShaderModule geomShaderModule;

	if (geometry) {
		geomShaderModule = CreateShaderModule(
			initInfo->GeometryShaderCode,
			initInfo->GeometryShaderSize);

		VkPipelineShaderStageCreateInfo geometryStageInfo{};
		geometryStageInfo.sType =
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		geometryStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		geometryStageInfo.module = geomShaderModule;
		geometryStageInfo.pName = "main";

		shaderStages.push_back(geometryStageInfo);
	}

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount =
		static_cast<uint32_t>(
			initInfo->VertexBindingDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions =
		initInfo->VertexBindingDescriptions.data();
	vertexInputInfo.vertexAttributeDescriptionCount =
		static_cast<uint32_t>(
			initInfo->VertexAttributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions =
		initInfo->VertexAttributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)_extent.width;
	viewport.height = (float)_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = _extent;

	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType =
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount =
		static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType =
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	if (initInfo->InvertFace) {
		rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
	}

	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType =
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_TRUE;
	multisampling.rasterizationSamples = _msaaSamples;
	multisampling.minSampleShading = 0.2f;
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType =
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = initInfo->DepthTestEnabled;
	depthStencil.depthWriteEnable = initInfo->DepthWriteEnabled;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;

	if (initInfo->AlphaBlending) {
		colorBlendAttachment.srcColorBlendFactor =
			VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor =
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	} else {
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
	}

	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_MAX;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType =
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount =
		static_cast<uint32_t>(initInfo->DescriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = initInfo->DescriptorSetLayouts.data();

	pipelineLayoutInfo.pushConstantRangeCount =
		initInfo->PushConstantRangeCount;

	if (initInfo->PushConstantRangeCount > 0) {
		pipelineLayoutInfo.pPushConstantRanges =
			initInfo->PushConstants;
	}

	VkResult res = vkCreatePipelineLayout(
		_device,
		&pipelineLayoutInfo,
		nullptr,
		&_pipelineLayout);

	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline layout.");
	}

	CreateRenderPass(initInfo);

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = _pipelineLayout;
	pipelineInfo.renderPass = _renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	res = vkCreateGraphicsPipelines(
		_device,
		VK_NULL_HANDLE,
		1,
		&pipelineInfo,
		nullptr,
		&_pipeline);
	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline.");
	}

	DestroyShaderModule(vertShaderModule);
	DestroyShaderModule(fragShaderModule);

	if (geometry) {
		DestroyShaderModule(geomShaderModule);
	}

	Logger::Verbose() << "Created pipeline.";
}

Pipeline::~Pipeline()
{
	vkDestroyPipeline(_device, _pipeline, nullptr);
	DestroyRenderPass();
	vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
	Logger::Verbose() << "Destroyed pipeline.";
}

VkShaderModule Pipeline::CreateShaderModule(const uint8_t* data, size_t size)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = size;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(data);

	VkShaderModule shaderModule;

	VkResult res = vkCreateShaderModule(
		_device,
		&createInfo,
		nullptr,
		&shaderModule);

	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to create shader module.");
	}

	Logger::Verbose() << "Created shader module.";

	return shaderModule;
}

void Pipeline::DestroyShaderModule(VkShaderModule shaderModule)
{
	vkDestroyShaderModule(_device, shaderModule, nullptr);
	Logger::Verbose() << "Destroyed shader module.";
}

void Pipeline::CreateRenderPass(InitInfo* initInfo)
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = _colorAttachmentFormat;
	colorAttachment.samples = _msaaSamples;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.finalLayout = initInfo->ColorImageFinalLayout;

	if (_clearColorImage) {
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	} else {
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.initialLayout =
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	uint32_t attachmentIndex = 0;

	VkAttachmentReference colorAttachmentRef{};
	if (_color) {
		colorAttachmentRef.attachment = attachmentIndex;
		++attachmentIndex;
	}

	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = _depthAttachmentFormat;
	depthAttachment.samples = _msaaSamples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = initInfo->DepthImageFinalLayout;

	if (!initInfo->ClearDepthImage) {
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		depthAttachment.initialLayout =
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	VkAttachmentReference depthAttachmentRef{};
	if (_depth) {
		depthAttachmentRef.attachment = attachmentIndex;
		++attachmentIndex;
	}

	depthAttachmentRef.layout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachmentResolve{};
	colorAttachmentResolve.format = _colorAttachmentFormat;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp =
		VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout =
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentReference colorAttachmentResolveRef{};
	colorAttachmentResolveRef.attachment = attachmentIndex;
	colorAttachmentResolveRef.layout =
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = _color ? 1 : 0;

	if (_color) {
		subpass.pColorAttachments = &colorAttachmentRef;
	}

	if (_depth) {
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
	}

	if (_resolve) {
		subpass.pResolveAttachments = &colorAttachmentResolveRef;
	}

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask =
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask =
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask =
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::vector<VkAttachmentDescription> attachments;

	if (_color) {
		attachments.push_back(colorAttachment);
	}

	if (_depth) {
		attachments.push_back(depthAttachment);
	}

	if (_resolve) {
		attachments.push_back(colorAttachmentResolve);
	}

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount =
		static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VkResult res = vkCreateRenderPass(
		_device,
		&renderPassInfo,
		nullptr,
		&_renderPass);

	if (res != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass.");
	}
}

void Pipeline::DestroyRenderPass()
{
	vkDestroyRenderPass(_device, _renderPass, nullptr);
}

void Pipeline::CreateFramebuffers(
	const std::vector<VkImageView>& resolveImageViews,
	const std::vector<VkImageView>& colorImageViews,
	const std::vector<VkImageView>& depthImageViews,
	uint32_t layerCount)
{
	_framebuffers.resize(
		resolveImageViews.size() *
		depthImageViews.size() *
		colorImageViews.size());

	for (size_t i = 0; i < resolveImageViews.size(); ++i) {
		for (size_t d = 0; d < depthImageViews.size(); ++d) {
			for (size_t c = 0; c < colorImageViews.size(); ++c) {
				std::vector<VkImageView> attachments;

				if (_color) {
					attachments.push_back(
						colorImageViews[c]);
				}

				if (_depth) {
					attachments.push_back(
						depthImageViews[d]);
				}

				if (_resolve) {
					attachments.push_back(
						resolveImageViews[i]);
				}

				VkFramebufferCreateInfo framebufferInfo{};
				framebufferInfo.sType =
					VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = _renderPass;
				framebufferInfo.attachmentCount =
					static_cast<uint32_t>(
						attachments.size());
				framebufferInfo.pAttachments =
					attachments.data();
				framebufferInfo.width = _extent.width;
				framebufferInfo.height = _extent.height;
				framebufferInfo.layers = layerCount;

				VkResult res = vkCreateFramebuffer(
					_device,
					&framebufferInfo,
					nullptr,
					&_framebuffers[
						i * depthImageViews.size() *
						colorImageViews.size() + d *
						colorImageViews.size() + c]);

				if (res != VK_SUCCESS) {
					throw std::runtime_error(
						"Failed to create framebuffer.");
				}
			}
		}
	}
}

void Pipeline::DestroyFramebuffers()
{
	for (auto framebuffer : _framebuffers) {
		vkDestroyFramebuffer(_device, framebuffer, nullptr);
	}
}

void Pipeline::RecordCommandBuffer(
	VkCommandBuffer commandBuffer,
	uint32_t imageIndex,
	float colorClearAlphaValue)
{
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = _renderPass;
	renderPassInfo.framebuffer = _framebuffers[imageIndex];
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = _extent;

	std::vector<VkClearValue> clearValues;

	if (_color) {
		VkClearValue clearValue{};
		clearValue.color = {{0.0f, 0.0f, 0.0f, colorClearAlphaValue}};
		clearValues.push_back(clearValue);
	}

	if (_depth) {
		VkClearValue clearValue{};
		clearValue.depthStencil = {1.0f, 0};
		clearValues.push_back(clearValue);
	}

	renderPassInfo.clearValueCount =
		static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(
		commandBuffer,
		&renderPassInfo,
		VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		_pipeline);
}
