/* my_module.c */
#include <linux/init.h> /* Needed for the macros */
#include <linux/kernel.h> /* Needed for pr_info() */
#include <linux/module.h> /* Needed by all modules */
#include <linux/proc_fs.h> /*proc_ops, proc)create, proc_remove, remove_proc_entry...*/
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/list.h>
#include <linux/tty.h>

#define PROCF_NAME "mytaskinfo"

#define MYBUF_SIZE 256
#define MYDATA_SIZE 256

static char filter_state = '\0'; /* echo'dan gelen harf kosulu icin  */

static const char * const task_state_array[] = 
{
/* states in TASK_REPORT: */
"R (running)", /* 0x00 */
"S (sleeping)", /* 0x01 */
"D (disk sleep)", /* 0x02 */
"T (stopped)", /* 0x04 */
"t (tracing stop)", /* 0x08 */
"X (dead)", /* 0x10 */
"Z (zombie)", /* 0x20 */
"P (parked)", /* 0x40 */
/* states beyond TASK_REPORT: */
"I (idle)", /* 0x80 */
};

struct my_data
{
	int size;
	char *buf; /* my data starts here */
};

/*utime ve stime icin saniyeye cevirme fonksiyonu */
unsigned long jiffies_to_seconds(unsigned long j)
{
    return j / HZ;
}

static ssize_t my_read(struct file *file, char __user *usr_buf, size_t size, loff_t *offset)
{
    struct my_data *my_data = (struct my_data *)file->private_data;
    struct task_struct *task;
    int len;

    int length = 0;
	int counter = 0;
    
    unsigned long utime_seconds, stime_seconds; 
    
    if (!my_data->buf) 
    {
        my_data->size = 256 * 100;  /* initializing size */
        my_data->buf = kmalloc(my_data->size, GFP_KERNEL);
        if (!my_data->buf) 
        {
            pr_err("Memory allocation for buf failed\n");
            return -ENOMEM;
        }
    }
    

    for_each_process(task) 
    {
	    char temp_buffer[256];  /* gecici buffer icin boyut */
	    int temp_length;
	    counter = counter + 1;
	    
	    /* utime ve stime kisimlarini daha anlamli sayilara cevirme (nano to saniye) */
	    utime_seconds = jiffies_to_seconds(task->utime);
		stime_seconds = jiffies_to_seconds(task->stime);
		

		if (filter_state && task_state_array[task_state_index(task)][0] != filter_state)
        {
            continue;
        }

	    temp_length = snprintf(temp_buffer, sizeof(temp_buffer), "%d. pid = %d\tstate = %s\tutime = %llu\tstime = %llu\tutime+stime = %llu\tvruntime = %llu\n",
	                                   counter,task->pid,task_state_array[task_state_index(task)], (unsigned long long)utime_seconds, (unsigned long long)stime_seconds, (unsigned long long)(utime_seconds + stime_seconds), (unsigned long long)task->se.vruntime);

	    
	    if (length + temp_length > my_data->size) 
	    {
	            my_data->size += 256 * 100;  
	            my_data->buf = krealloc(my_data->buf, my_data->size, GFP_KERNEL);
	            if (!my_data->buf) 
	            {
	                pr_err("Memory reallocation for buf failed\n");
	                return -ENOMEM;
	            }
	    }
    	/* temp_buffer'i my_data->buf'a ekle */
	    strncpy(my_data->buf + length, temp_buffer, temp_length);
	    length += temp_length;
	}
	my_data->buf[length] = '\0';

	

    len = min((int)(my_data->size - *offset), (int)size);

    if (len <= 0)
    {
        return 0;
    }

 	if (copy_to_user(usr_buf, my_data->buf + *offset, len))
 	{
        return -EFAULT;
 	}

    *offset = *offset + len;
    filter_state = '\0'; /* Bunu yapinca cat /proc/mytaskinfo diyince tüm processler, echo diyince sadece ilgili olanlar geliyor. Sifirliyoruz aslinda */
    return len; /* the number of bytes copied */
}

ssize_t my_write(struct file *file, const char __user *usr_buf, size_t size, loff_t *offset)
{	
	char *buf = kmalloc(size + 1, GFP_KERNEL);
	/* copies user space usr_buf to kernel buffer */

	if (copy_from_user(buf, usr_buf, size))
	{
		printk(KERN_INFO "Error copying from user\n");
		return -EFAULT;
	}

	if (size > 0)
    {
        filter_state = buf[0];
        printk(KERN_INFO "Filter state set to: %c\n", filter_state);
    }

	/* *offset += size; yine offseti bazi durumlarda set etmeniz vs gerekebilir, user tekrar yazdiginda fd+offsete yazar*/
	buf[size] = '\0';
	
	printk(KERN_INFO "the value of kernel buf: %s", buf);
	kfree(buf);
	return size;
}

int my_open(struct inode *inode, struct file *file)
{
	struct my_data *my_data = kmalloc(sizeof(struct my_data) * MYBUF_SIZE, GFP_KERNEL);

	if (!my_data) {
        pr_err("Memory allocation for my_data failed\n");
        return -ENOMEM;
    }

    my_data->buf = NULL;
    my_data->size = 0;
	
	/* validate access to data */
	file->private_data = my_data;

	return 0;
}

int my_release(struct inode *inode, struct file *file)
{
	/*free all memories*/
	struct my_data *my_data = file->private_data;
	kfree(my_data->buf);
	kfree(my_data);
	
	printk(KERN_INFO "my_release() for /proc/%s \n", PROCF_NAME);
	return 0;
}

const struct proc_ops my_ops = {
	.proc_read = my_read,
	.proc_write = my_write,
	.proc_open = my_open,
	.proc_release = my_release,
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
MODULE_AUTHOR("umutcan");
