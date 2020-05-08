#ifndef LIBCCOS_CCOS_HOST_HPP
#define LIBCCOS_CCOS_HOST_HPP

typedef void (*ccos_proc_t)(void);

void ccos_panic_at(char const* message, char const* file, int line) __attribute__((noreturn));
#define ccos_panic(message_) do { ccos_panic_at((message_), __FILE__, __LINE__); } while (0);

void ccos_diag_at(char const* message, char const* file, int line);
#define ccos_diag(message_) do { ccos_diag_at((message_), __FILE__, __LINE__); } while (0);

ccos_proc_t ccos_get_builtin_proc(char const* name);

#endif
