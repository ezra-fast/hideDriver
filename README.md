# Hide LKM

hideDriver.c implements a simple character device driver in pure C and uses DKOM to hide the driver from processes that show the user currently loaded modules and module information (lsmod, modinfo, /proc/modules, etc.) This technique is foundational for many classic and modern Linux-based kernel mode rootkits.
