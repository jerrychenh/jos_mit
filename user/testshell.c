#include <inc/x86.h>
#include <inc/lib.h>

void wrong(int, int, int);

void
umain(int argc, char **argv)
{
	char c1, c2;
	int r, rfd, wfd, kfd, n1, n2, off, nloff;
	int pfds[2];

	close(0);
	close(1);
	opencons();
	opencons();

	if ((rfd = open("testshell.sh", O_RDONLY)) < 0)
		panic("open testshell.sh: %e", rfd);
	if ((wfd = pipe(pfds)) < 0)
		panic("pipe: %e", wfd);
	wfd = pfds[1];

	cprintf("running sh -x < testshell.sh | cat\n");
	if ((r = fork()) < 0)
		panic("fork: %e", r);
	if (r == 0) {
		// child
		// below 4 lines are just test? of course not:P
		// fd 0 is used for std input, while fd 1 is std output
		// 1. we first open 0, 1 by two calls to opencons, 
		// 2. then open "testshell.sh" which is fd 2
		// 3. call pipe(pfds), where pfds[0] is fd 3, pfds[1] is fd 4
		// 4. dup rfd to 0, so fd 0 is open testshell.sh
		// 5. dup wfd to 1, which is pfds[1], fd 4 the write only part of the pipe
		// 6. then fd 2 and fd 4 are closed
		// so the status is 0 <- testshell.sh, 1 <- write only part of pipe, 3 <- read only part of pipe
		dup(rfd, 0);
		dup(wfd, 1);
		close(rfd);
		close(wfd);
		if ((r = spawnl("/sh", "sh", "-x", 0)) < 0)
			panic("spawn: %e", r);
		close(0);
		close(1);
		wait(r);
		exit();
	}

	// parent
	// what if close(rfd) before child dup?
	// this rfd and child rfd are in different memory space
	// and fork() would have the page ref of rfd increase,
	// so it is ok to close separately, infact if not, fd would leak. 
	close(rfd);
	close(wfd);

	rfd = pfds[0];
	if ((kfd = open("testshell.key", O_RDONLY)) < 0)
		panic("open testshell.key for reading: %e", kfd);

	nloff = 0;
	for (off=0;; off++) {
		n1 = read(rfd, &c1, 1);
		n2 = read(kfd, &c2, 1);
		if (n1 < 0)
			panic("reading testshell.out: %e", n1);
		if (n2 < 0)
			panic("reading testshell.key: %e", n2);
		if (n1 == 0 && n2 == 0)
			break;
		if (n1 != 1 || n2 != 1 || c1 != c2)
			wrong(rfd, kfd, nloff);
		if (c1 == '\n')
			nloff = off+1;
	}
	cprintf("shell ran correctly\n");

	breakpoint();
}

void
wrong(int rfd, int kfd, int off)
{
	char buf[100];
	int n;

	seek(rfd, off);
	seek(kfd, off);

	cprintf("shell produced incorrect output.\n");
	cprintf("expected:\n===\n");
	while ((n = read(kfd, buf, sizeof buf-1)) > 0)
		sys_cputs(buf, n);
	cprintf("===\ngot:\n===\n");
	while ((n = read(rfd, buf, sizeof buf-1)) > 0)
		sys_cputs(buf, n);
	cprintf("===\n");
	exit();
}

