#pragma once

#include <Tempest/AbstractGraphicsApi>
#include "../utility/dptr.h"

namespace Tempest {

class Device;
class CommandBuffer;
class PrimaryCommandBuffer;
class Frame;
class FrameBuffer;
class Texture2d;

class FrameBufferLayout final {
  public:
    uint32_t w() const { return mw; }
    uint32_t h() const { return mh; }

  private:
    FrameBufferLayout()=default;
    FrameBufferLayout(Detail::DSharedPtr<AbstractGraphicsApi::FboLayout*>&& f,uint32_t w,uint32_t h);

    Detail::DSharedPtr<AbstractGraphicsApi::FboLayout*> impl;
    uint32_t                                            mw=0, mh=0;

  friend class Tempest::Device;
  friend class Tempest::FrameBuffer;
  };

class FrameBuffer final {
  public:
    FrameBuffer()=default;
    FrameBuffer(FrameBuffer&& f)=default;
    ~FrameBuffer();
    FrameBuffer& operator = (FrameBuffer&& other)=default;

    uint32_t w() const { return lay.mw; }
    uint32_t h() const { return lay.mh; }

    auto     layout() const -> const FrameBufferLayout&;

  private:
    FrameBuffer(Tempest::Device& dev,
                Detail::DSharedPtr<AbstractGraphicsApi::Fbo*>&& f,
                FrameBufferLayout&& lay);

    Tempest::Device*                              dev=nullptr;
    Detail::DSharedPtr<AbstractGraphicsApi::Fbo*> impl;
    FrameBufferLayout                             lay;

  friend class Tempest::Device;
  friend class Tempest::CommandBuffer;
  friend class Tempest::PrimaryCommandBuffer;
  };
}
