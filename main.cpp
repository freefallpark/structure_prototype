#include <csignal>
#include <atomic>

#include "structure_prototype.h"

std::atomic<bool> g_quit = false;
static_assert(std::atomic<bool>::is_always_lock_free);
void SignalHandler(int) {
  g_quit.store(true, std::memory_order_relaxed);
}

int main() {
  std::signal(SIGINT, SignalHandler);
  std::signal(SIGTERM, SignalHandler);
  const Process p;
  return p.Run(g_quit);
}
