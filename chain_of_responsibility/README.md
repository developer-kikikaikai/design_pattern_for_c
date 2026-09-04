# ChainOfResponsibility
  The library for Linux C language to use ChainOfResponsibility pattern.

# How to use
  After build all, please link libchainofresp.so library.
  API header is in chain_of_responsibility.h

  Prefer `cor_manager_new()` when an application needs independent chains or
  multi-thread use. The original `cor_*` functions remain available and use a
  process-wide default manager for compatibility.

  Chain callbacks run without an internal lock, so they may add or remove
  elements recursively. The application owns callback context data and must
  keep it alive until concurrent calls have completed.

  Please see wiki "https://github.com/developer-kikikaikai/design_pattern_for_c/wiki/Chain-of-Responsibility" for more detai.
