$(shell chmod u+x build_tools)
all:
	@./build_tools -b
clean:
	@./build_tools -c
