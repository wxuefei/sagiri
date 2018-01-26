#include<driver/uart.h>
#include<lib/x64.h>
#include<lib/stdio.h>
#include<lib/string.h>
#include<mm/mm.h>
#include<mm/seg.h>
#include<mm/vm.h>
#include<mm/malloc.h>
#include<driver/acpi.h>
#include<driver/lapic.h>
#include<driver/ioapic.h>
#include<trap/trap.h>
#include<proc/cpu.h>
#include<sync/spinlock.h>
char bspstack[4096];
void _startmp();
static void startothers() {
    extern char _binary_out_entrymp_start[], _binary_out_entrymp_size[];
    memcopy((char*)p2k(0x7000), _binary_out_entrymp_start, (uint64_t)_binary_out_entrymp_size);
    extern char _binary_out_entrymp64_start[], _binary_out_entrymp64_size[];
    memcopy((char*)p2k(0x7200), _binary_out_entrymp64_start, (uint64_t)_binary_out_entrymp64_size);
    bsp = cpu;
    for(struct cpu* c = cpus; c < cpus + cpuno; c++) {
        //不需要启动自身
        if(c == cpu) {
            continue;
        }
        uint64_t stack = alloc();
        *(uint64_t*)(p2k(0x7000) - 8) = (uint64_t)_startmp;
        *(uint64_t*)(p2k(0x7000) - 16) = (uint64_t)(stack + 4096);
        *(uint64_t*)(p2k(0x7000) - 24) = k2p((uint64_t)kpgdir);
        lapicstartup(c->apicid, 0x7000);
        while(c->started == 0) {
            ;
        }
    }
}
void mpstart() {
    cpuinit();
    seginit();
    idtinit();
    lapicinit();
    printf("cpu %d starting\n", cpu->id);
    xchg(&(cpu->started), 1);
    while(bsp->started == 0){
        ;
    }
    //popcli();
    while(1) {
        ;
    }
}
int64_t main() {
    uartearlyinit();
    printf("loading...\n");
    mminit();
    acpiinit();
    cpuinit();
    seginit();
    idtinit();
    lapicinit();
    ioapicinit();
    uartinit();
    startothers();
    printf("cpu %d starting\n", cpu->id);
    xchg(&(cpu->started), 1);
    systemstarted = 1;
    popcli();
    while(1) {
        ;
    }
}
