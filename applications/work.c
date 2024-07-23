#include <rtthread.h>

// 定义宏 优先级，栈大小，时间片
#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

static rt_thread_t dad = RT_NULL;
static rt_thread_t mom = RT_NULL;
static rt_thread_t daughter = RT_NULL;
static rt_thread_t son = RT_NULL;

static rt_mutex_t dynamic_mutex = RT_NULL;
struct rt_semaphore apple;
struct rt_semaphore orange;
struct rt_semaphore plate;

static rt_uint8_t num_apple = 0;
static rt_uint8_t num_orange = 0;

void dad_(void *parameter) {
    while (1) {
        rt_sem_take(&plate, RT_WAITING_FOREVER);
        rt_mutex_take(dynamic_mutex, RT_WAITING_FOREVER);
        num_apple++;
        rt_kprintf("加入苹果 %d\n", num_apple);
        rt_mutex_release(dynamic_mutex);
        rt_sem_release(&apple);
        rt_thread_mdelay(100); // 模拟生产时间
    }
}

void mom_(void *parameter) {
    while (1) {
        rt_sem_take(&plate, RT_WAITING_FOREVER);
        rt_mutex_take(dynamic_mutex, RT_WAITING_FOREVER);
        num_orange++;
        rt_kprintf("加入橘子 %d\n", num_orange);
        rt_mutex_release(dynamic_mutex);
        rt_sem_release(&orange);
        rt_thread_mdelay(100); // 模拟生产时间
    }
}

void daughter_(void *parameter) {
    while (1) {
        rt_sem_take(&apple, RT_WAITING_FOREVER);
        rt_mutex_take(dynamic_mutex, RT_WAITING_FOREVER);
        num_apple--;
        rt_kprintf("吃掉苹果 %d\n", num_apple);
        rt_mutex_release(dynamic_mutex);
        rt_sem_release(&plate);
        rt_thread_mdelay(100); // 模拟消费时间
    }
}

void son_(void *parameter) {
    while (1) {
        rt_sem_take(&orange, RT_WAITING_FOREVER);
        rt_mutex_take(dynamic_mutex, RT_WAITING_FOREVER);
        num_orange--;
        rt_kprintf("吃掉橘子 %d\n", num_orange);
        rt_mutex_release(dynamic_mutex);
        rt_sem_release(&plate);
        rt_thread_mdelay(100); // 模拟消费时间
    }
}

int start() {
    // 创建动态互斥量
    dynamic_mutex = rt_mutex_create("dmutex", RT_IPC_FLAG_PRIO);
    
    // 初始化信号量
    rt_sem_init(&plate, "plate", 1, RT_IPC_FLAG_PRIO);
    rt_sem_init(&apple, "apple", 0, RT_IPC_FLAG_PRIO);
    rt_sem_init(&orange, "orange", 0, RT_IPC_FLAG_PRIO);

    // 创建线程
    dad = rt_thread_create("dad", dad_, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    if (dad != RT_NULL) rt_thread_startup(dad);
    
    mom = rt_thread_create("mom", mom_, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    if (mom != RT_NULL) rt_thread_startup(mom);
    
    daughter = rt_thread_create("daughter", daughter_, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (daughter != RT_NULL) rt_thread_startup(daughter);
    
    son = rt_thread_create("son", son_, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (son != RT_NULL) rt_thread_startup(son);

    return 0;
}

MSH_CMD_EXPORT(start, 经典同步问题);
