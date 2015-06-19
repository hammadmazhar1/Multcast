#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
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
	{ 0x59caa4c3, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x1468380c, __VMLINUX_SYMBOL_STR(nf_unregister_hook) },
	{ 0x9ab90172, __VMLINUX_SYMBOL_STR(nf_register_hook) },
	{ 0x1b6314fd, __VMLINUX_SYMBOL_STR(in_aton) },
	{ 0x49f3314f, __VMLINUX_SYMBOL_STR(dev_queue_xmit) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x89ff38df, __VMLINUX_SYMBOL_STR(dev_get_by_name) },
	{ 0x93ac14dc, __VMLINUX_SYMBOL_STR(init_net) },
	{ 0xe113bbbc, __VMLINUX_SYMBOL_STR(csum_partial) },
	{ 0x3920d96e, __VMLINUX_SYMBOL_STR(skb_push) },
	{ 0xf5739cfd, __VMLINUX_SYMBOL_STR(skb_put) },
	{ 0xe4e23f6d, __VMLINUX_SYMBOL_STR(__alloc_skb) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "DD41C3F6DBB7ACABE864008");
