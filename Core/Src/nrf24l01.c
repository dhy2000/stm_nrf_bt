#include "main.h"
#include "nrf24l01.h"

const uint8_t TX_ADDRESS[TX_ADR_WIDTH]={0x30,0x30,0x30,0x30,0x31}; //发送地址
const uint8_t RX_ADDRESS[RX_ADR_WIDTH]={0x30,0x30,0x30,0x30,0x31}; //接收地址


//针对NRF24L01修改SPI1驱动
void NRF24L01_SPI_Init(void)
{
    __HAL_SPI_DISABLE(&hspi1);               //先关闭SPI1
    hspi1.Init.CLKPolarity=SPI_POLARITY_LOW; //串行同步时钟的空闲状态为低电平
    hspi1.Init.CLKPhase=SPI_PHASE_1EDGE;     //串行同步时钟的第1个跳变沿（上升或下降）数据被采样
    HAL_SPI_Init(&hspi1);
    __HAL_SPI_ENABLE(&hspi1);                //使能SPI1
}

//初始化24L01的IO口
void NRF24L01_Init(void)
{
    // GPIO_InitTypeDef GPIO_Initure;
    // __HAL_RCC_GPIOA_CLK_ENABLE();			//开启GPIOA时钟
    // __HAL_RCC_GPIOB_CLK_ENABLE();			//开启GPIOB时钟


//	//GPIOB1,2推挽输出
//    GPIO_Initure.Pin=GPIO_PIN_1|GPIO_PIN_2;	//PB1,2
//    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //输出
//    HAL_GPIO_Init(GPIOB,&GPIO_Initure);     //初始化

	//GPIOA.4上拉输入
//		GPIO_Initure.Pin=GPIO_PIN_4;							//PA4
//		GPIO_Initure.Mode=GPIO_MODE_INPUT;      	//输入
//		HAL_GPIO_Init(GPIOA,&GPIO_Initure);     	//初始化

//		MX_SPI1_Init();    		              			//初始化SPI1
		NRF24L01_SPI_Init();                			//针对NRF的特点修改SPI的设置
		NRF24L01_CE_LOW(); 			            			//使能24L01
		NRF24L01_SPI_CS_DISABLE();			    			//SPI片选取消
}


static void SPI1_SetSpeed(uint8_t SPI_BaudRatePrescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));//判断有效性
    __HAL_SPI_DISABLE(&hspi1);            //关闭SPI
    hspi1.Instance->CR1&=0XFFC7;          //位3-5清零，用来设置波特率
    hspi1.Instance->CR1|=SPI_BaudRatePrescaler;//设置SPI速度
    __HAL_SPI_ENABLE(&hspi1);             //使能SPI
}


uint8_t SPIx_ReadWriteByte(SPI_HandleTypeDef* hspi,uint8_t byte)
{
  uint8_t d_read,d_send=byte;
  if(HAL_SPI_TransmitReceive(hspi,&d_send,&d_read,1,0xFF)!=HAL_OK)
  {
    d_read=0xFF;
  }
  return d_read;
}

uint8_t NRF24L01_Check(void)
{
	uint8_t buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
	uint8_t i;

	SPI1_SetSpeed(SPI_BAUDRATEPRESCALER_2); //spi速度为8.0Mhz（（24L01的最大SPI时钟为10Mhz,这里大一点没关系）
	NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,buf,5);//写入5个字节的地址.
	NRF24L01_Read_Buf(TX_ADDR,buf,5); //读出写入的地址
	for(i=0;i<5;i++)if(buf[i]!=0XA5)break;
	if(i!=5)return 1;//检测24L01错误
	return 0;		 	//检测到24L01
}


uint8_t NRF24L01_Write_Reg(uint8_t reg,uint8_t value)
{
	uint8_t status;
  NRF24L01_SPI_CS_ENABLE();                 //使能SPI传输
  status =SPIx_ReadWriteByte(&hspi1,reg);   //发送寄存器号
  SPIx_ReadWriteByte(&hspi1,value);         //写入寄存器的值
  NRF24L01_SPI_CS_DISABLE();                //禁止SPI传输
  return(status);       			//返回状态值
}


uint8_t NRF24L01_Read_Reg(uint8_t reg)
{
	uint8_t reg_val;
 	NRF24L01_SPI_CS_ENABLE();          //使能SPI传输
  SPIx_ReadWriteByte(&hspi1,reg);   //发送寄存器号
  reg_val=SPIx_ReadWriteByte(&hspi1,0XFF);//读取寄存器内容
  NRF24L01_SPI_CS_DISABLE();          //禁止SPI传输
  return(reg_val);           //返回状态值
}

uint8_t NRF24L01_Read_Buf(uint8_t reg,uint8_t *pBuf,uint8_t len)
{
	uint8_t status,uint8_t_ctr;

  NRF24L01_SPI_CS_ENABLE();           //使能SPI传输
  status=SPIx_ReadWriteByte(&hspi1,reg);//发送寄存器值(位置),并读取状态值
 	for(uint8_t_ctr=0;uint8_t_ctr<len;uint8_t_ctr++)
  {
    pBuf[uint8_t_ctr]=SPIx_ReadWriteByte(&hspi1,0XFF);//读出数据
  }
  NRF24L01_SPI_CS_DISABLE();       //关闭SPI传输
  return status;        //返回读到的状态值
}


