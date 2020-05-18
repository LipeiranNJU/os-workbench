// #include <stdio.h>
// #include <dlfcn.h>
// int (*mp)();
// int main() {
//     void* h;
//     h = dlopen("./wrapper.so", RTLD_NOW|RTLD_GLOBAL);
//     mp = dlsym(h, "__expr");
//     printf("%d\n",mp());
//     return 0;
// }