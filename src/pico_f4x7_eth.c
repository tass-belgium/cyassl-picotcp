/*********************************************************************
PicoTCP. Copyright (c) 2012 TASS Belgium NV. Some rights reserved.
See LICENSE and COPYING for usage.
Do not redistribute without a written permission by the Copyright
holders.

Authors: Maxime Vincent
*********************************************************************/
#include "pico_defines.h"
#include "pico_device.h"
#include "pico_stack.h"
#include "pico_f4x7_eth.h"
#include "stm32f4xx_hal_eth.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#include <stdio.h>

#define ETH_MTU ETH_MAX_PACKET_SIZE

static ETH_HandleTypeDef Heth;

void ETH_IRQHandler(void)
{
      HAL_ETH_IRQHandler(&Heth);
}

/***********************/
/* extern declarations */
/***********************/
struct pico_device_f4x7 {
  struct pico_device dev;
  ETH_HandleTypeDef *heth;
};

/* Ethernet Rx & Tx DMA Descriptors */
ETH_DMADescTypeDef  DMARxDscrTab[ETH_RXBUFNB], DMATxDscrTab[ETH_TXBUFNB];

/* Ethernet Driver Receive buffers  */
uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE];

/* Ethernet Driver Transmit buffers */
uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE];

/*******************************/
/* private function prototypes */
/*******************************/
int8_t f4x7_dev_init();
int8_t f4x7_set_macaddr(uint8_t *addr);
int16_t f4x7_frame_push(ETH_HandleTypeDef *heth, uint8_t *buf, uint16_t len);
int16_t f4x7_frame_get(struct pico_device_f4x7 *eth, uint8_t **buf, uint16_t max_len);
void ETH_MACStructInit(ETH_MACInitTypeDef* macinit);

/********************/
/* public functions */
/********************/
/* static*/
int pico_eth_send(struct pico_device *dev, void *buf, int len)
{
    int ret;
    struct pico_device_f4x7 *eth = (struct pico_device_f4x7 *) dev;

    if (len > ETH_MTU)
        return -1;

    /* Write buf content to dev and return amount written */
    ret = (int) f4x7_frame_push(eth->heth, buf, len);

    //dbg("F4x7_ETH> sent %d bytes\n",ret);
    if (len != ret)
        return -1;
    else
        return ret;
}

static int pico_eth_poll(struct pico_device *dev, int loop_score)
{
  uint16_t len;
  uint8_t *rxbuf, *dmabuf;
  struct pico_device_f4x7 *eth = (struct pico_device_f4x7 *) dev;

  if (loop_score <= 0)
      return 0;

  while (loop_score > 0)
  {
      /* get received frame */
      if (HAL_ETH_GetReceivedFrame(eth->heth) == HAL_OK)
      {
          /* New frame arrived! */
          ETH_DMADescTypeDef *dmadesc;
          len = eth->heth->RxFrameInfos.length;
          dmabuf = (uint8_t *)eth->heth->RxFrameInfos.buffer;
          /* Copy to local buffer, so that we can give back this descriptor to DMA */
          rxbuf = PICO_ZALLOC(len);
          /* Received length <= ETH_MTU. If bigger, we could have memory corruption or jumbo frames! */
          if (rxbuf && (len <= ETH_MTU))
          {
              /* copy received frame to pbuf chain */
              memcpy(rxbuf, dmabuf, len);
              pico_stack_recv_zerocopy(dev, rxbuf, len);
          }
          /* free up the DMA, even is there's no memory */
          dmadesc = eth->heth->RxFrameInfos.FSRxDesc;
          eth->heth->RxFrameInfos.SegCount = 0;
          dmadesc->Status = ETH_DMARXDESC_OWN;

          loop_score--;
      } else {
          break; /* no new packets */
      }
  }

  /* Clear Rx Buffer unavailable flag if set */
  if (((eth->heth->Instance)->DMASR & ETH_DMASR_RBUS) != (uint32_t)RESET)
  {
      /* Clear RBUS ETHERNET DMA flag */
      (eth->heth->Instance)->DMASR = ETH_DMASR_RBUS;
      /* Resume DMA reception */
      (eth->heth->Instance)->DMARPDR = 0;
  }

  return loop_score;
}

