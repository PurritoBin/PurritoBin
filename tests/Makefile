TESTS =		test_single_paste.sh test_concurrent_pastes.sh
TESTS_OUTPUT =	${TESTS:.sh=.sh.o}

test: ${TESTS}

${TESTS}: FORCE
	sh "$@" | tee "$@".o

FORCE:

clean:
	rm -f ${TESTS_OUTPUT}

.PHONY: clean FORCE test
