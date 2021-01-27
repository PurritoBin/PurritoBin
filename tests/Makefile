TESTS =		test_single_paste.sh
TESTS_OUTPUT =	${TESTS:.sh=.o}
TESTS_SUCCESS =	${TESTS:.sh=.sh.success}

test: ${TESTS_SUCCESS} common.sh

${TESTS_SUCCESS}: ${TESTS_OUTPUT}

${TESTS_OUTPUT}: ${TESTS}
	sh $< | tee $@

clean:
	rm -f ${TESTS_OUTPUT} ${TESTS_SUCCESS}

.PHONY: clean test
