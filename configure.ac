AC_INIT([Qnixx])
AC_CONFIG_SRCDIR([LICENSE])
AC_PREREQ([2.71])

K_CC="x86_64-elf-gcc"
K_LD="x86_64-elf-ld"


BUILDDIR="$(pwd -P)"
AC_ARG_WITH([ip],
            [AS_HELP_STRING([--with-ip],
              [local ip @<:@default=unknown@:>@])],
            [],
            [with_ip=unknown])

# Build variables.
AC_SUBST(KERNEL_CC, [$K_CC])
AC_SUBST(KERNEL_LD, [$K_LD])
AC_SUBST(LOCAL_IP, [$with_ip])

AC_ARG_VAR(KERNEL_CFLAGS, [C Compiler Flags [-fexceptions -std=gnu11 -ffreestanding -fno-stack-protector \
  -fno-pic -Werror=implicit -Werror=implicit-function-declaration -Werror=implicit-int \
  -Werror=int-conversion \
  -Werror=incompatible-pointer-types -Werror=int-to-pointer-cast -Werror=return-type -Wunused \
  -mabi=sysv -mno-80387 -mno-mmx \
  -mno-3dnow -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel]])

AC_PROG_INSTALL
AC_CHECK_PROG(CC_LOCATED, $K_CC, [yes])
AC_CHECK_PROG(LD_LOCATED, $K_LD, [yes])
AC_CHECK_PROG(NASM_LOCATED, [nasm], [yes])

if ! test "x$CC_LOCATED" = "xyes"; then
  bash tools/cross.sh
fi

if ! test "x$LD_LOCATED" = "xyes"; then
  bash tools/cross.sh
fi

if ! test "x$NASM_LOCATED" = "xyes"; then
   AC_MSG_ERROR("Please install nasm")
fi

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
