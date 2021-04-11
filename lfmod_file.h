#ifndef LFMOD_FILE_H_
#define LFMOD_FILE_H_

int lfmod_open(struct inode *inode, struct file *fp);
int lfmod_release(struct inode *inode, struct file *fp);

#endif // LFMOD_FILE_H_
