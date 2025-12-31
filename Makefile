all:
	make -C auth
	make -C ident
clean:
	make -C auth clean
	make -C ident clean
install:
	make -C auth install
	make -C ident install
