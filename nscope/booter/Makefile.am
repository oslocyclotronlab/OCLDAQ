

lib_LTLIBRARIES = libSystemBooter.la

bin_PROGRAMS = boot boot_extcl

noinst_PROGRAMS = unittests

include_HEADERS = SystemBooter.h

libSystemBooter_la_SOURCES = SystemBooter.cpp
libSystemBooter_la_CPPFLAGS = -I@top_srcdir@/configuration \
                                -I@top_srcdir@/xiaapi/app \
                                -I@top_srcdir@/xiaapi/sys \
                                -I@top_srcdir@/xiaapi/inc


libSystemBooter_la_LIBADD = @top_builddir@/configuration/libConfiguration.la \
                            @top_builddir@/xiaapi/app/libPixie16App.la \
                            @top_builddir@/xiaapi/sys/libPixie16Sys.la \
                            @PLX_LDFLAGS@


boot_SOURCES = boot.cpp
boot_CPPFLAGS = -I@srcdir@ -I@top_srcdir@/configuration \
                -I@top_srcdir@/xiaapi/inc -I@top_srcdir@/xiaapi/app \
                -I@top_srcdir@/xiaapi/sys \
                -DFIRMWARE_FILE=\"@prefix@/share/readout/DDASFirmwareVersions.txt\"
boot_LDADD = @builddir@/libSystemBooter.la \
                @top_builddir@/configuration/libConfiguration.la \
                @top_builddir@/xiaapi/app/libPixie16App.la \
                @top_builddir@/xiaapi/sys/libPixie16Sys.la \
                @PLX_LDFLAGS@ -ldl

boot_extcl_SOURCES = boot.cpp
boot_extcl_CPPFLAGS = -I@srcdir@ -I@top_srcdir@/configuration \
                -I@top_srcdir@/xiaapi/inc -I@top_srcdir@/xiaapi/app \
                -I@top_srcdir@/xiaapi/sys \
                -DFIRMWARE_FILE=\"@prefix@/share/readout/DDASFirmwareVersions_ExtClock.txt\"
boot_extcl_LDADD = @builddir@/libSystemBooter.la \
                    @top_builddir@/configuration/libConfiguration.la \
                    @top_builddir@/xiaapi/app/libPixie16App.la \
                    @top_builddir@/xiaapi/sys/libPixie16Sys.la \
                    @PLX_LDFLAGS@ -ldl


unittests_SOURCES = TestRunner.cpp SystemBooterTest.cpp
unittests_CPPFLAGS = -I@top_srcdir@/xiatest -I@top_srcdir@/configuration @CPPUNIT_CFLAGS@
unittests_LDADD = @builddir@/libSystemBooter.la \
                    @top_builddir@/configuration/libConfiguration.la \
                    @top_builddir@/xiatest/libTestPixie16.la \
                    @CPPUNIT_LIBS@

TESTS = unittests
