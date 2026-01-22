#include <csignal>

#include "structure_prototype.h"

volatile sig_atomic_t g_quit = 0;
void SignalHandler(int signal) {
  g_quit = signal;
}

int main() {
  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);
  const Process p;
  return p.Run(g_quit);
}
