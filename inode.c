/*
 * taskcachefs inode handling
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

extern struct dentry_operations taskcachefs_dentry_ops;
extern struct dentry_operations taskcache_dentry_ops;
extern struct inode *taskcachefs_get_inode(struct super_block *sb, int mode, 
					dev_t dev);

struct inode *inode_from_taskcachefs_vnode(struct taskcachefs_vnode *vnode)
{
	return &vnode->vfs_inode;
}

struct taskcachefs_vnode *vnode_from_vfs_inode(struct inode *inode)
{
	return container_of(inode,struct taskcachefs_vnode,vfs_inode);
}

static struct dentry *taskcachefs_lookup(struct inode *dir, struct dentry *dentry, 
				struct nameidata *nd)
{

	d_add(dentry, NULL);
	return NULL;
}

static int
taskcachefs_mknod(struct inode *dir, struct dentry *dentry, int mode, dev_t dev)
{
        struct inode * inode = taskcachefs_get_inode(dir->i_sb, mode, dev);
        int error = -ENOSPC;
	
	printk(KERN_INFO "samplefs: mknod\n");
        if (inode) {
                if (dir->i_mode & S_ISGID) {
                	inode->i_gid = dir->i_gid;
                        if (S_ISDIR(mode))
                                inode->i_mode |= S_ISGID;
                }
                d_instantiate(dentry, inode);
                dget(dentry);  
                error = 0;
                dir->i_mtime = dir->i_ctime = CURRENT_TIME;

		dir->i_size += 0x20;
        }

        return error;
}


static int taskcachefs_mkdir(struct inode * dir, struct dentry * dentry, int mode)
{
        int retval = 0;
	retval = taskcachefs_mknod(dir, dentry, mode | S_IFDIR, 0);
        if (!retval)
                dir->i_nlink++;

        return retval;
}

static int taskcachefs_create(struct inode *dir, struct dentry *dentry, int mode, 
			struct nameidata *nd)
{
        return taskcachefs_mknod(dir, dentry, mode | S_IFREG, 0);
}

static int taskcachefs_symlink(struct inode * dir, struct dentry *dentry, 
			const char * symname)
{
	struct inode *inode;
	int error = -ENOSPC;

	inode = taskcachefs_get_inode(dir->i_sb, S_IFLNK|S_IRWXUGO, 0);
	if (inode) {
		int l = strlen(symname)+1;
		error = page_symlink(inode, symname, l);
		if (!error) {
			if (dir->i_mode & S_ISGID)
				inode->i_gid = dir->i_gid;
			d_instantiate(dentry, inode);
			dget(dentry);
			dir->i_mtime = dir->i_ctime = CURRENT_TIME;
		} else
			iput(inode);
	}
	return error;
}

struct inode_operations taskcachefs_file_inode_ops = {
        .getattr        = simple_getattr,
};

struct inode_operations taskcachefs_dir_inode_ops = {
	.create         = taskcachefs_create,
	.lookup         = taskcachefs_lookup,
	.link		= simple_link,
	.unlink         = simple_unlink,
	.symlink	= taskcachefs_symlink,
	.mkdir          = taskcachefs_mkdir,
	.rmdir          = simple_rmdir,
	.mknod          = taskcachefs_mknod,
	.rename         = simple_rename,
};


