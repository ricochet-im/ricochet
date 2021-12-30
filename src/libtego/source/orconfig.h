#ifndef ORCONFIG_H
#define ORCONFIG_H

// This header provides needed #defines normally stored in the orconfig.h header
// generatd by the tor build process

/* Define to 1 iff memset(0) sets pointers to NULL */
#define NULL_REP_IS_ZERO_BYTES 1

/* Define to 1 iff memset(0) sets doubles to 0.0 */
#define DOUBLE_0_REP_IS_ZERO_BYTES 1

/* Define to 1 if the system has the type `ssize_t'. */
#define HAVE_SSIZE_T 1

/* Define to 1 iff we represent negative integers with two's complement */
#define USING_TWOS_COMPLEMENT 1

/* The size of `void *', as computed by sizeof. */
#define SIZEOF_VOID_P 8

/* Define to 1 if you have the `EVP_sha3_256' function. */
#define HAVE_EVP_SHA3_256 1

#endif // ORCONFIG_H