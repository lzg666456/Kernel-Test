#pragma once

#include <kinit.h>
#include <terminal/terminal.h>
#include <serial.h>
#include <libc.h>
#include <io.h>
#include <attr.h>
#include <bit.h>
#include <memory.h>
#include <signal_defs.h>
#include <log.h>
#include <arch/x86/cpu.h>
#include <fs.h>
#include <initrd.h>
#include <video.h>
#include <libc/tree.h>

/* Memory segment selectors: */
enum SEGSEL {
	SEG_NULL,
	SEG_KERNEL_CS = 0x8,
	SEG_KERNEL_DS = 0x10,
	SEG_USER_CS   = 0x18,
	SEG_USER_DS   = 0x20
};

/* Returns the size of a section (in relation to another section): */
#define SEGSIZE(seg_start, seg_end) ((uintptr_t)&seg_end - (uintptr_t)&seg_start)
/* Returns the difference between the end of the kernel binary and the start */
#define KERNELSIZE() SEGSIZE(kstart, end)

#define USER_STACK_BOTTOM 0xAFF00000
#define USER_STACK_TOP    0xB0000000
#define SHM_START         0xB0000000

#define asm __asm__
#define volatile __volatile__

#define ASSERT(cond, msg) { if(!(cond)) { char buff[256]; sprintf(buff, "Assert (%s): %s", STR(cond), msg); Kernel::Error::panic(buff, __LINE__, __FILE__, 0); } }

#ifndef MODULE
#define IRQ_OFF() Kernel::CPU::IRQ::int_disable()
#define IRQ_RES() Kernel::CPU::IRQ::int_resume()
#define IRQ_ON() Kernel::CPU::IRQ::int_enable()
#endif

#define KERNEL_PAUSE() { asm volatile ("hlt"); }
#define KERNEL_FULL_PAUSE() while (1) { KERNEL_PAUSE(); }

#define KERNEL_FULL_STOP() while(1) { IRQ_OFF(); KERNEL_FULL_PAUSE(); }

#define PUSH(stack, type, item) stack -= sizeof(type); *((type *) stack) = item

typedef volatile int spin_lock_t[2];
extern void spin_init(spin_lock_t lock);
extern void spin_lock(spin_lock_t lock);
extern void spin_unlock(spin_lock_t lock);

namespace Kernel {
	extern Terminal term;
	extern Serial serial;

	/* All CPU Related components, such as GDT,
	IDT (which includes ISR and PIC / IRQ) and registers */
	namespace CPU {
		namespace TSS {
			typedef struct tss_entry {
				uint32_t	prev_tss;
				uint32_t	esp0;
				uint32_t	ss0;
				uint32_t	esp1;
				uint32_t	ss1;
				uint32_t	esp2;
				uint32_t	ss2;
				uint32_t	cr3;
				uint32_t	eip;
				uint32_t	eflags;
				uint32_t	eax;
				uint32_t	ecx;
				uint32_t	edx;
				uint32_t	ebx;
				uint32_t	esp;
				uint32_t	ebp;
				uint32_t	esi;
				uint32_t	edi;
				uint32_t	es;
				uint32_t	cs;
				uint32_t	ss;
				uint32_t	ds;
				uint32_t	fs;
				uint32_t	gs;
				uint32_t	ldt;
				uint16_t	trap;
				uint16_t	iomap_base;
			} __packed tss_entry_t;
			void tss_set_kernel_stack(uintptr_t stack);
		}

		namespace GDT {
			void gdt_init(void);
			void gdt_set_gate(uint8_t num, uint64_t base, uint64_t limit, uint8_t access, uint8_t gran);
		}

