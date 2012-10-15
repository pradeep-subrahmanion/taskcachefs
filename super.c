/*
 * taskcachefs super block handling
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
#include <linux/pagemap.h>
#include <linux/version.h>
#include <linux/nls.h>
#include <linux/proc_fs.h>
#include <linux/backing-dev.h>
#include "taskcachefs.h"

#define TASKCACHEFS_MAGIC 0x73616333

extern struct inode_operations taskcachefs_dir_inode_ops;
extern struct inode_operations taskcachefs_file_inode_ops;
extern struct file_operations taskcachefs_file_operations;
extern struct address_space_operations taskcachefs_aops;

static struct kmem_cache *taskcachefs_inode_cachep;

static struct inode *taskcachefs_alloc_inode(struct super_block *sb);

struct inode *taskcachefs_alloc_inode(struct super_block *sb);

static struct dentry *taskcachefs_mount(struct file_system_type *fs_type,
                      int flags, const char *dev_name, void *data);

static struct file_system_type taskcachefs_type = {
	.owner = THIS_MODULE,
        .name = "taskcachefs",
        .mount = taskcachefs_mount,
        .kill_sb = kill_litter_super,
};

struct super_operations taskcachefs_super_ops = {
        .statfs         = simple_statfs,
        .drop_inode     = generic_delete_inode, 
        .alloc_inode    = taskcachefs_alloc_inode,
};


static struct backing_dev_info taskcachefs_backing_dev_info = {
        .ra_pages       = 0,   
        .capabilities   = BDI_CAP_NO_ACCT_DIRTY | BDI_CAP_NO_WRITEBACK |
                          BDI_CAP_MAP_DIRECT | BDI_CAP_MAP_COPY |
                          BDI_CAP_READ_MAP | BDI_CAP_WRITE_MAP |
                          BDI_CAP_EXEC_MAP,
};

struct inode *taskcachefs_alloc_inode(struct super_block *sb) {

        struct taskcachefs_vnode *vnode;
        struct inode *inode;
        vnode = kmem_cache_alloc(taskcachefs_inode_cachep, GFP_KERNEL);
        if(!vnode)
            return NULL;

        memset(vnode,0,sizeof(*vnode));
        vnode->pid = current->pid;
        inode_init_once(&vnode->vfs_inode);

        inode = &vnode->vfs_inode;
        inode->i_ino = get_next_ino();

        return &vnode->vfs_inode;
}

struct inode *taskcachefs_get_inode(struct super_block *sb, int mode, dev_t dev)
{
        struct inode * inode = new_inode(sb);

        if (inode) {
                inode->i_mode = mode;
                inode->i_blocks = 0;
                inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
                printk(KERN_INFO "about to set inode ops\n");
                inode->i_mapping->a_ops = &taskcachefs_aops;
                inode->i_mapping->backing_dev_info = &taskcachefs_backing_dev_info;
                switch (mode & S_IFMT) {
                default:
                        init_special_inode(inode, mode, dev);
                        break;
                case S_IFREG:
                        inode->i_op = &taskcachefs_file_inode_ops;
                        inode->i_fop =  &taskcachefs_file_operations;
                        break;
                case S_IFDIR:
                        inode->i_op = &taskcachefs_dir_inode_ops;
                        inode->i_fop = &simple_dir_operations;
                        inode->i_nlink++;
                        break;
                case S_IFLNK:
                        inode->i_op = &page_symlink_inode_operations;
                        break;
                }
        }

        return inode;
}
static int taskcachefs_fill_super(struct super_block * sb, void * data, int silent)
{
        struct inode * inode;
        
        sb->s_maxbytes = MAX_LFS_FILESIZE;
        sb->s_blocksize = PAGE_CACHE_SIZE;
        sb->s_blocksize_bits = PAGE_CACHE_SHIFT;
        sb->s_magic = TASKCACHEFS_MAGIC;
        sb->s_op = &taskcachefs_super_ops;
        sb->s_time_gran = 1;

        inode = taskcachefs_get_inode(sb, S_IFDIR | 0755, 0);
        if(!inode) {
               
                return -ENOMEM;
        }

        sb->s_root = d_alloc_root(inode);
        if (!sb->s_root) {
                iput(inode);
                return -ENOMEM;
        }

        return 0;
}


static int taskcachefs_test_super(struct super_block *sb, void *data)
{
	return 1;
}

static int taskcachefs_set_super(struct super_block *sb, void *data)
{
	return set_anon_super(sb, NULL);
}
static struct dentry *taskcachefs_mount(struct file_system_type *fs_type,
                      int flags, const char *dev_name, void *data) {

	int ret;
  	struct super_block *sb = sget(fs_type,taskcachefs_test_super, taskcachefs_set_super, NULL);
  	if (IS_ERR(sb)) {
        	ret = PTR_ERR(sb);
       		return ERR_PTR(ret);
  	}
  	if(!sb->s_root) {

    		taskcachefs_fill_super(sb,data,0);
    		sb->s_flags |= MS_ACTIVE;
  	}


  	return dget(sb->s_root);
}
static int __init init_taskcachefs_fs(void)
{

	taskcachefs_inode_cachep = kmem_cache_create("taskcachefs_inode_cache",
                                             sizeof(struct taskcachefs_vnode),
                                             0,
                                             SLAB_HWCACHE_ALIGN,
                                             NULL);
  	if (!taskcachefs_inode_cachep) {
        	printk(KERN_NOTICE "taskcachefs : Failed to allocate inode cache\n");
        	return -1;
     	}


  	return register_filesystem(&taskcachefs_type);

}

static void __exit exit_taskcachefs_fs(void)
{
	unregister_filesystem(&taskcachefs_type);
   	kmem_cache_destroy(taskcachefs_inode_cachep);
}

module_init(init_taskcachefs_fs)
module_exit(exit_taskcachefs_fs)

MODULE_LICENSE("GPL");
