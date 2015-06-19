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
	{ 0xa190a8eb, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x3f6c048, __VMLINUX_SYMBOL_STR(nf_unregister_hook) },
	{ 0xab221d7c, __VMLINUX_SYMBOL_STR(sock_release) },
	{ 0xfae5a61d, __VMLINUX_SYMBOL_STR(nf_register_hook) },
	{ 0x1b6314fd, __VMLINUX_SYMBOL_STR(in_aton) },
	{ 0x2566ec1c, __VMLINUX_SYMBOL_STR(sock_create) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x9fae1cf0, __VMLINUX_SYMBOL_STR(skb_copy_bits) },
	{ 0xf0fdf6cb, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0xf5babd18, __VMLINUX_SYMBOL_STR(sock_sendmsg) },
	{ 0x4c4fef19, __VMLINUX_SYMBOL_STR(kernel_stack) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "C52095AA1262E8CC1B58FAE");
