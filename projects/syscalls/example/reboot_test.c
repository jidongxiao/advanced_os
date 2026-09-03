#include <unistd.h>
#include <sys/syscall.h>
#include <linux/reboot.h>
#include <stdio.h>

int main() {
    // Calling raw syscall 169 (sys_reboot on x86_64) directly:
    int result = syscall(
        SYS_reboot, 
        LINUX_REBOOT_MAGIC1,  // 0xfee1dead ("FEEL DEAD")
        LINUX_REBOOT_MAGIC2,  // 672274793 (Linus' birthday: 28-12-1969)
        LINUX_REBOOT_CMD_RESTART, 
        NULL
    );

    if (result < 0) {
        perror("Reboot failed");
    }

    return 0;
}
