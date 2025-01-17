#pragma once

#include <Tempest/AbstractGraphicsApi>
#include "videobuffer.h"
#include <vector>

namespace Tempest {

class Device;

template<class T>
class VertexBufferDyn;

template<class T>
class VertexBuffer {
  public:
    VertexBuffer()=default;
    VertexBuffer(VertexBuffer&&)=default;
    VertexBuffer& operator=(VertexBuffer&&)=default;
    virtual ~VertexBuffer()=default;

    size_t size() const { return sz; }

  private:
    VertexBuffer(Tempest::VideoBuffer&& impl,size_t size)
      :impl(std::move(impl)),sz(size) {
      }

    Tempest::VideoBuffer impl;
    size_t               sz=0;

  friend class Tempest::Device;
  friend class Tempest::CommandBuffer;
  friend class Tempest::VertexBufferDyn<T>;
  };

template<class T>
class VertexBufferDyn:public VertexBuffer<T> {
  public:
    VertexBufferDyn()=default;
    VertexBufferDyn(VertexBufferDyn&&)=default;
    VertexBufferDyn& operator=(VertexBufferDyn&&)=default;

    void   update(const std::vector<T>& v)                    { return this->impl.update(v.data(),0,v.size()*sizeof(T)); }
    void   update(const void* data,size_t offset,size_t size) { return this->impl.update(data,offset,size); }

  private:
    VertexBufferDyn(Tempest::VideoBuffer&& impl,size_t size)
      :VertexBuffer<T>(std::move(impl),size) {
      }
  friend class Tempest::Device;
  };

template<class T>
inline std::initializer_list<Decl::ComponentType> vertexBufferDecl(){ return{}; };
}
