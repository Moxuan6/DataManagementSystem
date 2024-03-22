all:
	gcc server.c -o server-arm64 -lsqlite3 -Wall
	gcc client.c -o client-arm64 -Wall
	@arm-none-linux-gnueabihf-gcc server.c -o server-arm `pkg-config --cflags --libs sqlite3`
	cp server-arm ~/rootfs/
clean:
	rm -rf server-arm64 client-arm64 server-arm