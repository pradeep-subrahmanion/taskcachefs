/*
 * taskkcachefs common types
 *
 * Copyright (C) Pradeep Subrahmanion <subrahmanion.pradeep@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * 
 *
 */
struct taskcachefs_vnode {
	struct inode vfs_inode;
	pid_t pid;
};


