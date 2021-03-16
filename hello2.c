#include <linux/moduleparam.h>
#include <linux/buffer_head.h>
#include <linux/processor.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/segment.h>
#include <linux/init.h>
#include <linux/fs.h>

#define __KERNEL__       
#define MODULE       


struct myfile
{
    struct file *f;
    mm_segment_t fs;
    loff_t pos;
};

struct myfile *open_file_for_read(char *filename)
{
    struct myfile *read = kmalloc(sizeof(struct myfile), GFP_KERNEL);

    read->fs = get_fs();
    read->f = filp_open(filename, 0, 0);
    read->pos = 0;

    if ((read->f)==0)
    {
        printk("Error opening file\n");
        return NULL;

    }
    return read;
}

volatile int _read_from_file_until(struct myfile *mf, char *buf, unsigned long vlen, char c)
{   
    int return1,x1;
    return1=0;
    x1=0;

    set_fs(KERNEL_DS);

   while (x1 != vlen){
        return1 = vfs_read(mf->f, buf+x1, 1, &(mf->pos)); 
        if ((buf[x1]==c)) {
            x1++;
            buf[x1]='\0';
            break;
        }

        x1++;
    }

    set_fs(mf->fs);
    return return1;
} 


void close_file(struct myfile *mf)
{
    filp_close(mf->f, NULL);
}

static int initialize(void)
{
    char *fileName = "/boot/System.map-4.15.0-20-generic";
    char c = '\n';
    struct myfile *myfile_des;

    char *buffer = kmalloc(2048, GFP_KERNEL);
    memset(buffer, 0, 2048);
    myfile_des = open_file_for_read(fileName);

    void **sys_call_table;

    char *buffer2=kmalloc(sizeof(char) * 256, GFP_KERNEL);
    memset(buffer2, 0, 2048);


    while(true){

    _read_from_file_until(myfile_des, buffer, 1024, c);
    if (strstr(buffer, "sys_call_table")) break;

    }

    int x2=0;
    for(x2=0; x2<16; x2++){
		
        buffer2[x2]= buffer[x2];

    }

buffer2[16]='\0';
printk("buffer2: %s \n", buffer2);


unsigned long ul;

sscanf(buffer2, "%lx", &ul);
sys_call_table = (void *)ul;
printk(KERN_INFO "fork_sys_call: %px\n", sys_call_table[__NR_fork]);

    close_file(myfile_des);
    kfree(buffer);

    return 0;
}

static void cleanup(void)
{
    printk("cleaned up\n");
}

module_init(initialize);
module_exit(cleanup);

MODULE_AUTHOR("Seif Rady");
MODULE_DESCRIPTION("Simple Kernel Module");
MODULE_LICENSE("GPL");