uint8_t NRF24L01_Write_Buf(uint8_t reg, uint8_t *pBuf, uint8_t len)
{
	uint8_t status,uint8_t_ctr;
 	NRF24L01_SPI_CS_ENABLE();          //使能SPI传输
  status = SPIx_ReadWriteByte(&hspi1,reg);//发送寄存器值(位置),并读取状态值
  for(uint8_t_ctr=0; uint8_t_ctr<len; uint8_t_ctr++)
  {
    SPIx_ReadWriteByte(&hspi1,*pBuf++); //写入数据
  }
  NRF24L01_SPI_CS_DISABLE();       //关闭SPI传输
  return status;          //返回读到的状态值
}


uint8_t NRF24L01_TxPacket(uint8_t *txbuf)
{
	uint8_t sta;
	SPI1_SetSpeed(SPI_BAUDRATEPRESCALER_2); //spi速度为4.0Mhz（24L01的最大SPI时钟为10Mhz）
	NRF24L01_CE_LOW();
	NRF24L01_Write_Buf(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);//写数据到TX BUF  32个字节
 	NRF24L01_CE_HIGH();//启动发送

	while(NRF24L01_IRQ_PIN_READ()!=0);//等待发送完成

	sta=NRF24L01_Read_Reg(STATUS);  //读取状态寄存器的值
	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta); //清除TX_DS或MAX_RT中断标志
	if(sta&MAX_TX)//达到最大重发次数
	{
		NRF24L01_Write_Reg(FLUSH_TX,0xff);//清除TX FIFO寄存器
		return MAX_TX;
	}
	if(sta&TX_OK)//发送完成
	{
		return TX_OK;
	}
	return 0xff;//其他原因发送失败
}


uint8_t NRF24L01_RxPacket(uint8_t *rxbuf)
{
	uint8_t sta;
	SPI1_SetSpeed(SPI_BAUDRATEPRESCALER_8); //spi速度为4.0Mhz（24L01的最大SPI时钟为10Mhz）
	sta=NRF24L01_Read_Reg(STATUS);  //读取状态寄存器的值
	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta); //清除TX_DS或MAX_RT中断标志
	if(sta&RX_OK)//接收到数据
	{
		NRF24L01_Read_Buf(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);//读取数据
		NRF24L01_Write_Reg(FLUSH_RX,0xff);//清除RX FIFO寄存器
		return 0;
	}
	return 1;//没收到任何数据
}


void NRF24L01_RX_Mode(void)
{
	NRF24L01_CE_LOW();
  NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG, 0x0F);//配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC
  NRF24L01_Write_Reg(NRF_WRITE_REG+EN_AA,0x01);    //使能通道0的自动应答
  NRF24L01_Write_Reg(NRF_WRITE_REG+EN_RXADDR,0x01);//使能通道0的接收地址
  NRF24L01_Write_Reg(NRF_WRITE_REG+RF_CH,40);	     //设置RF通信频率
  NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x0f);//设置TX发射参数,0db增益,2Mbps,低噪声增益开启

  NRF24L01_Write_Reg(NRF_WRITE_REG+RX_PW_P0,RX_PLOAD_WIDTH);//选择通道0的有效数据宽度

  NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(uint8_t*)RX_ADDRESS,RX_ADR_WIDTH);//写RX节点地址

  NRF24L01_CE_HIGH(); //CE为高,进入接收模式
  HAL_Delay(1);
}


void NRF24L01_TX_Mode(void)
{
	NRF24L01_CE_LOW();
  NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,(uint8_t*)TX_ADDRESS,TX_ADR_WIDTH);//写TX节点地址
  NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(uint8_t*)RX_ADDRESS,RX_ADR_WIDTH); //设置TX节点地址,主要为了使能ACK

  NRF24L01_Write_Reg(NRF_WRITE_REG+EN_AA,0x01);     //使能通道0的自动应答
  NRF24L01_Write_Reg(NRF_WRITE_REG+EN_RXADDR,0x01); //使能通道0的接收地址
  NRF24L01_Write_Reg(NRF_WRITE_REG+SETUP_RETR,0xff);//设置自动重发间隔时间:4000us + 86us;最大自动重发次数:15次
  NRF24L01_Write_Reg(NRF_WRITE_REG+RF_CH,40);       //设置RF通道为40
  NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x0f);  //设置TX发射参数,0db增益,2Mbps,低噪声增益开启
  NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG,0x0e);    //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式,开启所有中断
	NRF24L01_CE_HIGH();//CE为高,10us后启动发送
  HAL_Delay(1);
}


void NRF_LowPower_Mode(void)
{
	NRF24L01_CE_LOW();
	NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG, 0x00);		//配置工作模式:掉电模式
}

