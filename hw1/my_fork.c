#include<linux/module.h>
#include<linux/sched.h>
#include<linux/pid.h>
#include<linux/kthread.h>
MODULE_LICENSE("GPL");

int my_monitor(void *argc){
	do_fork(SIGCHLD,NULL,NULL,NULL,0);

		printk("<0>KEKEKE\n");

}

static int __init kernel_object_test_init(void)
{
	int result, result1;
	struct pid *kpid;
	struct task_struct *task;
	struct sched_param param;
	
	task = kthread_create(my_monitor,NULL,"my_fork");
	wake_up_process(task);
	return 0;

}

static void __exit kernel_object_test_exit(void)
{
	printk("<0>Remove the module\n");
}

module_init(kernel_object_test_init);
module_exit(kernel_object_test_exit);
