#include <rtthread.h>
#include <rtdevice.h>
#include <ulog.h>
#include <drv_gpio.h>

#define LOG TAG "i2c.app"
#define LOG LVL "LOG_LVL_DBG"



void i2c_sample_signle_byte_write(void)
{
    struct rt_i2c_bus_device *i2c_bus;
    struct rt_i2c_msg msgs;
    rt_uint8_t buf[2];

    i2c_bus = (struct rt_i2c_bus_device*)rt_device_find("i2c2");

    buf[0] = 0x68;
    msgs.addr=0x68;
    msgs.flags=RT_I2C_WR;
    msgs.buf=buf;
    msgs.len=1;


    if(rt_i2c_transfer(i2c_bus,&msgs,1)==1)
    {
        LOG_I("success!");
    }

}

MSH_CMD_EXPORT(i2c_sample_signle_byte_write,单字节发送i2c)