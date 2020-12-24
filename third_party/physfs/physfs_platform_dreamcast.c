/*
 * Bare-bones Dreamcast support routines for PhysicsFS.
 *
 *  This file written by Gustavo Aranda.
 */

/* !!! FIXME: check for EINTR? */

#define __PHYSICSFS_INTERNAL__
#include "physfs_platforms.h"

#ifdef PHYSFS_PLATFORM_DREAMCAST

#include <kos.h>

#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
//#include <pthread.h>

#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>
#include <time.h>
#include <limits.h>

#include <alloca.h>

#include "physfs_internal.h"


static PHYSFS_ErrorCode errcodeFromErrnoError(const int err)
{
    switch (err)
    {
        case 0: return PHYSFS_ERR_OK;
        case EACCES: return PHYSFS_ERR_PERMISSION;
        case EPERM: return PHYSFS_ERR_PERMISSION;
        case EDQUOT: return PHYSFS_ERR_NO_SPACE;
        case EIO: return PHYSFS_ERR_IO;
        case ELOOP: return PHYSFS_ERR_SYMLINK_LOOP;
        case EMLINK: return PHYSFS_ERR_NO_SPACE;
        case ENAMETOOLONG: return PHYSFS_ERR_BAD_FILENAME;
        case ENOENT: return PHYSFS_ERR_NOT_FOUND;
        case ENOSPC: return PHYSFS_ERR_NO_SPACE;
        case ENOTDIR: return PHYSFS_ERR_NOT_FOUND;
        case EISDIR: return PHYSFS_ERR_NOT_A_FILE;
        case EROFS: return PHYSFS_ERR_READ_ONLY;
        case ETXTBSY: return PHYSFS_ERR_BUSY;
        case EBUSY: return PHYSFS_ERR_BUSY;
        case ENOMEM: return PHYSFS_ERR_OUT_OF_MEMORY;
        case ENOTEMPTY: return PHYSFS_ERR_DIR_NOT_EMPTY;
        default: return PHYSFS_ERR_OS_ERROR;
    } /* switch */
} /* errcodeFromErrnoError */


static inline PHYSFS_ErrorCode errcodeFromErrno(void)
{
    return errcodeFromErrnoError(errno);
} /* errcodeFromErrno */


static char *getUserDirByUID(void)
{
    return "/ram/";
} /* getUserDirByUID */


char *__PHYSFS_platformCalcUserDir(void)
{
    return "/ram/";
} /* __PHYSFS_platformCalcUserDir */


PHYSFS_EnumerateCallbackResult __PHYSFS_platformEnumerate(const char *dirname,
                               PHYSFS_EnumerateCallback callback,
                               const char *origdir, void *callbackdata)
{
    // GUSARBA: No idea if this works
    DIR *dir;
    struct dirent *ent;
    PHYSFS_EnumerateCallbackResult retval = PHYSFS_ENUM_OK;

    dir = opendir(dirname);
    BAIL_IF(dir == NULL, errcodeFromErrno(), PHYSFS_ENUM_ERROR);

    while ((retval == PHYSFS_ENUM_OK) && ((ent = readdir(dir)) != NULL))
    {
        const char *name = ent->d_name;
        if (name[0] == '.')  /* ignore "." and ".." */
        {
            if ((name[1] == '\0') || ((name[1] == '.') && (name[2] == '\0')))
                continue;
        } /* if */

        retval = callback(callbackdata, origdir, name);
        if (retval == PHYSFS_ENUM_ERROR)
            PHYSFS_setErrorCode(PHYSFS_ERR_APP_CALLBACK);
    } /* while */

    closedir(dir);

    return retval;
} /* __PHYSFS_platformEnumerate */


int __PHYSFS_platformMkDir(const char *path)
{
    const int rc = fs_mkdir(path);
    BAIL_IF(rc == -1, errcodeFromErrno(), 0);
    return 1;
} /* __PHYSFS_platformMkDir */