		namespace IDT {
			/* IDT Interrupt List (includes ISRs and IRQs): */
			enum IDT_IVT {
				ISR_DIVBY0,
				ISR_RESERVED0,
				ISR_NMI,
				ISR_BREAK,
				ISR_OVERFLOW,
				ISR_BOUNDS,
				ISR_INVOPCODE,
				ISR_DEVICEUN,
				ISR_DOUBLEFAULT,
				ISR_COPROC,
				ISR_INVTSS,
				ISR_SEG_FAULT,
				ISR_STACKSEG_FAULT,
				ISR_GENERALPROT,
				ISR_PAGEFAULT,
				ISR_RESERVED1,
				ISR_FPU,
				ISR_ALIGNCHECK,
				ISR_SIMD_FPU,
				ISR_RESERVED2,
				ISR_USR,
				SYSCALL_VECTOR = 0x7F
			};

			void idt_init();
			void idt_set_gate(uint8_t num, uintptr_t isr_addr, uint16_t sel, uint8_t flags);
		}

		namespace ISR {
			#define ISR_COUNT 32

			/* ISR Messages: */
			extern const char *exception_msgs[ISR_COUNT];

			/* Function callback type for ISRs: */
			typedef void(*isr_handler_t) (CPU::regs_t *);

			void isrs_install(void);
			void isr_install_handler(size_t isr_num, isr_handler_t handler);
			void isr_uninstall_handler(size_t isrs);
		}

		namespace IRQ {
			enum IRQ_NUMS {
				IRQ_PIT, IRQ_KBD, IRQ_CASCADE, IRQ_COM2, IRQ_COM1,
				IRQ_LPT2, IRQ_FLOPPY, IRQ_LPT1, IRQ_CMOS, IRQ_FREE1,
				IRQ_FREE2, IRQ_FREE3, IRQ_MOUSE, IRQ_FPU, IRQ_PRIM_ATA, IRQ_SEC_ATA
			};

			typedef int(*irq_handler_t) (CPU::regs_t *); /* Callback type */

			void irq_install(void);

			void int_disable(void);
			void int_enable(void);
			void int_resume(void);

			void irq_install_handler(size_t irq_num, irq_handler_t irq_handler);
			void irq_uninstall_handler(size_t irq_num);
			void irq_ack(size_t irq_num);
			void irq_mask(uint8_t irq_num, uint8_t enable);
			void irq_set_mask(uint8_t irq_num);
			void irq_clear_mask(uint8_t irq_num);
		}

		/* Enter Ring3 (usermode): */
		void usermode_enter(uintptr_t location, int argc, char ** argv, uintptr_t stack);
	}

#ifndef MODULE
	namespace Memory {
		/* Kernel memory manager. Contains paging/physical memory functions and a Kernel memory
		allocator (kmalloc, a dumb version of Alloc's malloc to be used before paging is enabled) */
		namespace Man {
			void kheap_starts(uintptr_t start_addr);
			uintptr_t kmalloc(size_t size, char align, uintptr_t * phys);
			uintptr_t kmalloc(size_t size);
			uintptr_t kvmalloc(size_t size);
			uintptr_t kmalloc_p(size_t size, uintptr_t *phys);
			uintptr_t kvmalloc_p(size_t size, uintptr_t *phys);
			void *sbrk(uintptr_t increment);

			void paging_install(uint32_t memsize);
			void heap_install(void);

			/* Page allocators/deallocators: */
			void alloc_page(page_t * page, char is_kernel, char is_writeable, uintptr_t map_to_virtual);
			void alloc_page(char is_kernel, char is_writeable, uintptr_t physical_address, uintptr_t map_to_virtual);
			void alloc_page(char is_kernel, char is_writeable, uintptr_t physical_address);
			void alloc_page(char is_kernel, char is_writeable);
			void alloc_pages(char is_kernel, char is_writeable, uintptr_t physical_address_start, uintptr_t physical_address_end);
			char alloc_pages(char is_kernel, char is_writeable, uintptr_t physical_address_start, uintptr_t physical_address_end, uintptr_t virtual_addr_start,  uintptr_t virtual_addr_end);
			void dealloc_page(page_t * page);
			void dealloc_page(uintptr_t physical_address);

			extern paging_directory_t * kernel_directory;
			extern paging_directory_t * curr_dir;

			extern uintptr_t frame_ptr;
		}

