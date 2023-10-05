#include <liburing.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/timerfd.h>
#include <unistd.h>

#define TIMER_INTERVAL_SEC 2

void timer_callback(struct io_uring *ring, int timer_fd) {
  uint64_t expirations;
  ssize_t ret;

  // Read the number of timer expirations
  ret = read(timer_fd, &expirations, sizeof(expirations));
  if (ret < 0) {
    perror("read");
    exit(1);
  }

  printf("Timer expired! Number of expirations: %lu\n", expirations);

  // Add the timer_fd to io_uring again for the next expiration
  struct io_uring_sqe *sqe;
  sqe = io_uring_get_sqe(ring);
  io_uring_prep_poll_add(sqe, timer_fd, POLLIN);
  io_uring_sqe_set_data(sqe, NULL);
  io_uring_submit(ring);
}

int main() {
  struct io_uring ring;
  int timer_fd;
  struct itimerspec its;
  struct timespec timeout = {TIMER_INTERVAL_SEC, 0};

  // Initialize io_uring
  if (io_uring_queue_init(64, &ring, 0) < 0) {
    perror("io_uring_queue_init");
    return 1;
  }

  // Create a timer using timerfd
  timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
  if (timer_fd < 0) {
    perror("timerfd_create");
    return 1;
  }

  // Set the timer to trigger at a regular interval
  its.it_interval = timeout;
  its.it_value = timeout;
  timerfd_settime(timer_fd, 0, &its, NULL);

  // Add the timer_fd to io_uring
  struct io_uring_sqe *sqe;
  sqe = io_uring_get_sqe(&ring);
  io_uring_prep_poll_add(sqe, timer_fd, POLLIN);
  io_uring_sqe_set_data(sqe, NULL);
  io_uring_submit(&ring);

  while (1) {
    struct io_uring_cqe *cqe;
    if (io_uring_wait_cqe(&ring, &cqe) < 0) {
      perror("io_uring_wait_cqe");
      return 1;
    }

    timer_callback(&ring, timer_fd);

    // Release the CQE
    io_uring_cqe_seen(&ring, cqe);
  }

  // Clean up
  io_uring_queue_exit(&ring);
  close(timer_fd);

  return 0;
}
