TESTS =		test_single_paste.sh \
		test_single_paste_abort.sh \
		test_single_paste_runtime_change_perms.sh \
		test_single_paste_really_large_no_abort.sh \
		test_single_paste_really_large_abort.sh \
		test_concurrent_pastes.sh \
		test_concurrent_pastes_really_large_no_abort.sh \
		test_single_paste_ssl.sh \
		test_concurrent_pastes_ssl.sh \
		test_concurrent_pastes_really_large_no_abort_ssl.sh

check: test

test: ${TESTS}

${TESTS}: FORCE PB.key
	sh "$@"

PB.key:
	openssl req -x509 -out PB.crt -keyout PB.key -newkey rsa:2048 -nodes -sha256 -subj '/CN=localhost' -extensions EXT -config <( printf "[dn]\nCN=localhost\n[req]\ndistinguished_name = dn\n[EXT]\nsubjectAltName=DNS:localhost\nkeyUsage=digitalSignature\nextendedKeyUsage=serverAuth")

FORCE:

clean:
	rm -f PB.key PB.crt

.PHONY: check clean FORCE keygen test
