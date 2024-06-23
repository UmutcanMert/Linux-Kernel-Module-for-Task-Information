/proc file system, process ve kernella alakalı farklı istatistiklere ulaşılabileceğiniz bir arayüz
olarak işlev görmektedir. Her bir /proc/pid ile pid idli processin istatistiklerine yada

/proc/kerneldatastructure ile kerneldatastructure kısmına isim vererek ilgili bilgilerine erişebilirsiniz. mesela

> cat /proc/stat
cpu 2255 34 2290 22625563 6290 127 456 0 0 0
cpu0 1132 34 1441 11311718 3675 127 438 0 0 0
cpu1 1123 0 849 11313845 2614 0 18 0 0 0
intr 114930548 113199788 3 0 5 263 0 4 [... lots more numbers ...]
ctxt 1990473
btime 1062191376
processes 2915
procs_running 1
procs_blocked 0
softirq 183433 0 21755 12 39 1137 231 21459 2263
> ls /proc/irq/
0 10 12 14 16 18 2 4 6 8 prof_cpu_mask
1 11 13 15 17 19 3 5 7 9 default_smp_affinity
> ls /proc/irq/0/
smp_affinity
> cat /proc/interrupts
CPU0 CPU1

0: 1243498 1214548 IO-APIC-edge timer
1: 8949 8958 IO-APIC-edge keyboard
2: 0 0 XT-PIC cascade

1

5: 11286 10161 IO-APIC-edge soundblaster
8: 1 0 IO-APIC-edge rtc
9: 27422 27407 IO-APIC-edge 3c503
12: 113645 113873 IO-APIC-edge PS/2 Mouse
13: 0 0 XT-PIC fpu
14: 22491 24012 IO-APIC-edge ide0
15: 2183 2415 IO-APIC-edge ide1
17: 30564 30414 IO-APIC-level eth0
18: 177 164 IO-APIC-level bttv
NMI: 2457961 2457959
LOC: 2457882 2457881
ERR: 2155
> cat /proc/net/dev
Inter-|Receive |[...
face |bytes packets errs drop fifo frame compressed multicast|[...
lo: 908188 5596 0 0 0 0 0 0 [...
ppp0:15475140 20721 410 0 0 410 0 0 [...
eth0: 614530 7085 0 0 0 0 0 1 [...
...] Transmit
...] bytes packets errs drop fifo colls carrier compressed
...] 908188 5596 0 0 0 0 0 0
...] 1375103 17405 0 0 0 0 0 0
...] 1703981 5535 0 0 0 3 0 0
3 Linux Kernel Modülle /proc file systeme dosya eklemek
3.1 Genel Ozet
Kernel tarafında struct file_operations ve kernel 5.6dan sonra eklenen proc_ops şeklinde data
structurelar tanımlanmıştır. Bu data structureların temel özellikleri okuma ve yazma yapılırken
çağrılacak fonksiyonları içermesidir.
struct proc_ops {
unsigned int proc_flags;
int (*proc_open)(struct inode *, struct file *);
ssize_t (*proc_read)(struct file *, char __user *, size_t, loff_t *);
ssize_t (*proc_read_iter)(struct kiocb *, struct iov_iter *);
ssize_t (*proc_write)(struct file *, const char __user *, size_t, loff_t *);
/* mandatory unless nonseekable_open() or equivalent is used */
loff_t (*proc_lseek)(struct file *, loff_t, int);
int (*proc_release)(struct inode *, struct file *);
__poll_t (*proc_poll)(struct file *, struct poll_table_struct *);
long (*proc_ioctl)(struct file *, unsigned int, unsigned long);
#ifdef CONFIG_COMPAT
long (*proc_compat_ioctl)(struct file *, unsigned int, unsigned long);
#endif

2

int (*proc_mmap)(struct file *, struct vm_area_struct *);
unsigned long (*proc_get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
} __randomize_layout;
Temelde yapacağımız, bu data structureınproc_open, proc_realese, proc_read, proc_write

pointerlarına gerekli atamaları yaptıktan sonra(bunlar file uzerinde yapilacak islemlerin davranis-
larini belirleyecek) aşağıdaki foksiyonla /proc file systemda dosya oluşturacağız:
