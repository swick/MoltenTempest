#pragma once

#include "systemapi.h"

namespace Tempest {

class WindowsApi final : SystemApi {
  public:
    static uint16_t translateKey(uint64_t scancode);

  private:
    WindowsApi();

    Window*  implCreateWindow(WindowCallback* cb,uint32_t width,uint32_t height) override;
    Window*  implCreateWindow(WindowCallback* cb,ShowMode sm) override;
    void     implDestroyWindow(Window* w) override;
    void     implExit() override;

    uint32_t implWidth (Window* w) override;
    uint32_t implHeight(Window* w) override;
    Rect     implWindowClientRect(SystemApi::Window *w) override;

    bool     implSetAsFullscreen(SystemApi::Window *w, bool fullScreen) override;
    bool     implIsFullscreen(SystemApi::Window *w) override;

    void     implSetCursorPosition(int x, int y) override;
    void     implShowCursor(bool show) override;

    int      implExec(AppCallBack& cb) override;

    struct KeyInf;
    struct TranslateKeyPair final {
      uint16_t src;
      uint16_t result;
      };

    static void     setupKeyTranslate(const TranslateKeyPair k[], uint16_t funcCount);

    static KeyInf ki;

  friend class SystemApi;
  };

}
