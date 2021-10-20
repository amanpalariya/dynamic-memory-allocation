--dependencies = logger.c ds.c
--libraries = -lpthread
--build-dir = build
--main-file = main.c
--output-filename = main.out
--output-filepath = ${--build-dir}/${--output-filename}

--test-dir = tests
--test-filename = tester.c
--test-filepath = ${--test-dir}/${--test-filename}
--test-output-filename = tester.out
--test-output-filepath = ${--test-dir}/${--test-output-filename}

main: ${--main-file} ${--dependencies}
	@echo "Compiling..."
	@mkdir -p ${--build-dir}
	@gcc ${--main-file} ${--dependencies}  ${--libraries} -o ${--output-filepath}
	@echo "Compiled to \"${--output-filepath}\""

test: ${--test-filepath} ${--dependencies}
	@gcc ${--test-filepath} ${--dependencies}  ${--libraries} -o ${--test-output-filepath}
	@${--test-output-filepath}
	@rm ${--test-output-filepath}

clean: ${--build-dir}
	@rm ${--build-dir}/*