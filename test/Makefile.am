TESTS = test_runner
check_PROGRAMS = $(TESTS)
test_runner_SOURCES = proxy_test.c \
                      mod_security_test.c \
                      repsheet_test.c \
                      test_runner.c \
                      ../src/proxy.c \
                      ../src/proxy.h \
                      ../src/mod_security.c \
                      ../src/mod_security.h \
                      ../src/repsheet.c \
                      ../src/repsheet.h
test_runner_CFLAGS = $(CHECK_CFLAGS)
test_runner_LDADD = $(CHECK_LIBS) -lpcre -lhiredis
