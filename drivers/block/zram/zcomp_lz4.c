/*
 * Copyright (C) 2014 Sergey Senozhatsky.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/lz4.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>

#include "zcomp_lz4.h"

static void *zcomp_lz4_create(gfp_t flags)
{
	void *ret;

	ret = kmalloc(LZ4_MEM_COMPRESS, flags);
	if (!ret)
		ret = __vmalloc(LZ4_MEM_COMPRESS,
				flags | __GFP_HIGHMEM,
				PAGE_KERNEL);
	return ret;
}

static void zcomp_lz4_destroy(void *private)
{
	kvfree(private);
}

static int zcomp_lz4_compress(const unsigned char *src, unsigned char *dst,
		size_t *dst_len, void *private)
{
	/*
	 * Our dst memory (zstrm->buffer) is always `2 * PAGE_SIZE' sized
	 * because sometimes we can endup having a bigger compressed data
	 * due to various reasons: for example compression algorithms tend
	 * to add some padding to the compressed buffer. Speaking of padding,
	 * comp algorithm `842' pads the compressed length to multiple of 8
	 * and returns -ENOSP when the dst memory is not big enough, which
	 * is not something that ZRAM wants to see. We can handle the
	 * `compressed_size > PAGE_SIZE' case easily in ZRAM, but when we
	 * receive -ERRNO from the compressing backend we can't help it
	 * anymore. To make `842' happy we need to tell the exact size of
	 * the dst buffer, zram_drv will take care of the fact that
	 * compressed buffer is too big.
	 */
	*dst_len = PAGE_SIZE * 2;

	*dst_len = LZ4_compress_default(src, dst, PAGE_SIZE, *dst_len, private);

	if (!*dst_len)
		return -1;

	return 0;
}

static int zcomp_lz4_decompress(const unsigned char *src, size_t src_len,
		unsigned char *dst)
{
	size_t dst_len = PAGE_SIZE;
	dst_len = LZ4_decompress_safe(src, dst, src_len, dst_len);

	if (dst_len > 0)
		return 0;

	return -1;
}

struct zcomp_backend zcomp_lz4 = {
	.compress = zcomp_lz4_compress,
	.decompress = zcomp_lz4_decompress,
	.create = zcomp_lz4_create,
	.destroy = zcomp_lz4_destroy,
	.name = "lz4",
};
