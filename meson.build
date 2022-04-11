project('PurritoBin', 'cpp',
	version: '0.6.7',
	license: 'ISC',
	default_options: [
		'cpp_std=c++17',
		'warning_level=3',
	]
)

add_project_arguments(
	[ '-DUWS_NO_ZLIB' ],
	language: 'cpp'
)

cpp      = meson.get_compiler('cpp')

cpp.has_header('lmdb++.h', required: true)
cpp.has_header('uWebSockets/App.h', required: true)

lmdb     = cpp.find_library('lmdb', required: true)

threads  = dependency('threads', required: true)
usockets = dependency('libusockets', required: true)

purrito  = executable('purrito', 'src/main.cc', dependencies: [ lmdb, threads, usockets ], install: true)
install_man('man/purrito.1')
install_data('frontend/about.html',
             'frontend/index.html',
             'frontend/paste.html',
             'frontend/decrypt.js',
             'frontend/submit.js',
             'frontend/style.css',
             'vendor/crypto-js-4.1.1.min.js',
             'clients/POSIX_shell_client.sh'
)

if get_option('enable_testing')
	find_program('curl')
        find_program(get_option('test_shuf'))
        find_program(get_option('test_seq'))
	openssl   = find_program('openssl')
	gen_certs = run_command(openssl,
		    'req',
		    '-x509',
		    '-out',
		    meson.build_root() / 'PB.crt',
		    '-keyout',
		    meson.build_root() / 'PB.key',
		    '-newkey',
		    'rsa:2048',
		    '-nodes',
		    '-sha256',
		    '-subj',
		    '/CN=localhost',
		    '-extensions',
		    'EXT',
		    '-config',
		    meson.source_root() / 'tests/PB.conf'
	)
	message(gen_certs.stdout())
	if gen_certs.returncode() != 0
		error(gen_certs.stderr())
	endif
	sh    = find_program('sh')
	tests = [
		'test_nossl_concurrent_pastes.sh',
		'test_nossl_concurrent_pastes_really_large_no_abort.sh',
		'test_nossl_getpaste.sh',
		'test_nossl_single_paste.sh',
		'test_nossl_single_paste_abort.sh',
		'test_nossl_single_paste_really_large_abort.sh',
		'test_nossl_single_paste_really_large_no_abort.sh',
		'test_ssl_concurrent_pastes.sh',
		'test_ssl_concurrent_pastes_really_large_no_abort.sh',
		'test_ssl_getpaste.sh',
		'test_ssl_single_paste.sh'
	]
	foreach ts : tests
		test(ts, sh,
			args: [ ts ],
			env: {
				'PURRITO':  get_option('test_valgrind_wrapper') + ' ' + purrito.full_path(),
				'SHUF':     get_option('test_shuf'),
				'SEQ':      get_option('test_seq'),
				'P_KEY':    meson.build_root() / 'PB.key',
				'P_CRT':    meson.build_root() / 'PB.crt'
			},
			depends: [ purrito ],
			workdir: meson.current_source_dir() / 'tests'
		)
	endforeach
endif
