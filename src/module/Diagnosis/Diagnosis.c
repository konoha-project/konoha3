/****************************************************************************
 * Copyright (c) 2012-2013, the Konoha project authors. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <konoha3/konoha.h>

#if HAVE_DB_H
#if defined(__linux__)
#include <db_185.h>
#else
#include <db.h>
#endif /*defined(__linux__)*/
#endif /*HAVE_DB_H*/
#include <sys/stat.h>
#include <fcntl.h>

// -------------------------------------------------------------------------
/* Risk */

static kbool_t CheckStaticRisk(KonohaContext *kctx, const char *keyword, size_t keylen, kfileline_t uline)
{
	return true; // OK
}

static void CheckDynamicRisk(KonohaContext *kctx, const char *keyword, size_t keylen, KTraceInfo *trace)
{

}

// -------------------------------------------------------------------------
/* Software Process */

static int DiagnosisSoftwareProcess(KonohaContext *kctx, kfileline_t uline, KTraceInfo *trace)
{
	return 0;
}

// -------------------------------------------------------------------------
/* System Resource */

static int DiagnosisSystemResource(KonohaContext *kctx, KTraceInfo *trace)
{
	return 0;
}

static int DiagnosisFileSystem(KonohaContext *kctx, const char *path, size_t pathlen, KTraceInfo *trace)
{
	return 0; // unknown
}

static int DiagnosisNetworking(KonohaContext *kctx, const char *path, size_t pathlen, int port, KTraceInfo *trace)
{
	return 0; // unknown
}

// -------------------------------------------------------------------------
/* Error Code */

