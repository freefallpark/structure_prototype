#include <csignal>
#include <atomic>

#include "structure_prototype.h"

namespace {
// Looking into sig_atomic_t vs std::atomic
// Use std::atomic for signal + potential inter-thread shutdown signaling.
// sig_atomic_t is only guaranteed safe w.r.t. signal interruption, not threads.
std::atomic<bool> g_quit = false;
static_assert(std::atomic<bool>::is_always_lock_free);
void SignalHandler(int) {
  g_quit.store(true, std::memory_order_relaxed);
}
}  // namespace

int main() {
  std::signal(SIGINT, SignalHandler);
  std::signal(SIGTERM, SignalHandler);
  const Process p;
  return p.Run(g_quit);
}
