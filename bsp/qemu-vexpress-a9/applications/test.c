#include <rtthread.h>

struct test_struct
{
    rt_uint8_t a;
    rt_uint8_t b;
    rt_uint16_t c;
    rt_uint8_t arr[10];
};
typedef struct test_struct *test_struct_t;

struct test_struct test_data;
test_struct_t test_data2;
rt_uint8_t buffer_4[512];
rt_thread_t thread_1,thread_2;

static void thread_1_entry(void *params)
{
    rt_uint8_t *buffer_1 = rt_calloc(1,2048);
    rt_uint8_t buffer_2[512];
    static rt_uint8_t buffer_3[512];
    rt_uint16_t i = 0;
    while(1)
    {
        test_data.a = buffer_4[i] = i;                
        buffer_4[i] = buffer_4[i] + 1;    
        rt_thread_mdelay(1);           
        test_data.b = buffer_4[i];
        test_data.c = test_data.a + test_data.b;
              
        rt_thread_mdelay(1);
        i++;
        if(i>=512) i = 0; 
    }
}

static void thread_2_entry(void *params)
{
    rt_uint16_t i = 0;
    while(1)        
    {
        if(test_data.a + test_data.b != test_data.c)        
        {     
            rt_kprintf("test_data.a = %d, test_data.b = %d, test_data.c = %d\n", test_data.a, test_data.b, test_data.c);
            break;
        }
        
        i++;
        if(i>=512) i = 0;        
        
        rt_thread_mdelay(1);
    }
}

void test_task(void)
{
    thread_1 = rt_thread_create("thread_1", thread_1_entry,
                                                                                    RT_NULL,1024,13,5);
    if(thread_1 != RT_NULL)
            rt_thread_startup(thread_1);
            
    thread_2 = rt_thread_create("thread_2", thread_2_entry,
                                                                                    RT_NULL,1024,13,5);
    if(thread_2 != RT_NULL)
            rt_thread_startup(thread_2);  

    test_data2 = &test_data;
    rt_kprintf("sizeof(struct test_struct)=%d\n", sizeof(struct test_struct));              
    rt_kprintf("sizeof(test_data)=%d\n", sizeof(test_data));
    rt_kprintf("sizeof(test_data.arr)=%d\n", sizeof(test_data.arr));
    rt_kprintf("sizeof(test_data2)=%d\n", sizeof(test_data2));
    rt_kprintf("sizeof(*test_data2)=%d\n", sizeof(*test_data2));
}

static void delt1(void)
{
    rt_thread_delete(thread_1);
}
MSH_CMD_EXPORT(delt1, del thread1);

static void delt2(void)
{
    rt_thread_delete(thread_2);
}
MSH_CMD_EXPORT(delt2, del thread2);