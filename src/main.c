/******************************************************************************
 PicoTCP. Copyright (c) 2012-2014 TASS Belgium NV. Some rights reserved.
 See LICENSE and COPYING for usage. https://github.com/tass-belgium/picotcp

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License Version 2
 as published by the Free Software Foundation;

 Authors: Maxime Vincent, Wim Leflere

******************************************************************************/

/* PicoTCP includes */
#include "pico_defines.h"
#include "pico_stack.h"
#include "pico_config.h"
#include "pico_ipv4.h"
#include "pico_socket.h"
#include "pico_icmp4.h"
#include "pico_device.h"
#include "pico_https_server.h"
//#include "pico_https_client.h"
#include "pico_https_util.h"
#include "SSL_keys.h"
#include <math.h>

#include "www_files.h"

#ifdef STM32F407xx
	#include "https_stm32.h"
	#define TARGET_SPECIFIC_POLL   stm32_https_poll_state
	#define TARGET_SPECIFIC_INIT   stm32_https_board_init
	#define TARGET_SPECIFIC_TOGGLE stm32_https_toggle
#else
	#include "https_sandbox.h"
	#define TARGET_SPECIFIC_POLL(...)   do {} while(0);
	#define TARGET_SPECIFIC_INIT(...)   do {} while(0);
	#define TARGET_SPECIFIC_TOGGLE(...) do {} while(0);
#endif

/************************** PRIVATE DEFINITIONS *************************/
#define PIN_INT_PORT        0
#define PIN_INT_NUM         0
#define PIN_INT_BIT         7
#define INT_ID              PIN_INT0_IRQn
#define TICKRATE_HZ         1000
/* buffer size (in byte) for R/W operations */
#define BUFFER_SIZE     1024

#ifdef dbg
    #undef dbg
#endif
//#define dbg lpc_printf
#define dbg(...) do {} while(0)
//#define dbg printf

/************************** PRIVATE VARIABLES *************************/


static const char formAccepted[] =
"<html><head><meta http-equiv=\"refresh\" content=\"1; url=/\"></head>\
<body><p>Thanks for posting your data.</p></body>";

static const char my_ip[] = "192.168.2.150";
static const char toggledString[] = "Led toggled!";
static uint32_t download_progress = 1u;
static uint32_t length_max = 1u;
static uint8_t http_not_found;
static struct pico_device *pico_dev_eth;

/************************** PRIVATE PROTOTYPES *************************/
void TIMER1_Configuration(void);
void TIMER1_IRQHandler(void);

/************************** PRIVATE FUNCTIONS *************************/
const struct Www_file * find_www_file(char * filename)
{
    uint16_t i;
    for(i = 0; i < num_files; i++)
    {
        if(strcmp(www_files[i].filename, filename) == 0)
        {
            return &www_files[i];
        }
    }
    return NULL;
}
/*** START HTTPS server ***/
#define SIZE 1 * 1024
char http_buffer[SIZE];