static void *doOpen(const char *filename, int mode)
{
    const int appending = (mode & O_APPEND);
    int fd;
    int *retval;
    errno = 0;

    //printf("doOpen %s %x\n", filename, mode);

    /* O_APPEND doesn't actually behave as we'd like. */
    mode &= ~O_APPEND;

    fd = fs_open(filename, mode);
    BAIL_IF(fd < 0, errcodeFromErrno(), NULL);

    if (appending)
    {
        if (fs_seek(fd, 0, SEEK_END) < 0)
        {
            const int err = errno;
            fs_close(fd);
            BAIL(errcodeFromErrnoError(err), NULL);
        } /* if */
    } /* if */

    retval = (int *) allocator.Malloc(sizeof (int));
    if (!retval)
    {
        fs_close(fd);
        BAIL(PHYSFS_ERR_OUT_OF_MEMORY, NULL);
    } /* if */

    *retval = fd;
    return ((void *) retval);
} /* doOpen */


void *__PHYSFS_platformOpenRead(const char *filename)
{
    return doOpen(filename, O_RDONLY);
} /* __PHYSFS_platformOpenRead */


void *__PHYSFS_platformOpenWrite(const char *filename)
{
    return doOpen(filename, O_WRONLY | O_CREAT | O_TRUNC);
} /* __PHYSFS_platformOpenWrite */


void *__PHYSFS_platformOpenAppend(const char *filename)
{
    return doOpen(filename, O_WRONLY | O_CREAT | O_APPEND);
} /* __PHYSFS_platformOpenAppend */


PHYSFS_sint64 __PHYSFS_platformRead(void *opaque, void *buffer,
                                    PHYSFS_uint64 len)
{
    const int fd = *((int *) opaque);
    ssize_t rc = 0;

    if (!__PHYSFS_ui64FitsAddressSpace(len))
        BAIL(PHYSFS_ERR_INVALID_ARGUMENT, -1);

    rc = fs_read(fd, buffer, (size_t) len);
    BAIL_IF(rc == -1, errcodeFromErrno(), -1);
    assert(rc >= 0);
    assert(rc <= len);
    return (PHYSFS_sint64) rc;
} /* __PHYSFS_platformRead */


PHYSFS_sint64 __PHYSFS_platformWrite(void *opaque, const void *buffer,
                                     PHYSFS_uint64 len)
{
    const int fd = *((int *) opaque);
    ssize_t rc = 0;

    if (!__PHYSFS_ui64FitsAddressSpace(len))
        BAIL(PHYSFS_ERR_INVALID_ARGUMENT, -1);

    rc = fs_write(fd, (void *) buffer, (size_t) len);
    BAIL_IF(rc == -1, errcodeFromErrno(), rc);
    assert(rc >= 0);
    assert(rc <= len);
    return (PHYSFS_sint64) rc;
} /* __PHYSFS_platformWrite */


int __PHYSFS_platformSeek(void *opaque, PHYSFS_uint64 pos)
{
    const int fd = *((int *) opaque);
    const off_t rc = fs_seek(fd, (off_t) pos, SEEK_SET);
    BAIL_IF(rc == -1, errcodeFromErrno(), 0);
    return 1;
} /* __PHYSFS_platformSeek */


PHYSFS_sint64 __PHYSFS_platformTell(void *opaque)
{
    const int fd = *((int *) opaque);
    PHYSFS_sint64 retval;
    retval = (PHYSFS_sint64) fs_tell64(fd);
    BAIL_IF(retval == -1, errcodeFromErrno(), -1);
    return retval;
} /* __PHYSFS_platformTell */


PHYSFS_sint64 __PHYSFS_platformFileLength(void *opaque)
{
    const int fd = *((int *) opaque);
    PHYSFS_sint64 retval;
    retval = (PHYSFS_sint64) fs_total64(fd);
    BAIL_IF(retval == -1, errcodeFromErrno(), -1);
    return retval;
} /* __PHYSFS_platformFileLength */


int __PHYSFS_platformFlush(void *opaque)
{
    // TODO: Maybe with fs_complete() ?
    return 1;
} /* __PHYSFS_platformFlush */


