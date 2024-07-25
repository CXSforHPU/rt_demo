#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <icm20608.h>
#include <ulog.h>
#include <drv_gpio.h>

#define LOG_TAG         "pin.irq"
#define LOG_LVL         LOG_LVL_DBG

#define KEY_UP          GET_PIN(C,5)
#define KEY_DOWN        GET_PIN(C,1)
#define KEY_LEFT        GET_PIN(C,0)
#define KEY_RIGHT       GET_PIN(C,4)

static rt_sem_t key_sem = RT_NULL;

/* ICM20608传感器线程入口函数 */
static void icm_thread_entry(void *param)
{
    icm20608_device_t dev = RT_NULL;
    const char *i2c_bus_name = "i2c2";
    int count = 0;
    rt_err_t result;

    /* 初始化ICM20608传感器 */
    dev = icm20608_init(i2c_bus_name);
    if (dev == RT_NULL)
    {
        LOG_E("传感器初始化失败");
        return;
    }
    else
    {
        LOG_D("传感器初始化成功");
    }

    /* 进行ICM20608零值校准：采样10次，取平均值作为零值 */
    result = icm20608_calib_level(dev, 10);
    if (result == RT_EOK)
    {
        LOG_D("传感器校准成功");
        LOG_D("加速度计偏移量: X%6d  Y%6d  Z%6d", dev->accel_offset.x, dev->accel_offset.y, dev->accel_offset.z);
        LOG_D("陀螺仪偏移量 : X%6d  Y%6d  Z%6d", dev->gyro_offset.x, dev->gyro_offset.y, dev->gyro_offset.z);
    }
    else
    {
        LOG_E("传感器校准失败");
        icm20608_deinit(dev);
        return;
    }

    /* 循环读取传感器数据 */
    while (count++ < 10)
    {
        rt_int16_t accel_x, accel_y, accel_z;
        rt_int16_t gyros_x, gyros_y, gyros_z;

        /* 读取三轴加速度 */
        result = icm20608_get_accel(dev, &accel_x, &accel_y, &accel_z);
        if (result == RT_EOK)
        {
            LOG_D("当前加速度计数据: accel_x%6d, accel_y%6d, accel_z%6d", accel_x, accel_y, accel_z);
        }
        else
        {
            LOG_E("传感器读取失败");
            break;
        }

        /* 读取三轴陀螺仪 */
        result = icm20608_get_gyro(dev, &gyros_x, &gyros_y, &gyros_z);
        if (result == RT_EOK)
        {
            LOG_D("当前陀螺仪数据: gyros_x%6d, gyros_y%6d, gyros_z%6d", gyros_x, gyros_y, gyros_z);
        }
        else
        {
            LOG_E("传感器读取失败");
            break;
        }
        rt_thread_mdelay(100);
    }

    /* 传感器使用完毕，进行反初始化 */
    icm20608_deinit(dev);
}

/* 按键左回调函数 */
void key_left_callback(void *args)
{
    rt_sem_release(key_sem);
    int value = rt_pin_read(KEY_LEFT);
    LOG_I("按键左被按下! %d", value);
}

/* 按键右回调函数 */
void key_right_callback(void *args)
{
    int value = rt_pin_read(KEY_RIGHT);
    LOG_I("按键右被按下! %d", value);
}

/* 按键上回调函数 */
void key_up_callback(void *args)
{
    int value = rt_pin_read(KEY_UP);
    LOG_I("按键上被按下! %d", value);
}

/* 按键下回调函数 */
void key_down_callback(void *args)
{
    int value = rt_pin_read(KEY_DOWN);
    LOG_I("按键下被按下! %d", value);
}

/* 按键处理线程入口函数 */
static void key_thread_entry(void *param)
{
    while (1)
    {
        /* 等待按键信号量 */
        rt_sem_take(key_sem, RT_WAITING_FOREVER);
        /* 创建并启动ICM20608传感器线程 */
        rt_thread_t icm_thread = rt_thread_create("icm", icm_thread_entry, RT_NULL, 2048, 20, 50);
        if (icm_thread != RT_NULL)
        {
            rt_thread_startup(icm_thread);
        }
        else
        {
            LOG_E("传感器线程创建失败");
        }
    }
}

/* 引脚中断示例 */
static int rt_pin_irq_example(void)
{
    /* 创建按键信号量 */
    key_sem = rt_sem_create("key_sem", 0, RT_IPC_FLAG_FIFO);
    if (key_sem == RT_NULL)
    {
        LOG_E("按键信号量创建失败");
        return -RT_ERROR;
    }

    /* 创建按键处理线程 */
    rt_thread_t key_thread = rt_thread_create("key", key_thread_entry, RT_NULL, 2048, 20, 50);
    if (key_thread != RT_NULL)
    {
        rt_thread_startup(key_thread);
    }
    else
    {
        LOG_E("按键处理线程创建失败");
        return -RT_ERROR;
    }

    /* 配置按键引脚为输入并使能上拉 */
    rt_pin_mode(KEY_UP, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY_DOWN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY_RIGHT, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY_LEFT, PIN_MODE_INPUT_PULLUP);

    /* 附加中断回调函数 */
    rt_pin_attach_irq(KEY_UP, PIN_IRQ_MODE_FALLING, key_up_callback, RT_NULL);
    rt_pin_attach_irq(KEY_DOWN, PIN_IRQ_MODE_FALLING, key_down_callback, RT_NULL);
    rt_pin_attach_irq(KEY_RIGHT, PIN_IRQ_MODE_FALLING, key_right_callback, RT_NULL);
    rt_pin_attach_irq(KEY_LEFT, PIN_IRQ_MODE_FALLING, key_left_callback, RT_NULL);

    /* 使能中断 */
    rt_pin_irq_enable(KEY_UP, PIN_IRQ_ENABLE);
    rt_pin_irq_enable(KEY_DOWN, PIN_IRQ_ENABLE);
    rt_pin_irq_enable(KEY_RIGHT, PIN_IRQ_ENABLE);
    rt_pin_irq_enable(KEY_LEFT, PIN_IRQ_ENABLE);

    return RT_EOK;
}

/* 导出命令到 msh */
MSH_CMD_EXPORT(rt_pin_irq_example, gpio示例);
