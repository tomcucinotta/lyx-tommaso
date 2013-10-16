# Macro to add for using qxmpp libraries!     -*- sh -*-
AC_DEFUN([CHECK_WITH_QXMPP],
[
	lyx_use_qxmpp=true
	AC_ARG_WITH(qxmpp, AC_HELP_STRING([--without-qxmpp],[do not check for QXMPP library]))
	test "$with_qxmpp" = "no" && lyx_use_qxmpp=false

	if $lyx_use_qxmpp; then
		PKG_CHECK_MODULES([QXMPP], [qxmpp], [], [
			AC_CHECK_HEADERS(qxmpp/QXmppClient.h,
				[lyx_use_qxmpp=true; break;],
				[lyx_use_qxmpp=false])
			AC_CHECK_LIB(qxmpp, main, LIBS="-lqxmpp $LIBS", lyx_use_qxmpp=false)
		])
		AC_MSG_CHECKING([whether to use qxmpp])
		if $lyx_use_qxmpp ; then
			AC_MSG_RESULT(yes)
			AC_DEFINE(USE_QXMPP, 1, [Define as 1 to use the qxmpp library])
			lyx_flags="$lyx_flags use-qxmpp"
		else
			AC_MSG_RESULT(no)
		fi
	fi
])

AC_DEFUN([LYX_CHECK_QXMPP],
[
	CHECK_WITH_QXMPP
	AM_CONDITIONAL(USE_QXMPP, $lyx_use_qxmpp)
	])
