#include "vrenderpass.h"

#include "vdevice.h"
#include "vframebufferlayout.h"
#include "vswapchain.h"

#include <Tempest/RenderPass>

using namespace Tempest::Detail;

VRenderPass::VRenderPass(VDevice& device, VSwapchain& sw, const Attachment **attach, uint8_t attCount)
  : attCount(attCount), device(device.device), swapchain(&sw) {
  input.reset(new Attachment[attCount]);
  for(size_t i=0;i<attCount;++i)
    input[i] = *attach[i];
  //impl = createInstance(device.device,sw,attach,attCount);
  }

VRenderPass::VRenderPass(VRenderPass &&other) {
  std::swap(attCount,other.attCount);
  std::swap(device,other.device);
  std::swap(impl,other.impl);
  std::swap(input,other.input);
  }

VRenderPass::~VRenderPass(){
  if(device==nullptr)
    return;
  vkDeviceWaitIdle(device);
  for(auto& i:impl)
    if(i.impl!=VK_NULL_HANDLE)
      vkDestroyRenderPass(device,i.impl,nullptr);
  }

void VRenderPass::operator=(VRenderPass &&other) {
  std::swap(attCount,other.attCount);
  std::swap(device,other.device);
  std::swap(swapchain,other.swapchain);
  std::swap(impl,other.impl);
  std::swap(input,other.input);
  }

VRenderPass::Impl &VRenderPass::instance(VFramebufferLayout &lay) {
  for(auto& i:impl)
    if(lay.isCompatible(*i.lay.handler))
      return i;

  VkRenderPass val=VK_NULL_HANDLE;
  try {
    std::unique_ptr<VkClearValue[]> clear(new VkClearValue[attCount]());
    for(size_t i=0;i<attCount;++i) {
      auto& cl = input[i].clear;
      if(Detail::nativeIsDepthFormat(lay.frm[i])){
        clear[i].depthStencil.depth   = cl.r();
        clear[i].depthStencil.stencil = 0;
        } else {
        clear[i].color.float32[0] = cl.r();
        clear[i].color.float32[1] = cl.g();
        clear[i].color.float32[2] = cl.b();
        clear[i].color.float32[3] = cl.a();
        }
      }
    val = createInstance(device,*swapchain,input.get(),lay.frm.get(),attCount);
    impl.emplace_back(lay,val,std::move(clear));
    }
  catch(...) {
    if(val!=VK_NULL_HANDLE)
      vkDestroyRenderPass(device,val,nullptr);
    throw;
    }
  return impl.back();
  }

VkRenderPass VRenderPass::createInstance(VkDevice &device, VSwapchain& sw,
                                         const Attachment *att, const VkFormat* format, uint8_t attCount) {
  VkRenderPass ret=VK_NULL_HANDLE;

  VkAttachmentDescription attach[2] = {};
  VkAttachmentReference   ref[2]    = {};

  VkSubpassDescription subpass    = {};
  subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.pColorAttachments       = nullptr;
  subpass.pDepthStencilAttachment = nullptr;

  for(size_t i=0;i<attCount;++i){
    VkAttachmentDescription& a = attach[i];
    VkAttachmentReference&   r = ref[i];
    const Attachment&        x = att[i];
    const VkFormat           f = format[i];

    a.format         = f;
    a.samples        = VK_SAMPLE_COUNT_1_BIT;

    a.loadOp         = bool(x.mode&FboMode::PreserveIn)  ? VK_ATTACHMENT_LOAD_OP_LOAD   : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    a.storeOp        = bool(x.mode&FboMode::PreserveOut) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;

    if(a.format==VK_FORMAT_UNDEFINED)
      a.format = sw.format();

    if(bool(x.mode&FboMode::Clear)) {
      a.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      }

    // Stencil not implemented
    a.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    a.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    r.attachment=i;
    if(Detail::nativeIsDepthFormat(f)) {
      subpass.pDepthStencilAttachment = &r;
      a.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      a.finalLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      r.layout        = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      } else {
      subpass.colorAttachmentCount    = 1; //TODO: multi-render target
      subpass.pColorAttachments       = &r;
      a.initialLayout  = bool(x.mode&FboMode::PreserveIn)          ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
      a.finalLayout    = (x.mode&FboMode::Submit)==FboMode::Submit ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR          : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      r.layout         = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      }
    }

  VkSubpassDependency dependency = {};
  dependency.srcSubpass      = VK_SUBPASS_EXTERNAL;
  dependency.srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;//VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

  dependency.dstSubpass      = 0;
  dependency.dstStageMask    = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;//VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask   = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;

  dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = attCount;
  renderPassInfo.pAttachments    = attach;
  renderPassInfo.subpassCount    = 1;
  renderPassInfo.pSubpasses      = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies   = &dependency;

  vkAssert(vkCreateRenderPass(device,&renderPassInfo,nullptr,&ret));
  return ret;
  }

VkRenderPass VRenderPass::createLayoutInstance(VkDevice& device,VSwapchain &sw, const VkFormat *att, uint8_t attCount) {
  VkRenderPass ret=VK_NULL_HANDLE;

  VkAttachmentDescription attach[2] = {};
  VkAttachmentReference   ref[2]    = {};

  VkSubpassDescription subpass    = {};
  subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.pColorAttachments       = nullptr;
  subpass.pDepthStencilAttachment = nullptr;

  for(size_t i=0;i<attCount;++i){
    VkAttachmentDescription& a = attach[i];
    VkAttachmentReference&   r = ref[i];

    if(att[i]==VK_FORMAT_UNDEFINED)
      a.format = sw.format(); else
      a.format = att[i];
    a.samples        = VK_SAMPLE_COUNT_1_BIT;

    a.loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    a.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    a.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    a.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    r.attachment=i;
    if(Detail::nativeIsDepthFormat(a.format)) {
      subpass.pDepthStencilAttachment = &r;
      a.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      a.finalLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      r.layout        = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      } else {
      subpass.colorAttachmentCount    = 1; //TODO: multi-render target
      subpass.pColorAttachments       = &r;
      a.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
      a.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      r.layout         = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      }
    }

  VkSubpassDependency dependency = {};
  dependency.srcSubpass      = VK_SUBPASS_EXTERNAL;
  dependency.srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependency.srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
  dependency.dstSubpass      = 0;
  dependency.dstStageMask    = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  dependency.dstAccessMask   = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
  dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = attCount;
  renderPassInfo.pAttachments    = attach;
  renderPassInfo.subpassCount    = 1;
  renderPassInfo.pSubpasses      = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies   = &dependency;

  vkAssert(vkCreateRenderPass(device,&renderPassInfo,nullptr,&ret));
  return ret;
  }
