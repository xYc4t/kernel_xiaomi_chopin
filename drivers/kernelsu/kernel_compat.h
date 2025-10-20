#ifndef __KSU_H_KERNEL_COMPAT
#define __KSU_H_KERNEL_COMPAT

#include "linux/fs.h"
#include "linux/key.h"
#include "linux/version.h"
#include "linux/key.h"

#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
// arch/arm64/include/asm/barrier.h, adding dsb probably unneeded
#define DONT_GET_SMART() do { barrier(); isb(); } while (0)
#else
// well, compiler atleast, and not our targets
#define DONT_GET_SMART() barrier()
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0) || defined(CONFIG_KSU_ALLOWLIST_WORKAROUND)
extern struct key *init_session_keyring;
#endif

extern void ksu_android_ns_fs_check();
extern int ksu_access_ok(const void *addr, unsigned long size);
extern struct file *ksu_filp_open_compat(const char *filename, int flags,
					 umode_t mode);
extern ssize_t ksu_kernel_read_compat(struct file *p, void *buf, size_t count,
				      loff_t *pos);
extern ssize_t ksu_kernel_write_compat(struct file *p, const void *buf,
				       size_t count, loff_t *pos);

extern int ksu_access_ok(const void *addr, unsigned long size);
extern long ksu_copy_from_user_nofault(void *dst, const void __user *src, size_t size);

/*
 * ksu_copy_from_user_retry
 * try nofault copy first, if it fails, try with plain
 * paramters are the same as copy_from_user
 * 0 = success
 * + hot since this is reused on sucompat
 */
__attribute__((hot))
static long ksu_copy_from_user_retry(void *to, 
		const void __user *from, unsigned long count)
{
	long ret = ksu_copy_from_user_nofault(to, from, count);
	if (likely(!ret))
		return ret;

	// we faulted! fallback to slow path
	return copy_from_user(to, from, count);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0) && !defined(KSU_HAS_ITERATE_DIR)
struct dir_context {
	const filldir_t actor;
	loff_t pos;
};

static int iterate_dir(struct file *file, struct dir_context *ctx)
{
	return vfs_readdir(file, ctx->actor, ctx);
}
#endif // KSU_HAS_ITERATE_DIR

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
__weak char *bin2hex(char *dst, const void *src, size_t count)
{
	const unsigned char *_src = src;
	while (count--)
		dst = pack_hex_byte(dst, *_src++);
	return dst;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 3, 0)
__weak ssize_t strscpy(char *dest, const char *src, size_t count)
{
    return strlcpy(dest, src, count);
}
#endif

#endif
