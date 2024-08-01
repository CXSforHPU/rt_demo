/*
AIGC

方法：
    发送user 
        返回 assitant

    加载 以往对话内容
        返回 对话数据

*/


/*
https://luckycola.com.cn/public/docs/shares/api/hunyuan.html


发送数据
{
   "ques": "hello",
   // 官网-个人中心-Appkey获取
   "appKey": "643************c3",
   // 官网-个人中心-用户ID
   "uid": "y*************6",
   // 是否支持上下文 值1表示支持,0表示不支持
   "isLongChat": 0
}
接受数据
{
	// 	成功状态码
	"code": 0,
	// 	成功提示
	"msg": "AI接口返回成功",
	"data": {
		"result": {
			"Role": "assistant",
			// 	AI回答结果
			"Content": "！有什么我可以帮您的吗？",
			"originResult": {
				"Note": "以上内容为AI生成，不代表开发者立场，请勿删除或修改本标记",
				"Choices": [
					{
						"Role": "assistant",
						"Content": "！有什么我可以帮您的吗？"
					}
				],
				"Created": 1702062119,
				"Id": "32080b85-7e7c-4628-a36f-da588caabb4d",
				"Usage": {
					"PromptTokens": 1,
					"CompletionTokens": 8,
					"TotalTokens": 9
				}
			},
			"tips": "当前是json模式，当前返回数据可自行响应给客户端"
		},
		// 当前是否是上下文对话模式,1表示是上下文模式,0为非上下文模式
		"longChat": 0
	}
}
*/


