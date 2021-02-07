TESTS =		test_single_paste.sh \
		test_single_paste_abort.sh \
		test_single_paste_really_large_no_abort.sh \
		test_concurrent_pastes.sh

test: ${TESTS}

${TESTS}: FORCE
	sh "$@"

FORCE:

.PHONY: clean FORCE test
