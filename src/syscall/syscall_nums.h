/*
 * syscall_nums.h
 *
 *  Created on: 22/07/2016
 *      Author: Miguel
 */

#ifndef SRC_SYSCALL_NUMS_H_
#define SRC_SYSCALL_NUMS_H_

#define SYS_EXT 0
#define SYS_OPEN 2
#define SYS_READ 3
#define SYS_WRITE 4
#define SYS_CLOSE 5
#define SYS_GETTIMEOFDAY 6
#define SYS_EXECVE 7
#define SYS_FORK 8
#define SYS_GETPID 9
#define SYS_SBRK 10
#define SYS_UNAME 12
#define SYS_OPENPTY 13
#define SYS_SEEK 14
#define SYS_STAT 15
#define SYS_MKPIPE 21
#define SYS_DUP2 22
#define SYS_GETUID 23
#define SYS_SETUID 24
#define SYS_REBOOT 26
#define SYS_READDIR 27
#define SYS_CHDIR 28
#define SYS_GETCWD 29
#define SYS_CLONE 30
#define SYS_SETHOSTNAME 31
#define SYS_GETHOSTNAME 32
#define SYS_MKDIR 34
#define SYS_SHM_OBTAIN 35
#define SYS_SHM_RELEASE 36
#define SYS_KILL 37
#define SYS_SIGNAL 38
#define SYS_GETTID 41
#define SYS_YIELD 42
#define SYS_SYSFUNC 43
#define SYS_SLEEPABS 45
#define SYS_SLEEP 46
#define SYS_IOCTL 47
#define SYS_ACCESS 48
#define SYS_STATF 49
#define SYS_CHMOD 50
#define SYS_UMASK 51
#define SYS_UNLINK 52
#define SYS_WAITPID 53
#define SYS_PIPE 54
#define SYS_MOUNT 55
#define SYS_SYMLINK 56
#define SYS_READLINK 57
#define SYS_LSTAT 58

#define SYSDECL(name, ...) extern "C" int name(__VA_ARGS__); int name(__VA_ARGS__)

#endif /* SRC_SYSCALL_NUMS_H_ */
