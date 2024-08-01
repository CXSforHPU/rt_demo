#include <make_data.h>
#include <rtthread.h>
#include <string.h>
#include <webclient.h>
#include <cJSON.h>

#define THREAD_SIZE 10240
#define THREAD_TIME_SLICE 5
#define THREAD_PRI 25

#define POST_RESP_BUFSZ 1024
#define POST_HEADER_BUFSZ 1024

#define POST_LCTXHY "https://luckycola.com.cn/hunyuan/txhy"

static rt_thread_t gpt_thread = RT_NULL;

/* send HTTP POST request by common request interface, it used to receive longer data */
static int webclient_post_comm(const char *uri, const void *post_data, size_t data_len)
{
    struct webclient_session* session = RT_NULL;
    unsigned char *buffer = RT_NULL;
    int index, ret = 0;
    int bytes_read, resp_status;

    buffer = (unsigned char *) web_malloc(POST_RESP_BUFSZ);
    if (buffer == RT_NULL)
    {
        rt_kprintf("no memory for receive response buffer.\n");
        ret = -RT_ENOMEM;
        goto __exit;
    }

    /* create webclient session and set header response size */
    session = webclient_session_create(POST_HEADER_BUFSZ);
    if (session == RT_NULL)
    {
        ret = -RT_ENOMEM;
        goto __exit;
    }

    /* build header for upload */
    webclient_header_fields_add(session, "Content-Length: %d\r\n", strlen(post_data));
    webclient_header_fields_add(session, "Content-Type:application/json\r\n");

    /* send POST request by default header */
    if ((resp_status = webclient_post(session, uri, post_data, data_len)) != 200)
    {
        rt_kprintf("webclient POST request failed, response(%d) error.\n", resp_status);
        ret = -RT_ERROR;
        goto __exit;
    }

    rt_kprintf("webclient post response data: \n");
    do
    {
        bytes_read = webclient_read(session, buffer, POST_RESP_BUFSZ);
        if (bytes_read <= 0)
        {
            break;
        }

        for (index = 0; index < bytes_read; index++)
        {
            rt_kprintf("%c", buffer[index]);
        }
    } while (1);

    rt_kprintf("\n");

__exit:
    if (session)
    {
        webclient_close(session);
    }

    if (buffer)
    {
        web_free(buffer);
    }

    return ret;
}

/* send HTTP POST request by simplify request interface, it used to received shorter data */
static int webclient_post_smpl(const char *uri, const char *post_data, size_t data_len)
{
    char *response = RT_NULL;
    char *header = RT_NULL;
    size_t resp_len = 0;
    int index = 0;

    webclient_request_header_add(&header, "Content-Length: %d\r\n", strlen(post_data));
    webclient_request_header_add(&header, "Content-Type: application/octet-stream\r\n");

    if (webclient_request(uri, header, post_data, data_len, (void **)&response, &resp_len) < 0)
    {
        rt_kprintf("webclient send post request failed.");
        web_free(header);
        return -RT_ERROR;
    }

    rt_kprintf("webclient send post request by simplify request interface.\n");
    rt_kprintf("webclient post response data: \n");
    for (index = 0; index < resp_len; index++)
    {
        rt_kprintf("%c", response[index]);
    }
    rt_kprintf("\n");

    if (header)
    {
        web_free(header);
    }

    if (response)
    {
        web_free(response);
    }

    return 0;
}

void gpt(void *param)
{
    char *uri = RT_NULL;
    cJSON *send_data = RT_NULL;
    char *str = RT_NULL;
    send_data = init_lctxhy_json(APP_KEY, UID, "你好", ISLONG_YES);
    str = cJSON_PrintUnformatted(send_data);
    
    uri = web_strdup(POST_LCTXHY);

    webclient_post_comm(uri, str,rt_strlen(str));

    /* 释放JSON数据 */
    cJSON_Delete(send_data);
    web_free(uri);
    web_free(str);
}

void gpt_test()
{
    gpt_thread = rt_thread_create(
        "gpt",
        gpt,
        RT_NULL,
        THREAD_SIZE,
        THREAD_PRI,
        THREAD_TIME_SLICE
    );

    if (gpt_thread != RT_NULL)
    {
        rt_thread_startup(gpt_thread);
    }
    else
    {
        rt_kprintf("线程创建失败！\n");
    }
}

MSH_CMD_EXPORT(gpt_test, 测试连接回答);
