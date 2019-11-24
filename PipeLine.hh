#include <sys/types.h>
#include <memory>
#include <atomic>

template<class M, class I>
class PipeLine {
  volatile std::atomic<M*> wPtr, rPtr;
  size_t size;
  M *Buf, *End;
public:
  I remote;
  struct slice {
    M *p;
    ptrdiff_t cnt;
    void next() { cnt--; p++; };
  };
  slice rd() {
    M *r = rPtr.load(std::memory_order_relaxed);
    M *w = wPtr.load(std::memory_order_relaxed);
    if(r == w) return {.p=r, .cnt=0};
    if(w <  r) return {.p=r, .cnt=End - r};
    return {.p=r, .cnt=w - r};
  };
  slice wr() {
    M *w = wPtr.load(std::memory_order_relaxed);
    M *e = w+1; if(e == End) e = Buf;
    M *r = rPtr.load(std::memory_order_relaxed);
    if(r == e) { return {.p=w, .cnt=0}; }
    if(r >  w) { return {.p=w, .cnt=r - e}; }
    if(r == Buf) { return {.p=w, .cnt=End - e}; }
    return {.p=w, .cnt=End - w};
  };
  void rd(M *p) { rPtr.store((p == End) ? Buf : p, std::memory_order_release); };
  void wr(M *p) { wPtr.store((p == End) ? Buf : p, std::memory_order_release); };
  PipeLine() {};
  PipeLine(size_t count) { size = count; wPtr = rPtr = Buf = new M[count]; End = &Buf[count]; }
} __aligned(64);

