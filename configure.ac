AC_INIT([Qnixx])
AC_CONFIG_SRCDIR([LICENSE])
AC_PREREQ([2.71])


BUILDDIR="$(pwd -P)"
AC_ARG_WITH([ip],
            [AS_HELP_STRING([--with-ip],
              [local ip @<:@default=unknown@:>@])],
            [],
            [with_ip=unknown])

# Build variables.
AC_SUBST(LOCAL_IP, [$with_ip])
AC_SUBST(KERNEL_CFLAGS, ["-fexceptions -std=gnu11 -ffreestanding -fno-stack-protector \
  -fno-pic -Werror=implicit -Werror=implicit-function-declaration -Werror=implicit-int \
  -Werror=int-conversion \
  -Werror=incompatible-pointer-types -Werror=int-to-pointer-cast -Werror=return-type -Wunused \
  -mabi=sysv -mno-80387 -mno-mmx \
  -mno-3dnow -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel"])

AC_PROG_INSTALL
AC_CHECK_FILE([$BUILDDIR/cross],[],AC_MSG_ERROR("Please run 'bash tools/cross.sh'"))
AC_CHECK_PROG(NASM_LOCATED, [nasm], [yes])
AC_CHECK_PROG(XORRISO_LOCATED, [xorriso], [yes])
AC_CHECK_PROG(MAKE_LOCATED, [make], [yes])
AC_CHECK_PROG(GIT_LOCATED, [git], [yes])


if ! test "x$NASM_LOCATED" = "xyes"; then
   AC_MSG_ERROR("Please install nasm")
fi


if ! test "x$XORRISO_LOCATED" = "xyes"; then
   AC_MSG_ERROR("Please install xorriso")
fi


if ! test "x$MAKE_LOCATED" = "xyes"; then
   AC_MSG_ERROR("Please install make")
fi


if ! test "x$GIT_LOCATED" = "xyes"; then
   AC_MSG_ERROR("Please install git")
fi

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
