#pragma once

#include <Tempest/TextureAtlas>
#include <cstdint>

namespace Tempest {

class Sprite final {
  public:
    Sprite();

    uint32_t w() const { return texW; }
    uint32_t h() const { return texH; }
    bool     isEmpty() const { return texW<=0 || texH<=0; }

  private:
    Sprite(TextureAtlas::Page* page,uint32_t x,uint32_t y,uint32_t w,uint32_t h);

    TextureAtlas::Page* page=nullptr;
    uint32_t            texW=0;
    uint32_t            texH=0;

  friend class TextureAtlas;
  };

}