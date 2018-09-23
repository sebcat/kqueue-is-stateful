# kqueue is stateful

## What

Most people doing I/O multiplexing in process context on POSIX-compatible
systems are familiar with select(2) and poll(2), but maybe not so familiar
with kqueue(2)/kevent(2).

poll(2) as a system call does not maintain any state between invocations.
You give it an array of structs containing file descriptors and event
flags and the kernel fills in what events are triggered, which the
application then can check in O(N) time, where N is the number of file
descriptors.

kevent(2) as a system call manipulates state associated with the kqueue
file descriptor. You give it two arrays: one containing changes you want
to make to the kqueue, and one array receving the triggered events. You
also pass the kqueue file descriptor and a timeout.

You do *not* need to pass the set of events that you are interested in
to kevent, you only need to pass the changes. Events you have added to
the kqueue will still be checked for on subsequent kevent(2) calls, unless
the EV_DISPATCH or EV_ONESHOT flags are set. Adding the events again are
a no-op at best, and at worst you end up with complex code in user space
trying to maintain an interest set for no reason.

Also, EV_ADD | EV_ENABLE is redundant. EV_ADD implies EV_ENABLE, unless
EV_DISABLE is set.

## Usage

````
$ make kev
cc -O2 -pipe  kev.c  -o kev
$ ./kev
aaa
aaa
bbb
bbb
ccc
ccc
^C
$ printf 'foo\nbar\nbaz\n' | ./kev | od -t x1 -t c
0000000    66  6f  6f  0a  62  61  72  0a  62  61  7a  0a
           f   o   o  \n   b   a   r  \n   b   a   z  \n
0000014
````
