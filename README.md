/proc file system, process ve kernella alakalı farklı istatistiklere ulaşılabileceğiniz bir arayüz
olarak işlev görmektedir. Her bir /proc/pid ile pid idli processin istatistiklerine yada /proc/kerneldatastructure ile kerneldatastructure kısmına isim vererek ilgili bilgilerine erişebilirsiniz. mesela


```
cat /proc/stat
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
ls /proc/irq/
```
>0 10 12 14 16 18 2 4 6 8 prof_cpu_mask <br>
1 11 13 15 17 19 3 5 7 9 default_smp_affinity <br>

```
ls /proc/irq/0/
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

Temelde yapacağımız, bu data structure'ın **proc_open**, **proc_realese**, **proc_read**, **proc_write**
pointerlarına gerekli atamaları yaptıktan sonra(bunlar file uzerinde yapilacak islemlerin davranislarini belirleyecek) aşağıdaki foksiyonla /proc file systemda dosya oluşturacağız:

```struct proc_dir_entry *proc_create(const char *name, umode_t mode, struct proc_dir_entry *parent, const struct proc_ops *proc_ops);```

> device deriver yazarken farklı olarak /dev altında struct cdev mydevice... ile device file
oluşturduktan sonra fileoperations tipindeki mydevice.ops.read vb üyelerine ilgili atamalar yapılır
ve device_create ile device file oluşturulur (yada terminalden mknod kullanabilirsiniz.).

<hr>
<H4>3.2.module ile procfs’e file ekleme/çıkarma</H4>

Daha önceki ödevde modul başlarken ve biterken hangi fonksiyonların çalıştırılabileceklerini
**module_init()** ve **module_exit()** ile yapmıştık.

Burada proc file systemda dosya oluşturma kısmını module_init()’e; bu dosyayı kaldırma kısmınıda module_exit()e argüman olarak vereceğiz. Bunun için öncelikli olarak my_module_init()
ve my_module_exit() şeklinde iki tane fonksiyon tanımlayalım. Bunlarda temel olarak dosya oluşturup kaldıracağız (/include/linux/proc_fs.h):

```
/* my_module.c */
#include <linux/init.h> /* Needed for the macros */
#include <linux/kernel.h> /* Needed for pr_info() */
#include <linux/module.h> /* Needed by all modules */
#include <linux/proc_fs.h> /*proc_ops, proc)create, proc_remove, remove_proc_entry...*/
#define PROCF_NAME "mytaskinfo"
const struct proc_ops my_ops = {
.proc_read = NULL,
.proc_write = NULL,
.proc_open = NULL,
.proc_release = NULL,
/*bunlari kullanarak dosya davranislarini belirleyebilirsiniz*/
};

/* This function is called when the module is loaded. */
static int __init my_module_init(void)
{
/* creates the [/proc/procf] entry*/
proc_create(PROCF_NAME, 0666, NULL, &my_ops);
printk(KERN_INFO "/proc/%s created\n", PROCF_NAME);
return 0;
}

/* This function is called when the module is removed. */
static void __exit my_module_exit(void)
{
/* removes the [/proc/procf] entry*/
remove_proc_entry(PROCF_NAME, NULL);
printk(KERN_INFO "/proc/%s removed\n", PROCF_NAME);
}
/* Macros for registering module entry and exit points.
*/
module_init(my_module_init);
module_exit(my_module_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("My Task Info Module");
MODULE_AUTHOR("kendi isminiz");

```

Bu adımdan sonra sudo insmod ile modülü yüklediğinizde /proc fs de kendi oluşturduğunuz dosyayı
görebilmeniz gerekiyor.

```
$ ls /proc/mytaskinfo
/proc/mytaskinfo
```
<hr>
<h4>3.3 Olusturdugumuz file’in open ve closeda yapacaklarini belirleme</h4>

[/proc/procf] olusturdugumuz dosya acildiginda ve kapandiginda sistem defualtlarindan farkli olarak ne yapilacagini belirleyebiliriz. Bunun icin yukaridaki proc_ops data structure’inda tanimli pointerlara uygun olarak; bizde asagidaki fonksiyon tanimlamalarini kullanacagiz.

```
int my_open(struct inode *inode, struct file *file)
{
printk(KERN_INFO "my_ropen() for /proc/%s \n", PROCF_NAME);
return 0;
}
int my_release(struct inode *inode, struct file *file)
{
printk(KERN_INFO "my_release() for /proc/%s \n", PROCF_NAME);
return 0;
}
```

Yukarida yazdigimiz module’de eger asagidaki degisikligi yaparsak
```
const struct proc_ops my_ops = {
.proc_read = NULL,
.proc_write = NULL,
.proc_open = my_open,
.proc_release = my_release,
/*bunlari kullanarak dosya davranislarini belirleyebilirsiniz*/
};

```

bir tane user_test.c programi yazalim:

```
/** user_test.c
*
*/
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
int main()
{
int fd = open("/proc/mytaskinfo", O_RDWR);
if (fd == -1)
{
printf("Couldn't open file\n");
return -1;
}
close(fd);
return 0;
}
```
Bu programi calistirdiktan sonra bash **$sudo dmesg** ile loga bakarak printk ile yukarida belirlemis olduğumuz mesajlarin yazildigini teyit edebilirsiniz.
