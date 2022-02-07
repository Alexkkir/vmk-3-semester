#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>


struct Args
{
    unsigned char arr[64];
};

int
main(int argc, char **argv)
{
    // libc.so.6 printf vssid 'Hello: %s, %d, %f' abc 10 12.4
    // 1         2      3      4...
    void *handle;
    handle = dlopen(argv[1], RTLD_LAZY);

    char r = argv[3][0];
    int pos = 0;
    struct Args args;

    void (*fun_void)(struct Args a) = NULL;
    char* (*fun_str)(struct Args a) = NULL;
    int (*fun_int)(struct Args a) = NULL;
    double (*fun_double)(struct Args a) = NULL;

    switch (r) {
        case 'v':
            fun_void = dlsym(handle, argv[2]);
            break;
        case 's':
            fun_str = dlsym(handle, argv[2]);
            break;
        case 'i':
            fun_int = dlsym(handle, argv[2]);
            break;
        case 'd':
            fun_double = dlsym(handle, argv[2]);
            break;
        default:
            exit(1);
            break;
    }

    for (int i = 4, j = 1; i < argc; i++, j++) {
        char type = argv[3][j];
        int size = 0;

        switch (type) {
            case 's':
                size = sizeof(char*);
                memcpy(&args.arr[pos], &argv[i], size);
                break;
            case 'i':
                size = sizeof(int);
                int x = strtol(argv[i], NULL, 10);
                memcpy(&args.arr[pos], &x, size);
                break;
            case 'd':
                size = sizeof(double);
                double d = strtod(argv[i], NULL);
                memcpy(&args.arr[pos], &d, size);
                break;
            default:
                exit(1);
                break;
        }
        pos += size;
    }

    switch (r) {
        case 'v':
            fun_void(args);
            break;
        case 's':
            printf("%s\n", fun_str(args));
            break;
        case 'i':
            printf("%d\n", fun_int(args));
            break;
        case 'd':
            printf("%.10g\n", fun_double(args));
            break;
        default:
            exit(1);
            break;
    }
    return 0;
}