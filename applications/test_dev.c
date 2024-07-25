#include <rtthread.h>
#include <rtdevice.h>


static int rt_device_test_init(void)
{
    rt_device_t test_dev = rt_device_create(RT_Device_Class_Char,0);

    rt_device_register(test_dev,"test_dev",RT_DEVICE_FLAG_RDWR);

    return RT_EOK;
}


MSH_CMD_EXPORT(rt_device_test_init, 驱动初始化)