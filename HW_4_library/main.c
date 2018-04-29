#include <stdio.h>
#include <dlfcn.h>
#include <errno.h>

// описываем функцию f1() как внешнюю
extern int f1(),f2(),f3();

int main(){
  void *dl_handle;
  int (*n4)();
	int n1, n2, n3;

	n1 = f1();
	n2 = f2();
  n3 = f3();
  printf("result of f1() (from static library) = %d\n",n1);
	printf("result of f2() (from static library) = %d\n",n2);
  printf("result of f3() (from dynamic library) = %d\n",n3);

  dl_handle = dlopen( "libfsdyn2.so", RTLD_LAZY );
    if (!dl_handle) {
    printf( "!!! %s\n", dlerror() );
    errno;
  }
  n4 = dlsym(dl_handle, "f4");

  printf("result of f4() (from dynamic library (with dlopen) = %d\n",n4());

	return 0;
}
