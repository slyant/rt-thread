#include <rtthread.h>
#include <webclient.h>
#include "url_code.h"
#include <string.h>
#include <cJSON_util.h>

#define GET_HEADER_BUFSZ	1024
#define GET_RESP_BUFSZ		1024
#define GET_RESP_CONTENTSZ	4096
#define URL_LEN_MAX			1024

#define GET_LOCAL_URI		"https://www.apiopen.top/weatherApi?city=%s"
#define LOCAL_CITY			"%E6%AD%A6%E6%B1%89"	//urlencode("Îäºº")

static void weather_data_parse(char* data)
{
   cJSON *root = RT_NULL, *object = RT_NULL, *list = RT_NULL, *item = RT_NULL, *weatheritem = RT_NULL;
	int index, list_size = 0;
   root = cJSON_Parse((const char *)data);
   if (!root)
   {
       rt_kprintf("No memory for cJSON root!\n");
       return;
   }
   item = cJSON_GetObjectItem(root, "code");   
   if(item->valueint != 200)
   {
   		item = cJSON_GetObjectItem(root, "msg");
   		rt_kprintf("\n%s", item->valuestring);
   		goto __EXIT;
   }
   object = cJSON_GetObjectItem(root, "data");
   item = cJSON_GetObjectItem(object, "city");
   rt_kprintf("\ncity:\t%s", item->valuestring);
   list = cJSON_GetObjectItem(object, "forecast");
   list_size = cJSON_GetArraySize(list);
   for(index=0;index<list_size;index++)
   {
	   weatheritem = cJSON_GetArrayItem(list, index);	   
	   rt_kprintf("\ndate:\t%s", cJSON_item_get_string(weatheritem, "date"));
	   rt_kprintf("\ntype:\t%s", cJSON_item_get_string(weatheritem, "type"));
	   rt_kprintf("\nwendu:\t%s-%s", cJSON_item_get_string(weatheritem, "low"), cJSON_item_get_string(weatheritem, "high"));
	   rt_kprintf("\nfengxiang:%s %s", cJSON_item_get_string(weatheritem, "fengxiang"), cJSON_item_get_string(weatheritem, "fengli"));
	   rt_kprintf("\n");
   }   
	rt_kprintf("\n");
__EXIT:
   if (root != RT_NULL)
       cJSON_Delete(root);	
}

int get_weather(int argc, char **argv)
{
    struct webclient_session* session = RT_NULL;
    unsigned char *buffer = RT_NULL;
    char *URI = RT_NULL;
    int index, ret = 0;
    int bytes_read, resp_status;
    int content_length = -1;
	char *city_name = rt_calloc(1,255);
	if(argc == 1)
	{
		strcpy(city_name, LOCAL_CITY);
	}
	else if (argc == 2)
    {
        strcpy(city_name, argv[1]);
		urlencode(city_name);//¶ÔÖÐÎÄ±àÂë
    }
    else if(argc > 2)
    {
        rt_kprintf("webclient_get_test [URI]  - webclient GET request test.\n");
        return -1;
    }
	URI = rt_calloc(1, URL_LEN_MAX);	
	rt_snprintf(URI, URL_LEN_MAX, GET_LOCAL_URI, city_name);
    buffer = (unsigned char *) web_malloc(GET_RESP_CONTENTSZ);
    if (buffer == RT_NULL)
    {
        rt_kprintf("no memory for receive buffer.\n");
        ret = -RT_ENOMEM;
        goto __exit;
    }

    /* create webclient session and set header response size */
    session = webclient_session_create(GET_HEADER_BUFSZ);
    if (session == RT_NULL)
    {
        ret = -RT_ENOMEM;
        goto __exit;
    }

    /* send GET request by default header */
	rt_kprintf("send GET request to %s\n", URI);
    if ((resp_status = webclient_get(session, URI)) != 200)
    {
        rt_kprintf("webclient GET request failed, response(%d) error.\n", resp_status);
        ret = -RT_ERROR;
        goto __exit;
    }
    content_length = webclient_content_length_get(session);
    if (content_length < 0)
    {
        rt_kprintf("webclient GET request type is chunked.\n");
        do
        {
            bytes_read = webclient_read(session, buffer, GET_RESP_BUFSZ);
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
    }
    else
    {
		int content_pos = 0;
        do
        {
            bytes_read = webclient_read(session, buffer+content_pos, GET_RESP_BUFSZ);
            if (bytes_read <= 0)
            {
				break;
            }
            content_pos += bytes_read;
        } while (content_pos < content_length);
        weather_data_parse((char*)buffer);
    }

__exit:
    if (session)
    {
        webclient_close(session);
    }

    if (buffer)
    {
        web_free(buffer);
    }

    if (URI)
    {
        web_free(URI);
    }

    return ret;
}

#ifdef FINSH_USING_MSH
#include <finsh.h>
MSH_CMD_EXPORT_ALIAS(get_weather, wt, wt [URI]  webclient GET request test);
#endif /* FINSH_USING_MSH */