		/* Proper memory allocator to be used after paging and heap are fully installed: */
		namespace Alloc {
			void * __malloc malloc(size_t size);
			void * __malloc realloc(void *ptr, size_t size);
			void * __malloc calloc(size_t nmemb, size_t size);
			void * __malloc valloc(size_t size);
			void free(void *ptr);
		}
	}

	/* Tasking: */
	namespace Task {
		/* Max number of processes the kernel will switch: */
		#define MAX_PID 32768
		#define MAX_TTL 2000 /* Maximum allowable time to live (expressed in switching count, not by ms or seconds) */
		#define TASK_STACK_SIZE (PAGE_SIZE * 2)
		#define USER_ROOT_UID ((user_t)0)

		#define THREAD_RETURN 0xFFFFB00F

		typedef signed int    pid_t;
		typedef unsigned int  user_t;
		typedef unsigned char status_t;

		typedef void (*tasklet_t)(void *, char *);

		typedef struct {
			unsigned int gs, fs, es, ds;
			unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
			unsigned int int_no, err_code;
			unsigned int eip, cs, eflags, useresp, ss;
		} regs_t;

		enum task_state {
			TASKST_CRADLE,
			TASKST_READY,
			TASKST_RUNNING,
			TASKST_BLOCKED,
			TASKST_GRAVE
		};

		enum wait_option {
			WCONTINUED,
			WNOHANG,
			WUNTRACED
		};

		/* Thread struct definition: */
		typedef struct thread {
			uintptr_t esp, ebp, eip;
			uint8_t fpu_enabled;
			uint8_t fp_regs[512];
			uint8_t padding[32];
			paging_directory_t * page_dir;
		} thread_t;

		/* Portable image struct: */
		typedef struct image {
			size_t size;           /* Image size */
			uintptr_t entry;       /* Binary entry point */
			uintptr_t heap;        /* Heap pointer */
			uintptr_t heap_actual; /* Actual heap location */
			uintptr_t stack;       /* Process kernel stack */
			uintptr_t user_stack;  /* User stack */
			uintptr_t start;
			uintptr_t shm_heap;
			volatile int lock[2];
		} image_t;

		typedef struct file_descriptor_table {
			FILE ** entries;
			size_t length;
			size_t capacity;
			size_t refs;
		} fd_table_t;

		typedef struct signal_table {
			uintptr_t functions[NUMSIGNALS + 1];
		} sig_table_t;

		/* Task struct definition: */
		typedef struct task {
			/* Identification: */
			pid_t pid;
			char * name;
			char * desc;
			pid_t group;
			pid_t job;
			pid_t session;

			/* Execution mode: */
			user_t user;
			int mask;
			char ** cmdline;

			/* Context: */
			regs_t * syscall_regs;
			image_t image;

			/* Tasking: */
			thread_t thread;

			/* Procfs/tree structure and Files: */
			tree_node_t * tree_entry;
			FILE * wd_node;
			char * work_dirpath; /* Working directory */
			fd_table_t * fds;

			/* TTL: */
			char ttl_pwm_mode;
			int ttl;
			int ttl_start;
			int ttl_fscale; /* Switching frequency scale */

			/* States:*/
			status_t status;
			uint8_t finished;
			uint8_t started;
			uint8_t running;
			int exitcode;

			/* Signals: */
			sig_table_t signals;
			list_t * signal_queue;
			thread_t signal_state;
			char * signal_kstack;

			/* Waiting: */
			list_t * wait_queue;
			node_t sched_node;
			node_t sleep_node;
			node_t * timed_sleep_node;
			volatile uint8_t sleep_interrupted;

			/* Shared memory: */
			list_t * shm_mappings;

			/* Process type: */
			uint8_t is_tasklet;
		} task_t;

		typedef struct sleeper {
			unsigned long end_tick;
			unsigned long end_subtick;
			task_t * task;
		} sleeper_t;

		extern volatile task_t * current_task;
		extern task_t * main_task;

