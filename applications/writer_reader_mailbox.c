#include <rtthread.h>

#define THREAD_STACK 1024
#define THREAD_PRIORITY 10
#define THREAD_TIMESLICE 5

// 邮箱控制块
static struct rt_mailbox mb;
// 信息模块
static char mb_str[] = "你好呀";

// 邮箱的内存池
static char mb_pool[128];

// 信号量设置
static struct rt_semaphore rw;
static struct rt_mutex mutex;

static void writer(void *param)
{
    while (1)
    {
        rt_sem_take(&rw, RT_WAITING_FOREVER);
        // 写操作
        rt_mb_send(&mb, (rt_ubase_t)&mb_str);
        rt_kprintf("我在写\t%s\n",&mb_str);
        rt_sem_release(&rw);

        rt_thread_mdelay(100);
    }
}

static void reader(void *param)
{
    char *str;
    while (1)
    {
        if (rt_mb_recv(&mb, (rt_ubase_t *)&str, RT_WAITING_FOREVER) == RT_EOK)
        {
            rt_mutex_take(&mutex, RT_WAITING_FOREVER);

            // 读操作
            rt_kprintf("我在读\t%s\n", str);

            rt_mutex_release(&mutex);
        }
        else
        {
            rt_thread_mdelay(100);
        }
    }
}

int writer_reader()
{
    rt_mb_init(&mb, "mbt", mb_pool, sizeof(mb_pool) / sizeof(rt_ubase_t), RT_IPC_FLAG_FIFO);
    
    // 初始化信号量和互斥锁
    rt_sem_init(&rw, "rw", 1, RT_IPC_FLAG_FIFO);
    rt_mutex_init(&mutex, "mutex", RT_IPC_FLAG_FIFO);

    rt_thread_t writer_ = rt_thread_create("writer", writer, RT_NULL, THREAD_STACK, THREAD_PRIORITY, THREAD_TIMESLICE);
    rt_thread_t reader_ = rt_thread_create("reader", reader, RT_NULL, THREAD_STACK, THREAD_PRIORITY, THREAD_TIMESLICE);

    rt_thread_startup(writer_);
    rt_thread_startup(reader_);
    return 1;
}

MSH_CMD_EXPORT(writer_reader, 邮箱读写者);
