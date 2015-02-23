AC_INIT([bblfs], [0.1], [vvu@vdev.ro], [bblfs.tar.gz], [https://github.com/ungureanuvladvictor/BBBlfs])
AC_PREREQ([2.69])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_CONFIG_FILES([
 Makefile
 src/Makefile
])

AC_LANG(C)
AC_PROG_CC
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_FUNC_CHOWN

AC_HEADER_STDC
AC_CHECK_HEADERS([string.h unistd.h errno.h fcntl.h sys/types.h stdio.h stdlib.h])

PKG_CHECK_MODULES([libusb], [libusb-1.0],
	[AC_SUBST([libusb_CFLAGS]) AC_SUBST([libusb_LIBS])],
	[AC_MSG_ERROR(["libusb not found"])])

AC_OUTPUT([])