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
	{ 0x65d7b0bf, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x486a0ab5, __VMLINUX_SYMBOL_STR(nf_unregister_hook) },
	{ 0x5c461c3, __VMLINUX_SYMBOL_STR(nf_register_hook) },
	{ 0x1b6314fd, __VMLINUX_SYMBOL_STR(in_aton) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xb0dc0d47, __VMLINUX_SYMBOL_STR(dev_queue_xmit) },
	{ 0xb1297cc, __VMLINUX_SYMBOL_STR(dev_get_by_name) },
	{ 0x6c9cbd25, __VMLINUX_SYMBOL_STR(init_net) },
	{ 0x958a5047, __VMLINUX_SYMBOL_STR(skb_push) },
	{ 0x1f307718, __VMLINUX_SYMBOL_STR(skb_put) },
	{ 0x498e8fb2, __VMLINUX_SYMBOL_STR(__alloc_skb) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "69EF4F93C8B282F28C32B97");
