# Module definitions
obj-m += target_module.o
obj-m += breakpoint_controller.o

# Current running kernel directory
KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

# Target symbol to monitor (override with: make test SYMBOL=custom_func)
SYMBOL ?= target_module_function

.PHONY: all clean load unload test status logs

# Default target: build kernel modules
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Clean build artifacts
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

# Load target module first, then attach the controller
load: all
	@echo "[+] Loading target_module.ko..."
	sudo insmod target_module.ko
	@echo "[+] Attaching breakpoint_controller.ko to symbol: $(SYMBOL)..."
	sudo insmod breakpoint_controller.ko symbol_name="$(SYMBOL)"

# Unload modules in reverse order
unload:
	-sudo rmmod breakpoint_controller 2>/dev/null || true
	-sudo rmmod target_module 2>/dev/null || true
	@echo "[+] Unloaded all debugging modules."

# Check if modules are loaded in kernel
status:
	@lsmod | grep -E "breakpoint_controller|target_module" || echo "[-] Modules are not loaded."

# Stream real-time kernel debug log output
logs:
	sudo dmesg -w | grep -E "\[Controller\]|\[Target\]"
