all:
	make -j 4 -C ./ident_cli
	make -j 4 -C ./ident_comm
	make -j 4 -C ./ident_trans
	make -j 4 -C ./ident_app

clean:
	make -C ./ident_cli clean
	make -C ./ident_comm clean
	make -C ./ident_trans clean
	make -C ./ident_app clean

install:
	make -C ./ident_trans cp
