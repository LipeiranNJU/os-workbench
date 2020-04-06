// #include <stdio.h>
// #include <dlfcn.h>
// int (*mp)();
// int main() {
//     void* h;
//     h = dlopen("./abc.so", RTLD_NOW|RTLD_GLOBAL);
//     mp = dlsym(h, "a");
//     printf("%d\n",mp());
//     return 0;
// }