void __PHYSFS_platformClose(void *opaque)
{
    const int fd = *((int *) opaque);
    (void) fs_close(fd);  /* we don't check this. You should have used flush! */
    allocator.Free(opaque);
} /* __PHYSFS_platformClose */


int __PHYSFS_platformDelete(const char *path)
{
    BAIL_IF(fs_unlink(path) == -1, errcodeFromErrno(), 0);
    return 1;
} /* __PHYSFS_platformDelete */


int __PHYSFS_platformStat(const char *fname, PHYSFS_Stat *st, const int follow)
{
    struct stat statbuf;
    DIR *d;

    // Most Dreamcast filesystems do not support symlinking,
    // so we enforce NOFOLLOW and ignore the follow parameter
    int flag = AT_SYMLINK_NOFOLLOW;

    int rc = fs_stat(fname, &statbuf, flag);
    if (rc == -1) {
      // If it's a vfs like iso9660, stat is not implemented, but fstat is.
      // So, let's try to open a file descriptor and fstat it.
      // First, try as a normal file with O_RDONLY
      file_t fd = fs_open(fname, O_RDONLY);
      if (fd == -1) {
        // Could not open the file descriptor
        // Second, try to open it as a directory with O_DIR
        fd = fs_open(fname, O_DIR);
        if (fd == -1) {
            // Could not open it neither as a normal file nor as a directory
            return 0;
        } else {
          rc = fs_fstat(fd, &statbuf);
          if (rc == -1) {
            // fs_fstat failed but it has a valid handle.
            // It's probably a dcload path special case
            if ((fname[0] == '/') && (fname[1] == 'p') && (fname[2] == 'c')) {
              rc = 0;
              st->filetype = PHYSFS_FILETYPE_DIRECTORY;
              st->filesize = 0;
              statbuf.st_mode = S_IFDIR;
            }
          }
          fs_close(fd);
        }
      } else {
        rc = fs_fstat(fd, &statbuf);
        fs_close(fd);
      }
    }
    BAIL_IF(rc == -1, errcodeFromErrno(), 0);
    
    if (S_ISREG(statbuf.st_mode))
    {
        st->filetype = PHYSFS_FILETYPE_REGULAR;
        st->filesize = statbuf.st_size;
    } /* if */

    else if(S_ISDIR(statbuf.st_mode))
    {
        st->filetype = PHYSFS_FILETYPE_DIRECTORY;
        st->filesize = 0;
    } /* else if */

    else if(S_ISLNK(statbuf.st_mode))
    {
        st->filetype = PHYSFS_FILETYPE_SYMLINK;
        st->filesize = 0;
    } /* else if */

    else
    {
        st->filetype = PHYSFS_FILETYPE_OTHER;
        st->filesize = statbuf.st_size;
    } /* else */

    st->modtime = statbuf.st_mtime;
    st->createtime = statbuf.st_ctime;
    st->accesstime = statbuf.st_atime;

    st->readonly = (access(fname, W_OK) == -1);
    return 1;
} /* __PHYSFS_platformStat */


typedef struct
{
    mutex_t       mutex;
    tid_t*        owner;
    PHYSFS_uint32 count;
} PthreadMutex;


void *__PHYSFS_platformGetThreadID(void)
{
    return ( (void *) ((tid_t) thd_get_current()->tid) );
} /* __PHYSFS_platformGetThreadID */


