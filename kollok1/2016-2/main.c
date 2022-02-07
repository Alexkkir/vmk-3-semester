#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>


struct cell {
    pid_t pid;
    unsigned virt_page;
};

unsigned func(struct cell *Table, unsigned addr) {
    unsigned table_size = 1 << (32 - 12);
    unsigned virt_page = addr / 4096;
    unsigned offset = addr % 4096;

    for (unsigned i = 0; i < table_size; i++) {
        if (Table[i].pid == getpid() && Table[i].virt_page == virt_page) {
            unsigned phys_page = i;
            unsigned phys_addr = phys_page * 4096 + offset;
            return phys_addr;
        }
    }
    return 0;
}

int main() {
    printf("Hello, World!\n");
    return 0;
}
