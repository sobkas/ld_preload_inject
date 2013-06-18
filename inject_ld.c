#include "redirect.h"
#include <stdio.h>
#include <dlfcn.h>
#include <assert.h>
#include <errno.h>        
#include <string.h>
#include <link.h>

typedef int (*main_t)(int, char **, char **);
main_t realmain;
void (*func_a_replaced)(int some_arg);
int wrap_main(int argc, char **argv, char **environ);

void log_func(const char *file, int line, int level, const char *str) {
  char *level_str = "";
  switch(level) {
  case libredirect_log_error:
    level_str = "error"; break;
  case libredirect_log_warning:
    level_str = "warning"; break;
  case libredirect_log_call:
    level_str = "call"; break;
  case libredirect_log_info:
    level_str = "info"; break;
  }
  printf("[%s at %s:%d] %s\n", level_str, file, line, str);
}

int __libc_start_main(main_t main,
                      int argc,
                      char *__unbounded *__unbounded ubp_av,
                      ElfW(auxv_t) *__unbounded auxvec,
                      __typeof (main) init,
                      void (*fini) (void),
                      void (*rtld_fini) (void), void *__unbounded
                      stack_end)
{
  void *libc;
  int (*libc_start_main)(main_t main,
			 int,
			 char *__unbounded *__unbounded,
			 ElfW(auxv_t) *,
			 __typeof (main),
			 void (*fini) (void),
			 void (*rtld_fini) (void),
			 void *__unbounded stack_end);

  libc = dlopen("libc.so.6", RTLD_LOCAL  | RTLD_LAZY);
  if (!libc)
    ERROR("  dlopen() failed: %s\n", dlerror());
  libc_start_main = dlsym(libc, "__libc_start_main");
  if (!libc_start_main)
    ERROR("     Failed: %s\n", dlerror());

  realmain = main;
  return (*libc_start_main)(wrap_main, argc, ubp_av, auxvec,
			    init, fini, rtld_fini, stack_end);
}
void print_b(void)
{
  printf("%s","B");
}


int wrap_main(int argc, char **argv, char **environ)
{
 int err = 0;
 void *self = dlopen(NULL, RTLD_LAZY);
 void *print_a_add = dlsym(self, "print");
 void *print_b_add = dlsym(self, "print_b");
 func_a_replaced = NULL; 



 libredirect_set_log(log_func, libredirect_log_all);
 libredirect_init();
 if((err = libredirect_redirect(print_a_add, print_b_add, (void **)&func_a_replaced))) {
   printf("libredirect_redirect failed: %s (%d)\n", strerror(err), err);
   return err;
 }
 (*realmain)(argc, argv, environ);
}