void *__PHYSFS_platformCreateMutex(void)
{
    int rc;
    PthreadMutex *m = (PthreadMutex *) allocator.Malloc(sizeof (PthreadMutex));
    BAIL_IF(!m, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
    rc = mutex_init(&m->mutex, MUTEX_TYPE_NORMAL);
    if (rc != 0)
    {
        allocator.Free(m);
        BAIL(PHYSFS_ERR_OS_ERROR, NULL);
    } /* if */

    m->count = 0;
    m->owner = (tid_t) 0xDEADBEEF;
    return ((void *) m);
} /* __PHYSFS_platformCreateMutex */


void __PHYSFS_platformDestroyMutex(void *mutex)
{
    PthreadMutex *m = (PthreadMutex *) mutex;

    /* Destroying a locked mutex is a bug, but we'll try to be helpful. */
    if ((m->owner == thd_get_current()->tid) && (m->count > 0))
        mutex_unlock(&m->mutex);

    mutex_destroy(&m->mutex);
    allocator.Free(m);
} /* __PHYSFS_platformDestroyMutex */


int __PHYSFS_platformGrabMutex(void *mutex)
{
    PthreadMutex *m = (PthreadMutex *) mutex;
    tid_t tid = thd_get_current()->tid;
    if (m->owner != tid)
    {
        if (mutex_lock(&m->mutex) != 0)
            return 0;
        m->owner = tid;
    } /* if */

    m->count++;
    return 1;
} /* __PHYSFS_platformGrabMutex */


void __PHYSFS_platformReleaseMutex(void *mutex)
{
    PthreadMutex *m = (PthreadMutex *) mutex;
    assert(m->owner == thd_get_current()->tid);  /* catch programming errors. */
    assert(m->count > 0);  /* catch programming errors. */
    if (m->owner == thd_get_current()->tid)
    {
        if (--m->count == 0)
        {
            m->owner = (tid_t) 0xDEADBEEF;
            mutex_unlock(&m->mutex);
        } /* if */
    } /* if */
} /* __PHYSFS_platformReleaseMutex */



// UNIX-like part of things

int __PHYSFS_platformInit(void)
{
    return 1;  /* always succeed. */
} /* __PHYSFS_platformInit */


void __PHYSFS_platformDeinit(void)
{
    /* no-op */
} /* __PHYSFS_platformDeinit */


void __PHYSFS_platformDetectAvailableCDs(PHYSFS_StringCallback cb, void *data)
{
#if (defined PHYSFS_NO_CDROM_SUPPORT)
    /* no-op. */
#else
    cb(data, "/cd/");
#endif
} /* __PHYSFS_platformDetectAvailableCDs */

/*
 * See where program (bin) resides in the $PATH specified by (envr).
 *  returns a copy of the first element in envr that contains it, or NULL
 *  if it doesn't exist or there were other problems. PHYSFS_SetError() is
 *  called if we have a problem.
 *
 * (envr) will be scribbled over, and you are expected to allocator.Free() the
 *  return value when you're done with it.
 */
static char *findBinaryInPath(const char *bin, char *envr)
{
    return "/";
} /* findBinaryInPath */

static char *readSymLink(const char *path)
{
    ssize_t len = 64;
    ssize_t rc = -1;
    char *retval = NULL;

    while (1)
    {
         char *ptr = (char *) allocator.Realloc(retval, (size_t) len);
         if (ptr == NULL)
             break;   /* out of memory. */
         retval = ptr;

         rc = fs_readlink(path, retval, len);
         if (rc == -1)
             break;  /* not a symlink, i/o error, etc. */

         else if (rc < len)
         {
             retval[rc] = '\0';  /* readlink doesn't null-terminate. */
             return retval;  /* we're good to go. */
         } /* else if */

         len *= 2;  /* grow buffer, try again. */
    } /* while */

    if (retval != NULL)
        allocator.Free(retval);
    return NULL;
} /* readSymLink */


char *__PHYSFS_platformCalcBaseDir(const char *argv0)
{
    // TODO: Work with /rd and /pc too for debugging
    char *retval = NULL;
    const char *envr = NULL;

    /* Try to avoid using argv0 unless forced to. Try system-specific stuff. */

    char fullpath[PATH_MAX];
    //strcpy(fullpath, "/cd/\0");
    strcpy(fullpath, "/\0");
    retval = fullpath;

    return retval;
} /* __PHYSFS_platformCalcBaseDir */


char *__PHYSFS_platformCalcPrefDir(const char *org, const char *app)
{
    // TODO: Work with VMU/SD files
    char *retval = NULL;
    
    char fullpath[PATH_MAX];
    strcpy(fullpath, "/ram/\0");
    retval = fullpath;

    return retval;
} /* __PHYSFS_platformCalcPrefDir */


#endif  /* PHYSFS_PLATFORM_DREAMCAST */

/* end of physfs_platform_dreamcast.c ... */