/* Public interface: create/destroy. */
void pico_eth_destroy(struct pico_device *dev)
{
    pico_device_destroy(dev);
}


struct pico_device *pico_eth_create(char *name)
{
    struct pico_device_f4x7 *eth;
    ETH_HandleTypeDef *heth;
    HAL_StatusTypeDef ret;
    ETH_MACInitTypeDef macconf;
    uint8_t mac[6] = {0x12,0x34,0x00,0x13,0x0b,0x00};


    eth = pico_zalloc(sizeof(struct pico_device_f4x7));
    if (!eth)
        return NULL;

    heth = &Heth;

    heth->Init.AutoNegotiation = ETH_AUTONEGOTIATION_ENABLE;
    heth->Init.Speed = ETH_SPEED_100M;
    heth->Init.DuplexMode = ETH_MODE_FULLDUPLEX;
    heth->Init.PhyAddress = 0x0; /* LAN8720's PHYAD0 is n/c and internal pulldown */
    heth->Init.MACAddr = mac;
    heth->Init.RxMode = ETH_RXPOLLING_MODE;
    heth->Init.ChecksumMode = ETH_CHECKSUM_BY_SOFTWARE;
    heth->Init.MediaInterface = ETH_MEDIA_INTERFACE_RMII;

    heth->Instance = ETH;

    if (HAL_ETH_Init(heth) != HAL_OK)
    {
        pico_device_destroy((struct pico_device *)eth);
        pico_free(heth);
        return NULL;
    }
    eth->heth = heth;

    ret = HAL_ETH_DMATxDescListInit(heth, DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);
    ret = HAL_ETH_DMARxDescListInit(heth, DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);
    HAL_ETH_Start(heth);

    /* MAC config -> We want a custom one, that allow IPv6 traffic */
    ETH_MACStructInit(&macconf);
    ret = HAL_ETH_ConfigMAC(heth, &macconf);

    /* initialize MAC address in ethernet MAC */
    if( 0 != pico_device_init((struct pico_device *)eth, name, mac)) {
        dbg ("F4x7 eth dev init failed.\n");
        pico_device_destroy((struct pico_device *)eth);
        return NULL;
    }
    eth->dev.send = pico_eth_send;
    eth->dev.poll = pico_eth_poll;
    eth->dev.destroy = pico_eth_destroy;

    (void)ret; /* TODO: check retval of above functions */
    dbg("Device %s created.\n", eth->dev.name);

    return (struct pico_device *)eth;

}

/*********************/
/* private functions */
/*********************/
/* F4x7_ETH DRIVER FUNCTIONS */
void ETH_MACStructInit(ETH_MACInitTypeDef* macinit)
{
    /* Ethernet MAC default initialization **************************************/
    macinit->Watchdog = ETH_WATCHDOG_ENABLE;
    macinit->Jabber = ETH_JABBER_ENABLE;
    macinit->InterFrameGap = ETH_INTERFRAMEGAP_96BIT;
    macinit->CarrierSense = ETH_CARRIERSENCE_ENABLE;
    macinit->ReceiveOwn = ETH_RECEIVEOWN_DISABLE;
    macinit->LoopbackMode = ETH_LOOPBACKMODE_DISABLE;
    macinit->ChecksumOffload = ETH_CHECKSUMOFFLAOD_DISABLE;
    macinit->RetryTransmission = ETH_RETRYTRANSMISSION_DISABLE;
    macinit->AutomaticPadCRCStrip = ETH_AUTOMATICPADCRCSTRIP_DISABLE;
    macinit->BackOffLimit = ETH_BACKOFFLIMIT_10;
    macinit->DeferralCheck = ETH_DEFFERRALCHECK_DISABLE;
    macinit->ReceiveAll = ETH_RECEIVEALL_ENABLE;
    macinit->SourceAddrFilter = ETH_SOURCEADDRFILTER_DISABLE;
    macinit->PassControlFrames = ETH_PASSCONTROLFRAMES_FORWARDALL; // Needed for IPv6 traffic!!
    macinit->BroadcastFramesReception = ETH_BROADCASTFRAMESRECEPTION_ENABLE;
    macinit->DestinationAddrFilter = ETH_DESTINATIONADDRFILTER_NORMAL;
    macinit->PromiscuousMode = ETH_PROMISCIOUSMODE_DISABLE;
    macinit->MulticastFramesFilter = ETH_MULTICASTFRAMESFILTER_PERFECT;
    macinit->UnicastFramesFilter = ETH_UNICASTFRAMESFILTER_PERFECT;
    macinit->HashTableHigh = 0x0;
    macinit->HashTableLow = 0x0;
    macinit->PauseTime = 0x0;
    macinit->ZeroQuantaPause = ETH_ZEROQUANTAPAUSE_DISABLE;
    macinit->PauseLowThreshold = ETH_PAUSELOWTHRESHOLD_MINUS4;
    macinit->UnicastPauseFrameDetect = ETH_UNICASTPAUSEFRAMEDETECT_DISABLE;
    macinit->ReceiveFlowControl = ETH_RECEIVEFLOWCONTROL_DISABLE;
    macinit->TransmitFlowControl = ETH_TRANSMITFLOWCONTROL_DISABLE;
    macinit->VLANTagComparison = ETH_VLANTAGCOMPARISON_16BIT;
    macinit->VLANTagIdentifier = 0x0;
}

