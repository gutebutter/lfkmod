/**
 *  lfmod_public.h
 *
 * public driver definitions to be used by userspace
 * api and/or applications.
 */
#ifndef LFMOD_PUBLIC_H_
#define LFMOD_PUBLIC_H_

#include <linux/ioctl.h>

#define LFMOD_DEVDEVICE "litefury"

// communication types
struct lfmod_register_io {
    uint32_t offset;
    uint32_t data;
    uint32_t mask;
};

// ioctl numbers
#define LFMOD_MAGIC 'l'

#define LFMOD_REGISTER_READ  _IOWR(LFMOD_MAGIC, 0, struct lfmod_register_io)
#define LFMOD_REGISTER_WRITE _IOWR(LFMOD_MAGIC, 1, struct lfmod_register_io)
#define LFMOD_REGISTER_RMW   _IOWR(LFMOD_MAGIC, 2, struct lfmod_register_io)

#endif // LFMOD_PUBLIC_H_
