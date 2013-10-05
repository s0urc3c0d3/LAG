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
	{ 0xc3a33336, "module_layout" },
	{ 0x2d9cb0e, "rcu_lock_map" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x3229307b, "kmalloc_caches" },
	{ 0x13176be8, "rcu_lockdep_current_cpu_online" },
	{ 0x40ca184b, "__register_chrdev" },
	{ 0x81f00f2c, "__init_waitqueue_head" },
	{ 0x892da873, "debug_lockdep_rcu_enabled" },
	{ 0x27e1a049, "printk" },
	{ 0xb4390f9a, "mcount" },
	{ 0x82d34d88, "module_put" },
	{ 0x809389a7, "init_task" },
	{ 0x217faff3, "kmem_cache_alloc_trace" },
	{ 0x8b9200fd, "lookup_address" },
	{ 0x37a0cba, "kfree" },
	{ 0x36f9cd94, "rcu_is_cpu_idle" },
	{ 0xd7ce27fd, "lock_is_held" },
	{ 0x5a999aba, "try_module_get" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "F38FDE590AFDECE88C029FE");
