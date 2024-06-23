/proc file system, process ve kernella alakalı farklı istatistiklere ulaşılabileceğiniz bir arayüz
olarak işlev görmektedir. Her bir /proc/pid ile pid idli processin istatistiklerine yada /proc/kerneldatastructure ile kerneldatastructure kısmına isim vererek ilgili bilgilerine erişebilirsiniz. mesela


```
cat /proc/stat <br>
```
>cpu 2255 34 2290 22625563 6290 127 456 0 0 0 <br>
cpu0 1132 34 1441 11311718 3675 127 438 0 0 0 <br>
cpu1 1123 0 849 11313845 2614 0 18 0 0 0 <br>
intr 114930548 113199788 3 0 5 263 0 4 [... lots more numbers ...] <br>
ctxt 1990473 <br>
btime 1062191376 <br>
processes 2915 <br>
procs_running 1 <br>
procs_blocked 0 <br>
softirq 183433 0 21755 12 39 1137 231 21459 2263 

```
ls /proc/irq/ <br>
```
>0 10 12 14 16 18 2 4 6 8 prof_cpu_mask <br>
1 11 13 15 17 19 3 5 7 9 default_smp_affinity <br>

```
ls /proc/irq/0/ <br>
```
>smp_affinity <br>
------------------------

<h3>Linux Kernel Modülle /proc file systeme dosya eklemek</h3> 

<h4>3.1 Genel Ozet</h4>

Kernel tarafında struct file_operations ve kernel 5.6dan sonra eklenen proc_ops şeklinde data structurelar tanımlanmıştır. Bu data structureların temel özellikleri okuma ve yazma yapılırken çağrılacak fonksiyonları içermesidir.

'struct proc_ops {

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

#endif'