static int DiagnosisSystemError(KonohaContext *kctx, int userFault)
{
	switch(errno) {  // C Standard Library
	case EILSEQ: /* Results from an illegal byte sequence */
		return userFault | SoftwareFault;
	}

	switch(errno) {
	case EPERM:  /* 1. Operation not permitted (Linux) */
		return userFault | SoftwareFault;
	case ENOENT: /* 2. No such file or directory */
		return userFault | SoftwareFault | SystemFault;
	case ESRCH:  /* 3. No such process */
		return userFault | SystemFault | SoftwareFault;
	case EINTR: /* 4. Interrupted system call */
		return SystemFault;
	case EIO: /* 5. I/O error */
		return SystemFault ;
	case ENXIO:  /* 6. No such device or address */
		return userFault | SoftwareFault | SystemFault;
	case E2BIG:  /* 7. Arg list too long */
		return userFault | SoftwareFault | UserFault;
	case ENOEXEC: /* 8. Exec format error */
		return SystemFault;
	case EBADF:  /* 9. Bad file number */
		return SoftwareFault;
	case ECHILD: /* 10. No child processes */
		return userFault | SoftwareFault | SystemFault;
	case EAGAIN: /* 11. Try again */
		return userFault;  /* not fault */
	case ENOMEM: /* 12. Out of memory */
		/* If you try to exec() another process or just ask for more memory in this process */
		return userFault | SoftwareFault | SystemFault;
	case EACCES: /* 13. Permission denied */
		return userFault | SystemFault;  /* running software can be wrong.. */
	case EFAULT: /* 14 Bad address */
		return userFault | SoftwareFault; /* At the C-Level */
	case ENOTBLK: /* 15 Node device required */
		return SystemFault; /* in case of unmount device */
	case EBUSY: /* 16 Device or resource busy */
		return SystemFault;
	case EEXIST: /*17 File exists */
		return userFault | SoftwareFault | SystemFault;
	case EXDEV:  /* 18. Cross-device link */
		return SystemFault;
	case ENODEV: /* 19 No such device */
	case ENOTDIR: /*20 Not a directory */
	case EISDIR: /* 21 Is a directory */
		return userFault | SoftwareFault | SystemFault;
	case EINVAL: /* 22 Invalid argument */
		/* EINVAL gets used a lot.
		 * TCP has the concept of "out of band data" (urgent data).
		 * If a reading process checks for this, and there isn't any, it get EINVAL.
		 * The plock() function ( which locks areas of a process into memory) returns this
		 * if you attempt to use it twice on the same memory segment.
		 * If you try to specify SIGKILL or SIGSTOP to sigaction(), you'll get this return.
		 * The readv() and writev() calls complain this way if you give them too large an array of buffers.
		 * As mentioned above, drivers may return this for inappropriate ioctl() calls.
		 * The mmap() call will return this if you've specified a specific address but that address can't be used.
		 * A seek() to before the beginning of a file returns this.
		 * Streams use this if you attempt to link a stream onto itself. It's used for many IPC errors also. */
		return SoftwareFault;
	case ENFILE:  /* 23. File table overflow */
		return userFault | SoftwareFault | SystemFault;
	case EMFILE: /* 24. Too many open files */
		return userFault | SoftwareFault;
	case ENOTTY: /* 25 Not a typewriter */
		return userFault | SoftwareFault | SystemFault;
	case ETXTBSY:  /* 26 Text file busy */
		/* It's illegal to write to a binary while it is executing- */
		return userFault | SoftwareFault;
	case EFBIG: /* 27 File too large */
		return userFault | SoftwareFault;
	case ENOSPC: /* 28 No space left on device */
		return SoftwareFault | SystemFault;
	case ESPIPE: /* 29 Illegal seek */
		return userFault | SoftwareFault;
	case EROFS:  /* 30 Read-only file system */
		return userFault | SoftwareFault;
	case EMLINK: /* 31 Too many links */
		return userFault | SoftwareFault;
	case EPIPE: /* 32 Broken pipe */
		return userFault | SystemFault | UserFault;
	case EDOM: /* 33 Math argument out of domain of func */
		/*Results from a parameter outside a function's domain, for example sqrt(-1) */
	case ERANGE: /* 34 Math result not representable */
		/* Results from a result outside a function's range, for example strtol("0xfffffffff",NULL,0) */
		return userFault | SoftwareFault;
	case EDEADLK:       /* 35 Resource deadlock would occur */
		return SoftwareFault |SystemFault;
	case ENAMETOOLONG:  /* 36 File name too long */
		return userFault | SoftwareFault;
	case ENOLCK: /* 37 No record locks available UNSURE*/
	case ENOSYS: /* 38 Function not implemented  UNSHRE*/
	case ENOTEMPTY: /* 39 Directory not empty */
		return userFault | SoftwareFault | SystemFault;
	case ELOOP: /* 40 Too many symbolic links encountered */
		return SystemFault;
		/*** ***/
	//case EWOULDBLOCK: /* Operation would block */
	case ENOMSG:   /* No message of desired type */
	case EIDRM:    /* Identifier removed */
	//case ECHRNG:   /* Channel number out of range */
	//case EL2NSYNC: /* Level 2 not synchronized */
	//case EL3HLT:   /* Level 3 halted */
	//case EL3RST:   /* Level 3 reset */
	//case ELNRNG:   /* Link number out of range */
	//case EUNATCH:  /* Protocol driver not attached */
	//case ENOCSI:   /* No CSI structure available */
	//case EL2HLT:   /* Level 2 halted */
	//case EBADE:    /* Invalid exchange */
	//case EBADR:    /* Invalid request descriptor */
	//case EXFULL:   /* Exchange full */
	//case ENOANO:   /* No anode */
	//case EBADRQC:  /* Invalid request code */
	//case EBADSLT:  /* Invalid slot */
	//case EDEADLOCK:
	//case EBFONT:   /* Bad font file format */
	case ENOSTR:   /* Device not a stream */
	case ENODATA:  /* No data available */
	case ETIME:    /* Timer expired */
	case ENOSR:    /* Out of streams resources */
	//case ENONET:   /* Machine is not on the network */
	//case ENOPKG:   /* Package not installed */
	case EREMOTE:  /* Object is remote */
	case ENOLINK:  /* Link has been severed */
	//case EADV:     /* Advertise error */
	//case ESRMNT:   /* Srmount error */
	//case ECOMM:    /* Communication error on send */
	case EPROTO:   /* Protocol error */
	case EMULTIHOP: /* Multihop attempted */
	//case EDOTDOT:   /* RFS specific error */
	case EBADMSG:   /* Not a data message */
	case EOVERFLOW: /* Value too large for defined data type */
	//case ENOTUNIQ: /* Name not unique on network */
	//case EBADFD:  /* File descriptor in bad state */
	//case EREMCHG: /* Remote address changed */
	//case ELIBACC: /* Can not access a needed shared library */
	//case ELIBBAD: /* Accessing a corrupted shared library */
	//case ELIBSCN: /* .lib section in a.out corrupted */
	//case ELIBMAX: /* Attempting to link in too many shared libraries */
	//case ELIBEXEC:/* Cannot exec a shared library directly */
	case EILSEQ:  /* Illegal byte sequence */
	//case ERESTART:/* Interrupted system call should be restarted */
	//case ESTRPIPE:/* Streams pipe error */
	case EUSERS:  /* Too many users */
	case ENOTSOCK: /* Socket operation on non-socket */
	case EDESTADDRREQ: /* Destination address required */
	case EMSGSIZE:    /* Message too long */
	case EPROTOTYPE:  /* Protocol wrong type for socket */
	case ENOPROTOOPT: /* Protocol not available */
	case EPROTONOSUPPORT: /* Protocol not supported */
	case ESOCKTNOSUPPORT: /* Socket type not supported */
	case EOPNOTSUPP:   /* Operation not supported on transport endpoint */
	case EPFNOSUPPORT: /* Protocol family not supported */
	case EAFNOSUPPORT: /* Address family not supported by protocol */
	case EADDRINUSE:   /* Address already in use */
	case EADDRNOTAVAIL: /* Cannot assign requested address */
	case ENETDOWN:    /* Network is down */
	case ENETUNREACH: /* Network is unreachable */
	case ENETRESET:   /* Network dropped connection because of reset */
	case ECONNABORTED: /* Software caused connection abort */
	case ECONNRESET:/* Connection reset by peer */
	case ENOBUFS:   /* No buffer space available */
	case EISCONN:   /* Transport endpoint is already connected */
	case ENOTCONN:  /* Transport endpoint is not connected */
	case ESHUTDOWN: /* Cannot send after transport endpoint shutdown */
	case ETOOMANYREFS: /* Too many references: cannot splice */
	case ETIMEDOUT: /* Connection timed out */
	case ECONNREFUSED: /* Connection refused */
	case EHOSTDOWN: /* Host is down */
	case EHOSTUNREACH: /* No route to host */
	case EALREADY: /* Operation already in progress */
	case EINPROGRESS: /* Operation now in progress */
	case ESTALE:  /* Stale NFS file handle */
	//case EUCLEAN: /* Structure needs cleaning */
	//case ENOTNAM: /* Not a XENIX named type file */
	//case ENAVAIL: /* No XENIX semaphores available */
	//case EISNAM:  /* Is a named type file */
	//case EREMOTEIO: /* Remote I/O error */
	case EDQUOT:  /* Quota exceeded */
	//case ENOMEDIUM: /* No medium found */
	//case EMEDIUMTYPE: /* Wrong medium type */
		break;
		/*** ***/
	}
	return userFault | SoftwareFault |SystemFault;
}

//static kbool_t DiagnosisFetchCoverageLog(KonohaContext *kctx, const char *filename, int line)
//{
//#if HAVE_DB_H
//#define DATABASE "konoha_coverage.db"
//#define BUFSIZE 128
//
//	DB *db = NULL;
//	DBT DBkey = {};
//	DBT DBvalue = {};
//	char key[BUFSIZE];
//
//	if((db = dbopen(DATABASE, O_RDONLY, S_IRGRP | S_IWGRP, DB_BTREE, NULL)) == NULL) {
//		return false;
//	}
//
//	PLATAPI snprintf_i(key, BUFSIZE, "\"%s:%d\"", filename, line);
//
//	DBkey.data = key;
//	DBkey.size = strlen(key);
//
//	if(!db->get(db, &DBkey, &DBvalue, 0)) {
//		PLATAPI syslog_i(5/*LOG_NOTICE*/, "{\"event\": \"DiagnosisErrorCode\", \"ScriptName\": \"%s\", \"ScriptLine:%d, \"Count\":%s, }", filename, line, (char *)DBvalue.data);
//		db->close(db);
//		return true;
//	}
//	else{
//		db->close(db);
//		return false;
//	}
//#else
//	return false;
//#endif
//}

static kbool_t DiagnosisCheckSoftwareTestIsPass(KonohaContext *kctx, const char *filename, int line)
{
	DBG_P("filename='%s', line=%d", filename, line);
	kbool_t res = false;
//#if HAVE_DB_H
//	res = DiagnosisFetchCoverageLog(kctx, filename, line);
//#endif
	return res;
}

// -------------------------------------------------------------------------

kbool_t LoadDiagnosisModule(KonohaFactory *factory, ModuleType type)
{
	static KModuleInfo ModuleInfo = {
		"Diagnosis", "0.1", 0, "deos",
	};
	factory->DiagnosisModule.DiagnosisInfo                    = &ModuleInfo;
	factory->DiagnosisModule.CheckStaticRisk                  = CheckStaticRisk;
	factory->DiagnosisModule.CheckDynamicRisk                 = CheckDynamicRisk;
	factory->DiagnosisModule.DiagnosisSystemError             = DiagnosisSystemError;
	factory->DiagnosisModule.DiagnosisSoftwareProcess         = DiagnosisSoftwareProcess;
	factory->DiagnosisModule.DiagnosisSystemResource          = DiagnosisSystemResource;
	factory->DiagnosisModule.DiagnosisFileSystem              = DiagnosisFileSystem;
	factory->DiagnosisModule.DiagnosisNetworking              = DiagnosisNetworking;
	factory->DiagnosisModule.DiagnosisCheckSoftwareTestIsPass = DiagnosisCheckSoftwareTestIsPass;
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
