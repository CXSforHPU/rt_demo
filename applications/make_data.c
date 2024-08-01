#include <cJSON.h>
#include <rtthread.h>
#include <ulog.h>

#define DBG_TAG    "make_json.app"
#define DBG_LVL    DBG_INFO

#define ISLONG_YES  1
#define ISLONG_NO   0
#define APP_KEY     "657976893302bfa20b9f2b56"
#define UID         "NtoAW31702458883264reDjRnIakr"

char* send_data =
"{                                          \
   \"ques\": \"hello\",                     \
   \"appKey\": \"643************c3\",       \
   \"uid\": \"y*************6\",            \
   \"isLongChat\": 0                        \
}";

char* recv_data =
"{                                                                                          \
	\"code\": 0,                                                                            \
	\"msg\": \"AI接口返回成功\",                                                                \
	\"data\": {                                                                                 \
		\"result\": {                                                                           \
			\"Role\": \"assistant\",                                                            \
			\"Content\": \"！有什么我可以帮您的吗？\",                                              \
			\"originResult\": {                                                                     \
				\"Note\": \"以上内容为AI生成，不代表开发者立场，请勿删除或修改本标记\",                   \
				\"Choices\": [                                                                      \
					{                                                                                       \
						\"Role\": \"assistant\",                                                    \
						\"Content\": \"！有什么我可以帮您的吗？\"                                   \
					}                                                                               \
				],                                                                                          \
				\"Created\": 1702062119,\
				\"Id\": \"32080b85-7e7c-4628-a36f-da588caabb4d\",\
				\"Usage\": {\
					\"PromptTokens\": 1,\
					\"CompletionTokens\": 8,\
					\"TotalTokens\": 9\
				}\
			},\
			\"tips\": \"当前是json模式，当前返回数据可自行响应给客户端\"                                \
		},\
		\"longChat\": 0\
	}\
}";




cJSON * init_lctxhy_json(char* _APP_KEY,char* _UID,char* _ques,int _isLong)
{
    cJSON *root = RT_NULL;
    root = cJSON_CreateObject();

    cJSON_AddStringToObject(root,"appKey",_APP_KEY);
    cJSON_AddStringToObject(root,"uid",_UID);
    cJSON_AddStringToObject(root,"ques",_ques);
    cJSON_AddNumberToObject(root,"isLongChat",_isLong);

    return root;
}


char* get_data_lctxhy(char* message)
{
    cJSON* root = RT_NULL;
    cJSON* code = RT_NULL;
    cJSON* result = RT_NULL;
    cJSON* data = RT_NULL;
    cJSON* Content = RT_NULL;
    char * str = RT_NULL; 
    char * assitant = RT_NULL; 
    root = cJSON_Parse(message);

    code = cJSON_GetObjectItem(root,"code");
    if(cJSON_GetNumberValue(code)!=0)
    {
        str = cJSON_Print(root);
        LOG_I("返回结果不符合");
    }

    data = cJSON_GetObjectItem(root,"data");
    result = cJSON_GetObjectItem(data,"result");
    Content = cJSON_GetObjectItem(result,"Content");
    rt_strcpy(assitant,cJSON_GetStringValue(Content));
    rt_kprintf("assitant:\n%s\n",cJSON_GetStringValue(Content));
    cJSON_Delete(root);
    return assitant;
}



void gpt_json_smaple(){

    cJSON* send_data = RT_NULL;
    char* str=RT_NULL;
    send_data = init_lctxhy_json(APP_KEY,UID,"你好",ISLONG_YES);

    str = cJSON_Print(send_data);

    rt_kprintf("%s",str);


    str = get_data_lctxhy(recv_data);
    LOG_I("assitant:\n%s",str);
}


MSH_CMD_EXPORT(gpt_json_smaple,json测试);
