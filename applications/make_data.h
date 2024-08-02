#include<cJSON.h>
/*
定义发送数据的结构体

格式化发送的数据

解析接受到的数据

将json数据保存成为文件存储
*/
#define ISLONG_YES  1
#define ISLONG_NO   0
#define APP_KEY     "657976893302bfa20b9f2b56"
#define UID         "NtoAW31702458883264reDjRnIakr"


struct ip_port
{
    char *url;
    int port;
};






cJSON * init_lctxhy_json(char* _APP_KEY,char* _UID,char* _ques,int _isLong);
char* get_data_lctxhy(char* message);

