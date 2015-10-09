$(srcdir)/package.m4: $(top_srcdir)/configure.ac
	{                                      \
	echo '# Signature of the current package.'; \
	echo 'm4_define([AT_PACKAGE_NAME],      [@PACKAGE_NAME@])'; \
	echo 'm4_define([AT_PACKAGE_TARNAME],   [@PACKAGE_TARNAME@])'; \
	echo 'm4_define([AT_PACKAGE_VERSION],   [@PACKAGE_VERSION@])'; \
	echo 'm4_define([AT_PACKAGE_STRING],    [@PACKAGE_STRING@])'; \
	echo 'm4_define([AT_PACKAGE_BUGREPORT], [@PACKAGE_BUGREPORT@])'; \
	} >$(srcdir)/package.m4

ATCONFIG=atconfig
TESTSUITE_AT=testsuite.at
EXTRA_DIST = $(TESTSUITE_AT) $(srcdir)/package.m4 $(TESTSUITE) testdata
TESTSUITE = $(srcdir)/testsuite
DISTCLEANFILES = $(ATCONFIG)
check-local: $(ATCONFIG) $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' $(TESTSUITEFLAGS)
installcheck-local: $(ATCONFIG) $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' AUTOTEST_PATH='$(bindir)' \
		$(TESTSUITEFLAGS)
clean-local:
	test ! -f '$(TESTSUITE)' || \
		$(SHELL) '$(TESTSUITE)' --clean
AUTOM4TE = $(SHELL) $(top_srcdir)/missing --run autom4te
AUTOTEST = $(AUTOM4TE) --language=autotest
$(TESTSUITE): $(srcdir)/$(TESTSUITE_AT) $(srcdir)/package.m4
	$(AUTOTEST) -I '$(srcdir)' -o $@.tmp $@.at
	mv $@.tmp $@