		void tasking_install(void);
		void switch_task(status_t new_process_state);
		void tasking_enable(char enable);
		void task_set_ttl(task_t * task, int duty_cycle_or_preload);
		void task_set_ttl(int pid, int duty_cycle_or_preload);
		void task_set_ttl_fscale(task_t * task, int fscale);
		void task_set_ttl_fscale(int pid, int fscale);
		void task_set_ttl_mode(task_t * task, char pwm_or_pulse_mode);
		void task_set_ttl_mode(int pid, char pwm_or_pulse_mode);
		void task_exit(int pid);
		void kexit(int retval);
		void task_free(task_t * task_to_free, int retval);

		void set_task_environment(task_t * task, paging_directory_t * pagedir);

		task_t * spawn_rootproc(void);
		task_t * spawn_childproc(task_t * parent);
		task_t * spawn_proc(task_t * parent, char addtotree, paging_directory_t * pagedir);

		uint32_t fork(void);
		uint32_t task_clone(uintptr_t new_stack, uintptr_t thread_function, uintptr_t arg);
		int task_create_tasklet(tasklet_t tasklet, char * name, void * argp);

		task_t * current_task_get(void);
		uint32_t current_task_getpid(void);
		task_t * task_from_pid(pid_t pid);
		task_t * task_get_parent(task_t * task);

		void make_task_ready(task_t * task);
		int wakeup_queue(list_t * queue);
		int wakeup_queue_interrupted(list_t * queue);
		void wakeup_sleepers(unsigned long seconds, unsigned long subseconds);
		int sleep_on(list_t * queue);
		void sleep_until(task_t * task, unsigned long seconds, unsigned long subseconds);

		uint32_t task_append_fd(task_t * task, FILE * node);
		uint32_t process_move_fd(task_t * task, int src, int dest);
	}
#endif

	/* Error related functions: */
	namespace Error {
		void panic(const char * msg, const int line, const char * file, int intno);
		void panic(const char * msg, int intno);
		void panic(const char * msg);
		void panic(void);
		void infinite_idle(const char * msg);
	}

	/* Virtual Mode 8086: */
	namespace VM8086 {
		char v86_detect(void);
		unsigned v86_peekb(unsigned seg, unsigned off);
		unsigned v86_peekw(unsigned seg, unsigned off);
		void v86_pokeb(unsigned seg, unsigned off, unsigned val);
		void v86_pokew(unsigned seg, unsigned off, unsigned val);
		void v86_push16(CPU::regs_t *regs, unsigned value);
		void v86_int(CPU::regs_t *regs, unsigned int_num);
		void v86_test(void);
	}

	/* System Calls: */
	namespace Syscall {
		#define SYSCALL_MAXCALLS 128
		typedef uint32_t (*syscall_callback_t)(unsigned int, ...);

		void syscalls_initialize(void);
		char syscall_schedule_install(char * syscall_name, int no, uintptr_t syscall_addr);
		#define syscall_install(syscall_name, no) syscall_install_s((char*)# syscall_name, no, (uintptr_t)syscall_name)
		void syscall_install_n(int no, uintptr_t syscall);
		void syscall_install_s(char * syscall_name, int no, uintptr_t syscall_addr);
		void syscall_install_s(char * syscall_name, uintptr_t syscall_addr);
		void syscall_install_p(uintptr_t syscall_addr);
		void syscall_uninstall(uintptr_t syscall);
		void syscall_run_n(int intno);
		void syscall_run_s(char * syscall_name);
		#define syscall_run(syscall_name) syscall_run_s((char*)# syscall_name)
	}

	/* Shared Memory: */
	namespace SharedMemory {
		void shm_install(void);
	}
}

#ifndef MODULE
using namespace Kernel::Memory::Man;
using namespace Kernel::Memory::Alloc;
using namespace Kernel::Task;

#ifdef __cplusplus
inline void * operator new(__SIZE_TYPE__ n) {
	return malloc(n);
}

inline void * operator new[](__SIZE_TYPE__ n) {
	return malloc(n);
}
#endif

#endif
using namespace Kernel::Error;
using namespace Kernel::KInit;
using namespace Kernel::VM8086;
using namespace Kernel::Syscall;
using namespace Kernel::SharedMemory;
