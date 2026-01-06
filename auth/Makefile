all:
	make -j 4 -C ./auth_cli
	make -j 4 -C ./auth_comm
	make -j 4 -C ./auth_trans

clean:
	make -C ./auth_cli clean
	make -C ./auth_comm clean
	make -C ./auth_trans clean

install:
	make -C ./auth_trans cp
