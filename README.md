# wss
```
                    /     \
                   ((     ))
               ===  \\_v_//  ===
 Art by          ====)_^_(====
Roland Waylor    ===/ O O \===
                 = | /_ _\ | =
                =   \/_ _\/   =
                     \_ _/
                     (o_o)
                      VwV
```
### Backlog
- [ ] Run valgrind on build(`cmake`)

```
POLLIN:
POLLOUT: Pure virtual function

POLLRDHUP:
POLLHUP: Shutdown read-write, but wait for EOF

POLLERR: Shutdown read-write, but wait for EF, retry: 1

POLLNVAL: Stop polling

on_eof:
    SHUTDOWN
    CLOSE
    STOP POLLING
```

Struct:
```
FD_DESC
    state - PENDING_EOF
            OPEN
            CLOSED
    fd
    close_retry_counter
```