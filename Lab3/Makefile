CC=gcc
CFLAGS=-Wall #启动所有饿警报

vm_manager: vm_manager.o vm_manager.c
	$(CC) $(CFLAGS) -o vm_manager vm_manager.o

vm_manager.o: vm_manager.c
	$(CC) $(CFLAGS) -c vm_manager.c

clean:
	rm -f *.o vm_manager.exe vm_manager
