
#include <kos.h>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

struct passwd_fake {
  char    pw_name[15];   /* username */
  char    pw_passwd[1];  /* user password */
  uid_t   pw_uid;        /* user ID */
  gid_t   pw_gid;        /* group ID */
  char    pw_gecos[11];      /* user information */
  char    pw_dir[4];        /* home directory */
  char    pw_shell[1];      /* shell program */
} passwd_stub;

uid_t getuid() { return 1000; }
struct passwd* getpwuid(uid_t uid) {
  strncpy(passwd_stub.pw_name, "dreamcast\0", 10);
  strncpy(passwd_stub.pw_passwd, "\0", 1);
  passwd_stub.pw_uid = 1000;
  passwd_stub.pw_gid = 1000;
  strncpy(passwd_stub.pw_gecos, "Dreamcast\0", 10);
  strncpy(passwd_stub.pw_dir, "/ram\0", 4);
  strncpy(passwd_stub.pw_shell, "\0", 1);
  return &passwd_stub;
}
int fsync(int fildes) { 
  // TODO
  return 0; 
}
int access(const char *pathname, int mode) {
  int grant = -1;
  if (mode && X_OK) {
    // No execution permissions on Dreamcast ATM
    return grant;
  }
  if (mode && W_OK) {
    file_t hnd = fs_open(pathname, O_WRONLY);
    if (hnd == -1) {
      return grant;
    } else {
      fs_close(hnd);
    }
  }
  if (mode && R_OK) {
    file_t hnd = fs_open(pathname, O_RDONLY);
    if (hnd == -1) {
      return grant;
    } else {
      fs_close(hnd);
    }
  }
  // Same as R_OK ATM
  if (mode && F_OK) {
    file_t hnd = fs_open(pathname, O_RDONLY);
    if (hnd == -1) {
      return grant;
    } else {
      fs_close(hnd);
    }
  }

  return 0;
}

