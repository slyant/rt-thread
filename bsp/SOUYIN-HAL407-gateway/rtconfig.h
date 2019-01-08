#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* Automatically generated file; DO NOT EDIT. */
/* RT-Thread Configuration */

/* RT-Thread Kernel */

#define RT_NAME_MAX 8
#define RT_ALIGN_SIZE 4
#define RT_THREAD_PRIORITY_32
#define RT_THREAD_PRIORITY_MAX 32
#define RT_TICK_PER_SECOND 1000
#define RT_USING_OVERFLOW_CHECK
#define IDLE_THREAD_STACK_SIZE 256
#define RT_USING_TIMER_SOFT
#define RT_TIMER_THREAD_PRIO 4
#define RT_TIMER_THREAD_STACK_SIZE 512
#define RT_DEBUG

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE

/* Memory Management */

#define RT_USING_MEMPOOL
#define RT_USING_MEMHEAP
#define RT_USING_SMALL_MEM
#define RT_USING_MEMTRACE
#define RT_USING_HEAP

/* Kernel Device Object */

#define RT_USING_DEVICE
#define RT_USING_INTERRUPT_INFO
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE 128
#define RT_CONSOLE_DEVICE_NAME "uart3"
#define RT_VER_NUM 0x40000

/* RT-Thread Components */

#define RT_USING_COMPONENTS_INIT
#define RT_USING_USER_MAIN
#define RT_MAIN_THREAD_STACK_SIZE 8192
#define RT_MAIN_THREAD_PRIORITY 10

/* C++ features */


/* Command shell */

#define RT_USING_FINSH
#define FINSH_THREAD_NAME "tshell"
#define FINSH_USING_HISTORY
#define FINSH_HISTORY_LINES 5
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION
#define FINSH_THREAD_PRIORITY 20
#define FINSH_THREAD_STACK_SIZE 10240
#define FINSH_CMD_SIZE 80
#define FINSH_USING_MSH
#define FINSH_USING_MSH_DEFAULT
#define FINSH_ARG_MAX 10

/* Device virtual file system */

#define RT_USING_DFS
#define DFS_USING_WORKDIR
#define DFS_FILESYSTEMS_MAX 5
#define DFS_FILESYSTEM_TYPES_MAX 5
#define DFS_FD_MAX 16
#define RT_USING_DFS_ELMFAT

/* elm-chan's FatFs, Generic FAT Filesystem Module */

#define RT_DFS_ELM_CODE_PAGE 437
#define RT_DFS_ELM_WORD_ACCESS
#define RT_DFS_ELM_USE_LFN_3
#define RT_DFS_ELM_USE_LFN 3
#define RT_DFS_ELM_MAX_LFN 255
#define RT_DFS_ELM_DRIVES 2
#define RT_DFS_ELM_MAX_SECTOR_SIZE 4096
#define RT_DFS_ELM_USE_ERASE
#define RT_DFS_ELM_REENTRANT
#define RT_USING_DFS_DEVFS
#define RT_USING_DFS_ROMFS

/* Device Drivers */

#define RT_USING_DEVICE_IPC
#define RT_PIPE_BUFSZ 512
#define RT_USING_SERIAL
#define RT_USING_I2C
#define RT_USING_I2C_BITOPS
#define RT_USING_PIN
#define RT_USING_SPI
#define RT_USING_SFUD
#define RT_SFUD_USING_SFDP
#define RT_SFUD_USING_FLASH_INFO_TABLE

/* Using WiFi */


/* Using USB */


/* POSIX layer and C standard library */

#define RT_USING_LIBC
#define RT_USING_POSIX

/* Network */

/* Socket abstraction layer */


/* light weight TCP/IP stack */


/* Modbus master and slave stack */


/* AT commands */


/* VBUS(Virtual Software BUS) */


/* Utilities */


/* RT-Thread online packages */

/* IoT - internet of things */

#define PKG_USING_NRF24L01_NETWORK
#define PKG_USING_NRF24L01_NETWORK_LATEST_VERSION
#define NRF_SPI_BUS_NAME "spi2"
#define NRF_SPI_DEV_NAME "spi20"
#define NRF_SPI_CS_PIN 56
#define NRF_CE_PIN 55
#define NRF_IRQ_PIN 51
#define NRF24L01_USING_GATEWAY
#define PKG_USING_CJSON
#define PKG_USING_CJSON_V102

/* Wi-Fi */

/* Marvell WiFi */


/* Wiced WiFi */


/* IoT Cloud */


/* security packages */

#define PKG_USING_TINYCRYPT
#define PKG_USING_TINYCRYPT_V100
#define TINY_CRYPT_MD5

/* language packages */


/* multimedia packages */


/* tools packages */


/* system packages */

#define PKG_USING_SQLITE

/* peripheral libraries and drivers */


/* miscellaneous packages */

#define PKG_USING_MULTIBUTTON
#define PKG_USING_MULTIBUTTON_LATEST_VERSION

/* MultiButton Options */


/* sample package */

/* samples: kernel and components samples */


/* example package: hello */

#define SOC_STM32F407VG
#define RT_USING_RNG
#define RT_HSE_VALUE 25000000
#define RT_HSE_HCLK 168000000
#define RT_USING_ICCARD
#define RT_USING_UART_LCD
#define RT_USING_UART_GPS

/* Serial Select */

#define RT_USING_UART1
#define RT_USING_UART2
#define RT_USING_UART3
#define RT_USING_UART5
#define RT_USING_UART6

/* SPI Select */

#define RT_USING_SPI1
#define RT_USING_SPI2
#define RT_USING_SPI3

/* IIC Device Config */

#define I2C_BUS_NAME "i2cbus"
#define RT_USING_PCF8563

/* ICCARD Config */

#define ICCARD_SPI_CS_PIN 33
#define ICCARD_SPI_BUS_NAME "spi1"
#define ICCARD_SPI_DEV_NAME "spi10"

/* UART LCD Config */

#define UART_LCD_UART_NAME "uart2"
#define UART_LCD_BAUD_RATE 115200

/* UART GPS Config */

#define UART_GPS_UART_NAME "uart1"
#define UART_GPS_BAUD_RATE 4800

/* SFUD Config */

#define SFUD_SPI_CS_PIN 88
#define SFUD_SPI_BUS_NAME "spi3"
#define SFUD_SPI_DEV_NAME "spi30"
#define SFUD_FLASH_DEV_NAME "flash0"

#endif
