#include <rtthread.h>
#include <sys/socket.h> /* 使用BSD socket，需要包含socket.h头文件 */
#include <netdb.h>
#include <string.h>
#include <finsh.h>

#define BUFSZ 1024
#define THREAD_tcp_SIZE 10240
#define THREAD_PRI  25
#define THREAD_TIMSLICE 5

static const char sample_message[] = "你好";

static rt_sem_t lock_recv_get = RT_NULL;

// 线程
static rt_thread_t tcp_thread = RT_NULL;
static rt_thread_t chat_thread = RT_NULL;
// 邮箱
static rt_mailbox_t user = RT_NULL;
static rt_mailbox_t ip_port = RT_NULL;

typedef struct info
{
    char info_uri[256]; // 调整为固定大小数组以避免指针问题
    int info_port;
} info, *info_t;

static void gpt_tcpclient(void *params)
{
    int ret;
    char *recv_data;
    struct hostent *host;
    int sock, bytes_received;
    struct sockaddr_in server_addr;
    char *url;
    int port;
    info_t ip = (info_t)rt_malloc(sizeof(info));

    // 创建信号量
    if (lock_recv_get == NULL)
    {
        lock_recv_get = rt_sem_create("lock", 0, RT_IPC_FLAG_PRIO);
    }

    if (rt_mb_recv(ip_port, (rt_uint32_t)&ip, RT_WAITING_FOREVER) == RT_EOK)
    {
        url = ip->info_uri;
        port = ip->info_port;
    }

    // 通过函数入口参数url获得host地址（如果是域名，会做域名解析）
    host = gethostbyname(url);

    // 分配用于存放接收数据的缓冲
    recv_data = rt_malloc(BUFSZ);
    if (recv_data == RT_NULL)
    {
        rt_kprintf("No memory\n");
        return;
    }

    // 创建一个socket，类型是SOCKET_STREAM，TCP类型
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        rt_kprintf("Socket error\n");
        rt_free(recv_data);
        return;
    }

    // 初始化预连接的服务端地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    // 连接到服务端
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        rt_kprintf("Connect fail!\n");
        closesocket(sock);
        rt_free(recv_data);
        return;
    }
    else
    {
        rt_kprintf("Connect successful\n");
    }

    while (1)
    {
        char *data;
        rt_sem_take(lock_recv_get, RT_WAITING_FOREVER);

        rt_mb_recv(user, (rt_uint32_t)&data, RT_WAITING_FOREVER);

        // 发送数据到sock连接
        ret = send(sock, data, strlen(data), 0);
        if (ret < 0)
        {
            closesocket(sock);
            rt_kprintf("\nsend error, close the socket.\r\n");
            rt_free(recv_data);
            break;
        }
        // 从sock连接中接收最大BUFSZ - 1字节数据
        bytes_received = recv(sock, recv_data, BUFSZ - 1, 0);
        if (bytes_received < 0)
        {
            closesocket(sock);
            rt_kprintf("\nreceived error, close the socket.\r\n");
            rt_free(recv_data);
            break;
        }
        // 有接收到数据，把末端清零
        recv_data[bytes_received] = '\0';

        // 在控制终端显示收到的数据
        rt_kprintf("\nReceived data = %s ", recv_data);
    }
    return;
}

void chat(int argc, char **argv)
{
    char *data = rt_malloc(strlen(argv[1]) + 1); // 动态分配内存存储参数
    if (data == NULL)
    {
        rt_kprintf("No memory\n");
        return;
    }
    rt_strcpy(data, argv[1]);
    rt_mb_send(user, (rt_uint32_t)data);
    rt_sem_release(lock_recv_get);
}

void chat_client(int argc, char **argv)
{
    info_t IpPort = rt_malloc(sizeof(info)); // 动态分配内存存储IP和端口信息
    if (IpPort == NULL)
    {
        rt_kprintf("No memory\n");
        return;
    }

    rt_strncpy(IpPort->info_uri, argv[1], sizeof(IpPort->info_uri) - 1);
    IpPort->info_uri[sizeof(IpPort->info_uri) - 1] = '\0'; // 确保字符串以空字符结尾
    IpPort->info_port = strtoul(argv[2],0,10);

    if (user == RT_NULL)
    {
        user = rt_mb_create("user", 1024, RT_IPC_FLAG_FIFO);
    }
    if (ip_port == RT_NULL)
    {
        ip_port = rt_mb_create("ip_port", 128, RT_IPC_FLAG_FIFO);
    }
    if (tcp_thread == RT_NULL)
    {
        tcp_thread = rt_thread_create("tcp", gpt_tcpclient, RT_NULL, THREAD_tcp_SIZE, THREAD_PRI, THREAD_TIMSLICE);
        rt_thread_startup(tcp_thread);
    }

    rt_mb_send(ip_port, (rt_uint32_t)IpPort);
}

MSH_CMD_EXPORT(chat_client, 设置ip 端口);
MSH_CMD_EXPORT(chat, 交流);
