# The Story Behind sys_reboot Magic Numbers

The signature for the Linux reboot system call is:

```c
long sys_reboot(int magic1, int magic2, unsigned int cmd, void __user *arg);
```

At first glance, passing two separate "magic" numbers just to restart a computer seems redundant. However, these numbers serve as a fail-safe against accidental system destruction.

## Why Magic Numbers Exist

In early C programming, calling a function via an uninitialized pointer, a corrupted stack, or an incorrect syscall table index could cause the CPU to jump directly into sys_reboot. If sys_reboot took no arguments, any accidental jump into kernel memory would immediately hard-reset the machine, causing catastrophic data loss.

To prevent accidental invocations, Linus Torvalds required sys_reboot to prove caller intentionality: the system call must pass two specific 32-bit constants in exact order. If either constant is wrong, the kernel rejects the call immediately with -EINVAL.

## The Easter Egg Values

Linus embedded personal easter eggs directly into the kernel source code for these magic constants:

```c
LINUX_REBOOT_MAGIC1 = 0xfee1dead (4276215469)
```

Written in hex, this spells out "FEEL DEAD"—a dark-humor reference to forcing the operating system to shut down or restart.

```c
LINUX_REBOOT_MAGIC2 = 672274793
```

Converting this decimal number into a 6-digit date format (28-12-1969 or 28121969) reveals Linus Torvalds' date of birth (December 28, 1969).

Later kernel versions introduced alternate valid constants for MAGIC2 that encode the birthdates (in DDMMYYYY hex format) of Linus Torvalds' three daughters: MAGIC2A (0x05121996 / Patricia, Dec 5, 1996), MAGIC2B (0x16041998 / Dhani, Apr 16, 1998), and MAGIC2C (0x20112000 / Celeste, Nov 20, 2000). The kernel accepts any of these values alongside MAGIC1, though LINUX_REBOOT_MAGIC2 (0x28121969 / Dec 28, 1969, Linus's own birthday) remains the classic default.

As a reference, the following code, which is the kernel implementation (version 7.0) of the reboot system call, defined in the kernel source tree kernel/reboot, shows how these magic numbers are used.

```c
/*
 * Reboot system call: for obvious reasons only root may call it,
 * and even root needs to set up some magic numbers in the registers
 * so that some mistake won't make this reboot the whole machine.
 * You can also set the meaning of the ctrl-alt-del-key here.
 *
 * reboot doesn't sync: do that yourself before calling this.
 */
SYSCALL_DEFINE4(reboot, int, magic1, int, magic2, unsigned int, cmd,
		void __user *, arg)
{
	struct pid_namespace *pid_ns = task_active_pid_ns(current);
	char buffer[256];
	int ret = 0;

	/* We only trust the superuser with rebooting the system. */
	if (!ns_capable(pid_ns->user_ns, CAP_SYS_BOOT))
		return -EPERM;

	/* For safety, we require "magic" arguments. */
	if (magic1 != LINUX_REBOOT_MAGIC1 ||
			(magic2 != LINUX_REBOOT_MAGIC2 &&
			magic2 != LINUX_REBOOT_MAGIC2A &&
			magic2 != LINUX_REBOOT_MAGIC2B &&
			magic2 != LINUX_REBOOT_MAGIC2C))
		return -EINVAL;

	/*
	 * If pid namespaces are enabled and the current task is in a child
	 * pid_namespace, the command is handled by reboot_pid_ns() which will
	 * call do_exit().
	 */
	ret = reboot_pid_ns(pid_ns, cmd);
	if (ret)
		return ret;

	/* Instead of trying to make the power_off code look like
	 * halt when pm_power_off is not set do it the easy way.
	 */
	if ((cmd == LINUX_REBOOT_CMD_POWER_OFF) && !kernel_can_power_off()) {
		poweroff_fallback_to_halt = true;
		cmd = LINUX_REBOOT_CMD_HALT;
	}

	mutex_lock(&system_transition_mutex);
	switch (cmd) {
	case LINUX_REBOOT_CMD_RESTART:
		kernel_restart(NULL);
		break;

	case LINUX_REBOOT_CMD_CAD_ON:
		C_A_D = 1;
		break;

	case LINUX_REBOOT_CMD_CAD_OFF:
		C_A_D = 0;
		break;

	case LINUX_REBOOT_CMD_HALT:
		kernel_halt();
		do_exit(0);

	case LINUX_REBOOT_CMD_POWER_OFF:
		kernel_power_off();
		do_exit(0);
		break;

	case LINUX_REBOOT_CMD_RESTART2:
		ret = strncpy_from_user(&buffer[0], arg, sizeof(buffer) - 1);
		if (ret < 0) {
			ret = -EFAULT;
			break;
		}
		buffer[sizeof(buffer) - 1] = '\0';

		kernel_restart(buffer);
		break;

#ifdef CONFIG_KEXEC_CORE
	case LINUX_REBOOT_CMD_KEXEC:
		ret = kernel_kexec();
		break;
#endif

#ifdef CONFIG_HIBERNATION
	case LINUX_REBOOT_CMD_SW_SUSPEND:
		ret = hibernate();
		break;
#endif

	default:
		ret = -EINVAL;
		break;
	}
	mutex_unlock(&system_transition_mutex);
	return ret;
}
```

and in include/uapi/linux/reboot.h, we can find the definition of these magic numbers.

```c
/*
 * Magic values required to use _reboot() system call.
 */

#define LINUX_REBOOT_MAGIC1     0xfee1dead
#define LINUX_REBOOT_MAGIC2     672274793
#define LINUX_REBOOT_MAGIC2A    85072278
#define LINUX_REBOOT_MAGIC2B    369367448
#define LINUX_REBOOT_MAGIC2C    537993216
```
