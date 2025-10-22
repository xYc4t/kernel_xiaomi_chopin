#ifndef __KSU_H_KERNEL_COMPAT
#define __KSU_H_KERNEL_COMPAT

#include <linux/fs.h>
#include <linux/version.h>
#include <linux/cred.h>
#include "ss/policydb.h"
#include "linux/key.h"

#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
#define kcompat_barrier() do { barrier(); isb(); } while (0)
#else
#define kcompat_barrier() barrier()
#endif

/*
 * Linux 6.8+ does not have LKM support, due to numerous changes on LSM.
 * Let's fails if MODULE were defined.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 8, 0) && defined(MODULE) 
#error "LKM mode is not supported on Linux 6.8+, aborting build."
#endif

/**
 * list_count_nodes - count the number of nodes in a list
 * @head: the head of the list
 *
 * This function iterates over the list starting from @head and counts
 * the number of nodes in the list. It does not modify the list.
 *
 * Context: Any context. The function is safe to call in any context,
 *          including interrupt context, as it does not sleep or allocate
 *          memory.
 *
 * Return: the number of nodes in the list (excluding the head)
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 6, 0)
static inline __maybe_unused size_t list_count_nodes(const struct list_head *head)
{
    const struct list_head *pos;
    size_t count = 0;

    if (!head)
        return 0;

    list_for_each(pos, head)
        count++;

	return count;
}
#endif

/*
 * Adapt to Huawei HISI kernel without affecting other kernels ,
 * Huawei Hisi Kernel EBITMAP Enable or Disable Flag ,
 * From ss/ebitmap.h
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)) &&             \
		(LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0)) || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)) &&        \
		(LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0))
#ifdef HISI_SELINUX_EBITMAP_RO
#define CONFIG_IS_HW_HISI
#endif
#endif

// Checks for UH, KDP and RKP
#ifdef SAMSUNG_UH_DRIVER_EXIST
#if defined(CONFIG_UH) || defined(CONFIG_KDP) || defined(CONFIG_RKP)
#error "CONFIG_UH, CONFIG_KDP and CONFIG_RKP is enabled! Please disable or remove it before compile a kernel with KernelSU!"
#endif
#endif

extern long ksu_strncpy_from_user_nofault(char *dst,
					  const void __user *unsafe_addr,
					  long count);
extern long ksu_strncpy_from_user_retry(char *dst,
					const void __user *unsafe_addr,
					long count);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0) || \
	defined(CONFIG_IS_HW_HISI) || defined(CONFIG_KSU_ALLOWLIST_WORKAROUND)
extern struct key *init_session_keyring;
#endif

extern void ksu_android_ns_fs_check(void);
extern struct file *ksu_filp_open_compat(const char *filename, int flags,
					 umode_t mode);
extern ssize_t ksu_kernel_read_compat(struct file *p, void *buf, size_t count,
				      loff_t *pos);
extern ssize_t ksu_kernel_write_compat(struct file *p, const void *buf,
				       size_t count, loff_t *pos);
extern long ksu_copy_from_user_nofault(void *dst, const void __user *src, size_t size);
/*
 * ksu_copy_from_user_retry
 * try nofault copy first, if it fails, try with plain
 * paramters are the same as copy_from_user
 * 0 = success
 */
static long ksu_copy_from_user_retry(void *to, 
		const void __user *from, unsigned long count)
{
	long ret = ksu_copy_from_user_nofault(to, from, count);
	if (likely(!ret))
		return ret;

	// we faulted! fallback to slow path
	return copy_from_user(to, from, count);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
#define ksu_access_ok(addr, size) access_ok(addr, size)
#else
#define ksu_access_ok(addr, size) access_ok(VERIFY_READ, addr, size)
#endif

#endif
