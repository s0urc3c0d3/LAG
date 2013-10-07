#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x626a41e3, "module_layout" },
	{ 0x4640139a, "cdev_del" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0x9f32ebff, "cdev_init" },
	{ 0x38410bd, "device_destroy" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x50eedeb8, "printk" },
	{ 0xe51aff52, "device_create" },
	{ 0x99cdaa41, "cdev_add" },
	{ 0xa8ec5b05, "module_put" },
	{ 0xd0e796cd, "init_task" },
	{ 0xc58cdb60, "lookup_address" },
	{ 0x37a0cba, "kfree" },
	{ 0xbd87173a, "class_destroy" },
	{ 0x6efbcc7a, "__class_create" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0x4a04eeef, "try_module_get" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "1D34D90DC6F740871C20040");
