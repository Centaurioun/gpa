#!/bin/bash
# Run this to generate all the initial makefiles, etc.
#
# Use --build-w32 to prepare the cross compiling build for Windoze
#

configure_ac="configure.ac"

cvtver () {
  awk 'NR==1 {split($NF,A,".");X=1000000*A[1]+1000*A[2]+A[3];print X;exit 0}'
}

check_version () {
    if [ `("$1" --version || echo "0") | cvtver` -ge "$2" ]; then
       return 0
    fi
    echo "**Error**: "\`$1\'" not installed or too old." >&2
    echo '           Version '$3' or newer is required.' >&2
    [ -n "$4" ] && echo '           Note that this is part of '\`$4\''.' >&2
    DIE="yes"
    return 1
}

# Allow to override the default tool names
AUTOCONF=${AUTOCONF_PREFIX}${AUTOCONF:-autoconf}${AUTOCONF_SUFFIX}
AUTOHEADER=${AUTOCONF_PREFIX}${AUTOHEADER:-autoheader}${AUTOCONF_SUFFIX}

AUTOMAKE=${AUTOMAKE_PREFIX}${AUTOMAKE:-automake}${AUTOMAKE_SUFFIX}
ACLOCAL=${AUTOMAKE_PREFIX}${ACLOCAL:-aclocal}${AUTOMAKE_SUFFIX}

GETTEXT=${GETTEXT_PREFIX}${GETTEXT:-gettext}${GETTEXT_SUFFIX}
MSGMERGE=${GETTEXT_PREFIX}${MSGMERGE:-msgmerge}${GETTEXT_SUFFIX}

DIE=no
FORCE=
if test "$1" == "--force"; then
  FORCE=" --force"
  shift
fi


# ***** W32 build script *******
# Used to cross-compile for Windows.
if test "$1" = "--build-w32"; then
    tmp=`dirname $0`
    tsdir=`cd "$tmp"; pwd`
    shift
    if [ ! -f $tsdir/config.guess ]; then
        echo "$tsdir/config.guess not found" >&2
        exit 1
    fi
    build=`$tsdir/config.guess`

    [ -z "$w32root" ] && w32root="$HOME/w32root"
    echo "Using $w32root as standard install directory" >&2

    # Locate the cross compiler
    crossbindir=
    for host in i586-mingw32msvc i386-mingw32msvc mingw32; do
        if ${host}-gcc --version >/dev/null 2>&1 ; then
            crossbindir=/usr/${host}/bin
            conf_CC="CC=${host}-gcc"
            break;
        fi
    done
    if [ -z "$crossbindir" ]; then
        echo "Cross compiler kit not installed" >&2
        echo "Under Debian GNU/Linux, you may install it using" >&2
        echo "  apt-get install mingw32 mingw32-runtime mingw32-binutils" >&2
        echo "Stop." >&2
        exit 1
    fi

    if [ -f "$tsdir/config.log" ]; then
        if ! head $tsdir/config.log | grep "$host" >/dev/null; then
            echo "Please run a 'make distclean' first" >&2
            exit 1
        fi
    fi

    $tsdir/configure ${conf_CC} --build=${build} --host=${host} \
            --prefix=${w32root} \
            --with-zlib=${w32root} \
            --with-gpg-error-prefix=${w32root} \
	    --with-gpgme-prefix=${w32root} \
            --with-lib-prefix=${w32root} \
            --with-libiconv-prefix=${w32root} \
            PKG_CONFIG="$w32root/bin/pkg-config" \
            PKG_CONFIG_LIBDIR="$w32root/lib/pkgconfig" "$@"
    rc=$?
    exit $rc
fi
# ***** end W32 build script *******


# Grep the required versions from configure.ac
autoconf_vers=`sed -n '/^AC_PREREQ(/ {
s/^.*(\(.*\))/\1/p
q
}' ${configure_ac}`
autoconf_vers_num=`echo "$autoconf_vers" | cvtver`

automake_vers=`sed -n '/^min_automake_version=/ {
s/^.*="\(.*\)"/\1/p
q
}' ${configure_ac}`
automake_vers_num=`echo "$automake_vers" | cvtver`

gettext_vers=`sed -n '/^AM_GNU_GETTEXT_VERSION(/ {
s/^.*(\(.*\))/\1/p
q
}' ${configure_ac}`
gettext_vers_num=`echo "$gettext_vers" | cvtver`


if [ -z "$autoconf_vers" -o -z "$automake_vers" -o -z "$gettext_vers" ]
then
  echo "**Error**: version information not found in "\`${configure_ac}\'"." >&2
  exit 1
fi


if check_version $AUTOCONF $autoconf_vers_num $autoconf_vers ; then
    check_version $AUTOHEADER $autoconf_vers_num $autoconf_vers autoconf
fi
if check_version $AUTOMAKE $automake_vers_num $automake_vers; then
  check_version $ACLOCAL $automake_vers_num $autoconf_vers automake
fi
if check_version $GETTEXT $gettext_vers_num $gettext_vers; then
  check_version $MSGMERGE $gettext_vers_num $gettext_vers gettext
fi

if test "$DIE" = "yes"; then
    cat <<EOF

Note that you may use alternative versions of the tools by setting
the corresponding environment variables; see README.CVS for details.

EOF
    exit 1
fi

# Check the git setup.
if [ -d .git ]; then
  if [ -f .git/hooks/pre-commit.sample -a ! -f .git/hooks/pre-commit ] ; then
    cat <<EOF >&2
*** Activating trailing whitespace git pre-commit hook. ***
    For more information see this thread:
      http://mail.gnome.org/archives/desktop-devel-list/2009-May/msg00084html
    To deactivate this pre-commit hook again move .git/hooks/pre-commit
    and .git/hooks/pre-commit.sample out of the way.
EOF
      cp -av .git/hooks/pre-commit.sample .git/hooks/pre-commit
      chmod -c +x  .git/hooks/pre-commit
  fi
  tmp=$(git config --get filter.cleanpo.clean)
  if [ "$tmp" != "awk '/^\"POT-Creation-Date:/&&!s{s=1;next};!/^#: /{print}'" ]
  then
    echo "*** Adding GIT filter.cleanpo.clean configuration." >&2
    git config --add filter.cleanpo.clean \
        "awk '/^\"POT-Creation-Date:/&&!s{s=1;next};!/^#: /{print}'"
  fi
  if [ -f build-aux/git-hooks/commit-msg -a ! -f .git/hooks/commit-msg ] ; then
    cat <<EOF >&2
*** Activating commit log message check hook. ***
EOF
      cp -av build-aux/git-hooks/commit-msg .git/hooks/commit-msg
      chmod -c +x  .git/hooks/commit-msg
  fi
fi

echo "Running aclocal -I m4 ${ACLOCAL_FLAGS:+$ACLOCAL_FLAGS }..."
$ACLOCAL -I m4  $ACLOCAL_FLAGS
echo "Running autoheader..."
$AUTOHEADER
echo "Running automake --gnu ..."
$AUTOMAKE --gnu;
echo "Running autoconf${FORCE} ..."
$AUTOCONF${FORCE}


echo "You may now run \"
  ./configure --enable-maintainer-mode
\" and then \"make\"
"