int16_t f4x7_frame_push(ETH_HandleTypeDef *heth, uint8_t *buf, uint16_t len)
{
    uint8_t *buffer =  (uint8_t *)(heth->TxDesc->Buffer1Addr);

    /* copy frame from local buf to DMA buffers */
    memcpy(buffer, buf, len);
  
    /* Prepare transmit descriptors to give to DMA*/
    if (HAL_ETH_TransmitFrame(heth, len) != HAL_OK)
        return 0;
  
    return len;
}

// ============================================================================
void HAL_ETH_MspInit(ETH_HandleTypeDef *heth)
{
    volatile uint32_t i;
    GPIO_InitTypeDef GPIO_InitStructure;
  
    /* Enable GPIOs clocks */
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __GPIOE_CLK_ENABLE();
  
    /* Enable SYSCFG clock */
    __SYSCFG_CLK_ENABLE();
  
    /* Enable ETH clock */
    __ETH_CLK_ENABLE(); /* MAC, MACTx and MACRx clock */
  
    /* MII/RMII Media interface selection --------------------------------------*/
    /* done in HAL_ETH_Init */
  
    /* Ethernet pins configuration ************************************************/
    /* For STM32F4DIS-BB Discover-Mo board 
          ETH_MDIO --------------> PA2
          ETH_MDC ---------------> PC1
  
          ETH_RMII_REF_CLK-------> PA1
  
          ETH_RMII_CRS_DV -------> PA7
  		ETH_MII_RX_ER   -------> PB10
          ETH_RMII_RXD0   -------> PC4
          ETH_RMII_RXD1   -------> PC5
          ETH_RMII_TX_EN  -------> PB11
          ETH_RMII_TXD0   -------> PB12
          ETH_RMII_TXD1   -------> PB13
  
          ETH_nRST_PIN    -------> PE2
     */
  
    /* Configure PA1,PA2 and PA7 */
    GPIO_InitStructure.Pin   = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
    GPIO_InitStructure.Speed = GPIO_SPEED_HIGH; /* was 100 MHz */
    GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Pull  = GPIO_NOPULL;
    GPIO_InitStructure.Alternate  = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
  
    /* Configure PB10,PB11,PB12 and PB13 */
    GPIO_InitStructure.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
    GPIO_InitStructure.Alternate  = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
  
    /* Configure PC1, PC4 and PC5 */
    GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStructure.Alternate  = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
  
    /* Configure the PHY RST  pin */
    GPIO_InitStructure.Pin = GPIO_PIN_2;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM; /* was 50 MHz */
    HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
  
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);
    for (i = 0; i < 20000; i++);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);
    for (i = 0; i < 20000; i++);
}