void serverWakeup(uint16_t ev, uint16_t conn)
{
    char * body;
    uint32_t read = 0;

    if(ev & EV_HTTPS_CON)
    {
        dbg("New connection received\n");
        pico_https_server_accept();

    }

    if(ev & EV_HTTPS_REQ) /* new header received */
    {
        char *resource;
        int method;
        dbg("Header request received\n");
        resource = pico_https_getResource(conn);
        if(strcmp(resource, "/") == 0)
        {
            resource = "/stm32f4.html";
        }
        method = pico_https_getMethod(conn);
        if(strcmp(resource, "/board_info") == 0)
        {
			TARGET_SPECIFIC_POLL();
            pico_https_respond(conn, HTTPS_RESOURCE_FOUND);
            strcpy(http_buffer, "{\"uptime\":");
            pico_itoa(PICO_TIME(), http_buffer + strlen(http_buffer));
			
            strcat(http_buffer, ", \"l1\":\"");
            strcat(http_buffer, (led1_state ? "on" : "off"));

            strcat(http_buffer, "\", \"l2\":\"");
            strcat(http_buffer, (led2_state ? "on" : "off"));

            strcat(http_buffer, "\", \"l3\":\"");
            strcat(http_buffer, (led3_state ? "on" : "off"));
            
			strcat(http_buffer, "\", \"l4\":\"");
            strcat(http_buffer, (led4_state ? "on" : "off"));
			
			strcat(http_buffer, "\", \"button\":\"");
			strcat(http_buffer, (button_state ? "down" : "up"));
			
			strcat(http_buffer, "\"}");

            pico_https_submitData(conn, http_buffer, strlen(http_buffer));
        }
        else if(strcmp(resource, "/ip") == 0)
        {
            pico_https_respond(conn, HTTPS_RESOURCE_FOUND);

            struct pico_ipv4_link * link;
            link = pico_ipv4_link_by_dev(pico_dev_eth);
            if (link)
                pico_ipv4_to_string(http_buffer, link->address.addr);
            else
                strcpy(http_buffer, "0.0.0.0");
            pico_https_submitData(conn, http_buffer, strlen(http_buffer));
        }
		else if(strcmp(resource,"/led1") == 0){ TARGET_SPECIFIC_TOGGLE(0); pico_https_respond(conn, HTTPS_RESOURCE_FOUND); pico_https_submitData(conn, toggledString, sizeof(toggledString)); }
		else if(strcmp(resource,"/led2") == 0){ TARGET_SPECIFIC_TOGGLE(1); pico_https_respond(conn, HTTPS_RESOURCE_FOUND); pico_https_submitData(conn, toggledString, sizeof(toggledString)); }
		else if(strcmp(resource,"/led3") == 0){ TARGET_SPECIFIC_TOGGLE(2); pico_https_respond(conn, HTTPS_RESOURCE_FOUND); pico_https_submitData(conn, toggledString, sizeof(toggledString)); }
		else if(strcmp(resource,"/led4") == 0){ TARGET_SPECIFIC_TOGGLE(3); pico_https_respond(conn, HTTPS_RESOURCE_FOUND); pico_https_submitData(conn, toggledString, sizeof(toggledString)); }
        else /* search in flash resources */
        {
            struct Www_file * www_file;
            www_file = find_www_file(resource + 1);
            if(www_file != NULL)
            {
                uint16_t flags;
                flags = HTTPS_RESOURCE_FOUND | HTTPS_STATIC_RESOURCE;
                if(www_file->cacheable)
                {
                    flags = flags | HTTPS_CACHEABLE_RESOURCE;
                }
                pico_https_respond(conn, flags);
                pico_https_submitData(conn, www_file->content, (int) *www_file->filesize);
            } else { /* not found */
                /* reject */
                dbg("Rejected connection...\n");
                pico_https_respond(conn, HTTPS_RESOURCE_NOT_FOUND);
            }
        }

    }

    if(ev & EV_HTTPS_PROGRESS) /* submitted data was sent */
    {
        uint16_t sent, total;
        pico_https_getProgress(conn, &sent, &total);
        dbg("Chunk statistics : %d/%d sent\n", sent, total);
    }

    if(ev & EV_HTTPS_SENT) /* submitted data was fully sent */
    {
        dbg("Last chunk post !\n");
        pico_https_submitData(conn, NULL, 0); /* send the final chunk */
    }

    if(ev & EV_HTTPS_CLOSE)
    {
        dbg("Close request: %p\n", conn);
        if (conn)
            pico_https_close(conn);
        else
            dbg(">>>>>>>> Close request w/ conn=NULL!!\n");
    }

    if(ev & EV_HTTPS_ERROR)
    {
        dbg("Error on server: %p\n", conn);
        //TODO: what to do?
        //pico_http_close(conn);
    }
}
/* END HTTP server */

static void print_connections(pico_time now, void * arg)
{
    uint32_t count;
    count = pico_count_sockets(PICO_PROTO_TCP);
    dbg("TCP socket count: %d\n", count);
    pico_timer_add(1000, print_connections, NULL); 
}

void picoTickTask(void *pvParameters) {
    uint8_t mac[6] = {0x00,0x00,0x00,0x12,0x34,0x56};
    //char ipaddr[]="192.168.2.150";
    struct pico_ip4 my_eth_addr, netmask;

    pico_stack_init();
	TARGET_SPECIFIC_INIT();

    pico_dev_eth = (struct pico_device *) pico_eth_create("eth", mac);
    if (!pico_dev_eth)
        while (1);

    pico_string_to_ipv4(my_ip, &my_eth_addr.addr);
    pico_string_to_ipv4("255.255.255.0", &netmask.addr);
    pico_ipv4_link_add(pico_dev_eth, my_eth_addr, netmask);

	pico_https_setCertificate(cert_pem_1024, sizeof(cert_pem_1024));
	pico_https_setPrivateKey(privkey_pem_1024, sizeof(privkey_pem_1024));
    pico_https_server_start(0, serverWakeup);

    pico_timer_add(1000, print_connections, NULL); 

    for (;;) {
        pico_stack_tick();
        PICO_IDLE();
    }
}

/* Entrypoint after C runtime init */
int main(void)
{
    TASS_BSP_Init();
#ifdef PICO_SUPPORT_MM
    pico_mem_init(8192);
#endif
    picoTickTask(NULL);

}

