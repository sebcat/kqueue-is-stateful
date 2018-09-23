#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

static ssize_t copy_chunk(int from, int to) {
  ssize_t ret;
  char buf[512];

  ret = read(from, buf, sizeof(buf));
  if (ret <= 0) {
    return ret;
  }

  return write(to, buf, ret);
}

int main() {
  int kq;
  int ret;
  struct kevent ev;
  ssize_t ncopied;
  int status = EXIT_FAILURE;

  kq = kqueue();
  if (kq < 0) {
    perror("kqueue");
    return EXIT_FAILURE;
  }

  EV_SET(&ev, STDIN_FILENO, EVFILT_READ, EV_ADD, 0, 0, NULL);
  ret = kevent(kq, &ev, 1, NULL, 0, NULL);
  if (ret < 0) {
    perror("kevent add");
    goto done;
  }

  while (1) {
    ret = kevent(kq, NULL, 0, &ev, 1, NULL);
    if (ret < 0) {
      perror("kevent wait");
      goto done;
    } else if (ret == 0) {
      continue;
    }

    ncopied = copy_chunk(STDIN_FILENO, STDOUT_FILENO);
    if (ncopied < 0) {
      goto done;
    } else if (ncopied == 0) {
      break;
    }
  }

  status = EXIT_SUCCESS;
done:
  close(kq);
  return status;
}
