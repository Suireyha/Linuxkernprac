#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xe486c4b7, "device_create" },
	{ 0x1595e410, "device_destroy" },
	{ 0xa1dacb42, "class_destroy" },
	{ 0x0bc5fb0d, "unregister_chrdev_region" },
	{ 0x9f222e1e, "alloc_chrdev_region" },
	{ 0xd5f66efd, "cdev_init" },
	{ 0x8ea73856, "cdev_add" },
	{ 0x4e54d6ac, "cdev_del" },
	{ 0xcb8b6ec6, "kfree" },
	{ 0x0040afbe, "param_ops_int" },
	{ 0xd272d446, "__fentry__" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0xe8213e80, "_printk" },
	{ 0xd710adbf, "__kmalloc_noprof" },
	{ 0xc5bd6261, "register_chrdev_region" },
	{ 0x653aa194, "class_create" },
	{ 0xbebe66ff, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xe486c4b7,
	0x1595e410,
	0xa1dacb42,
	0x0bc5fb0d,
	0x9f222e1e,
	0xd5f66efd,
	0x8ea73856,
	0x4e54d6ac,
	0xcb8b6ec6,
	0x0040afbe,
	0xd272d446,
	0xd272d446,
	0xe8213e80,
	0xd710adbf,
	0xc5bd6261,
	0x653aa194,
	0xbebe66ff,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"device_create\0"
	"device_destroy\0"
	"class_destroy\0"
	"unregister_chrdev_region\0"
	"alloc_chrdev_region\0"
	"cdev_init\0"
	"cdev_add\0"
	"cdev_del\0"
	"kfree\0"
	"param_ops_int\0"
	"__fentry__\0"
	"__x86_return_thunk\0"
	"_printk\0"
	"__kmalloc_noprof\0"
	"register_chrdev_region\0"
	"class_create\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "3FEBB76DD45409F8828F445");
