# Threadpool
  The library for Linux C language to use Threadpool pattern.
  All of this thread is used main loop by libevent, so if user add fd with event, create or add new event in thread.

# How to use
  After build all, please link libevent_threadpool.so library.
  API header is in event_threadpool.h

  Please see wiki "https://github.com/developer-kikikaikai/design_pattern_for_c/wiki/Threadpool" for more detai.

# To use full fd

Please update /etc/sysctl.conf

```
net.ipv4.tcp_max_syn_backlog = 8192
net.core.somaxconn=8192
```

and up /etc/security/limits.conf like

```
*    soft nofile 8192
*    hard nofile 8192
root soft nofile 8192
root hard nofile 8192
```

# How to check max of fds?

Please check

```
ulimit -Sn
```
