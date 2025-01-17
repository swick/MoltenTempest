#include "textcodec.h"

#include "utf8_helper.h"

using namespace Tempest;

std::string TextCodec::toUtf8(const std::u16string &s) {
  return toUtf8(s.c_str());
  }

std::string TextCodec::toUtf8(const char16_t *s) {
  size_t sz=0;

  for(size_t i=0;s[i];++i)
    sz += Detail::codepointToUtf8(s[i]);

  std::string u(sz,'?');
  sz=0;

  for(size_t i=0;s[i];++i)
    sz += Detail::codepointToUtf8(s[i],&u[sz]);

  return u;
  }

std::u16string TextCodec::toUtf16(const std::string &s) {
  return toUtf16(s.c_str());
  }

std::u16string TextCodec::toUtf16(const char *inS) {
  const uint8_t* s  = reinterpret_cast<const uint8_t*>(inS);
  size_t         sz = 0;

  for(size_t i=0;s[i];) {
    uint32_t cp = 0;
    size_t   l  = Detail::utf8ToCodepoint(&s[sz],cp);

    if(cp > 0xFFFF)
      sz+=2; else
      sz+=1;

    i+=l;
    }

  std::u16string u(sz,'?'); sz=0;
  for(size_t i=0;i<sz;) {
    uint32_t cp = 0;
    size_t   l  = Detail::utf8ToCodepoint(&s[sz],cp);

    if(cp > 0xFFFF) {
      cp -= 0x10000;
      u[i] = 0xD800 + ((cp >> 10) & 0x3FF);
      cp = 0xDC00 + (cp & 0x3FF);
      ++i;
      }
    u[i] = char16_t(cp);
    ++i;
    }

  return u;
  }
