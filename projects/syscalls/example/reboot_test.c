#include <unistd.h>
#include <sys/syscall.h>
#include <linux/reboot.h>
#include <stdio.h>

int main() {
    /*
     * The syscall() wrapper takes the target system call number as its
     * first parameter, followed by up to six arguments required by that syscall.
     *
     *   - SYS_reboot: Architecture-specific syscall number (e.g., 169 on x86_64, 142 on arm64)
     *   - Arg 1 (magic1): Primary safety constant 0xfee1dead ("FEEL DEAD")
     *   - Arg 2 (magic2): Secondary safety constant 672274793 (Linus Torvalds' birthday: 28-12-1969)
     *   - Arg 3 (cmd):    Action identifier (LINUX_REBOOT_CMD_RESTART instructs system restart)
     *   - Arg 4 (arg):    User-space payload pointer (NULL here, or passphrase string pointer)
     */
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
