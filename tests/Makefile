TESTS =		test_*

check: test

test: ${TESTS}

${TESTS}: keygen
	sh "$@"

keygen: clean FORCE
	openssl req -x509 -out PB.crt -keyout PB.key -newkey rsa:2048 -nodes -sha256 -subj '/CN=localhost' -extensions EXT -config PB.conf

FORCE:

clean:
	rm -f PB.key PB.crt

.PHONY: check clean FORCE keygen test
