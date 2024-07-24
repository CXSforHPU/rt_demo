#include <rtthread.h>

#define THREAD_STACK 1024
#define THREAD_PRIORITY 10
#define THREAD_TIMESLICE 5

// 消息队列控制块
static struct rt_messagequeue mq;
// 信息模块
static char mb_str[] = "你好呀";

// 消息队列中用到的内存池
static char msg_pool[2048];

// 信号量设置
static struct rt_semaphore rw;
static struct rt_mutex mutex;

static void writer(void *param)
{
    while (1)
    {
        rt_sem_take(&rw, RT_WAITING_FOREVER);
        // 写操作
        rt_mq_send(&mq, mb_str, sizeof(mb_str));
        rt_kprintf("我在写\t%s\n", mb_str);
        rt_sem_release(&rw);

        rt_thread_mdelay(100);
    }
}

static void reader(void *param)
{
    char str[128]; // 使用足够大的缓冲区来接收消息
    while (1)
    {
        if (rt_mq_recv(&mq, str, sizeof(str), RT_WAITING_FOREVER) == RT_EOK)
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

int writer_reader_msgq()
{
    rt_mq_init(&mq, "mq", msg_pool, sizeof(mb_str), sizeof(msg_pool), RT_IPC_FLAG_FIFO);
    
    // 初始化信号量和互斥锁
    rt_sem_init(&rw, "rw", 1, RT_IPC_FLAG_FIFO);
    rt_mutex_init(&mutex, "mutex", RT_IPC_FLAG_FIFO);

    rt_thread_t writer_ = rt_thread_create("writer", writer, RT_NULL, THREAD_STACK, THREAD_PRIORITY, THREAD_TIMESLICE);
    rt_thread_t reader_ = rt_thread_create("reader", reader, RT_NULL, THREAD_STACK, THREAD_PRIORITY, THREAD_TIMESLICE);

    if (writer_ != RT_NULL) rt_thread_startup(writer_);
    if (reader_ != RT_NULL) rt_thread_startup(reader_);
    
    return 1;
}

MSH_CMD_EXPORT(writer_reader_msgq, 消息队列读写者);
