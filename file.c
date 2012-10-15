/*
 * taskkcachefs file handling
 *
 * Copyright (C) Pradeep Subrahmanion <subrahmanion.pradeep@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * 
 */
#include <linux/module.h>
#include <linux/fs.h>
#include "taskcachefs.h"
#include <linux/sched.h>

struct taskcachefs_vnode *vnode_from_vfs_inode(struct inode *inode);

int taskcachefs_open(struct inode *inode ,struct file *file)
{

	struct taskcachefs_vnode *vnode = vnode_from_vfs_inode(inode);

  	if(vnode) {
   		int pid = current->pid;
   		struct inode *in = &vnode->vfs_inode;

   		if(vnode->pid == pid)
 			return 0;

   	}

   	return -1;
}


const struct address_space_operations taskcachefs_aops = {
        .readpage       = simple_readpage,
        .write_begin    = simple_write_begin,
        .write_end      = simple_write_end,
};

const struct file_operations taskcachefs_file_operations = {
        .open           = taskcachefs_open,
        .read           = do_sync_read,
        .aio_read       = generic_file_aio_read,
        .write          = do_sync_write,
        .aio_write      = generic_file_aio_write,
        .mmap           = generic_file_mmap,
        .fsync          = noop_fsync,
        .splice_read    = generic_file_splice_read,
        .splice_write   = generic_file_splice_write,
        .llseek         = generic_file_llseek,
};


