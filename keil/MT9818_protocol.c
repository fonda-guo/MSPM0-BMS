#include "MT9818_protocol.h"

MT9818_Data_Struct mt9818_data;

OV_DELAY ov_delay_time = OV_DELAY_TWO_SECOND;          
UV_DELAY uv_delay_time = UV_DELAY_FOUR_SECOND;         
OCD_THRESH ocd_thresh = OCD_THRESH_95MV;               
OCD_DELAY ocd_delay_time = OCD_DELAY_1280MS;					
SCD_THRESH scd_thresh = SCD_THRESH_190MV;            
SCD_DELAY scd_delay_time = SCD_DELAY_400US;     


void MT9818_Parameter_Init(void)
{

	uint8_t delay = 0;
	uint8_t ov_val = 0;
	uint8_t uv_val = 0;
//	uint8_t sys_ctl = 0;
	double BAT_ADC_GAIN3;
	uint16_t temp = 0;
	volatile uint8_t protect_reg2 = 0,protect_reg1 = 0;

	#if 1
	if (ov_delay_time == OV_DELAY_TWO_SECOND)
	{		
			delay |= (1 << 4);
	} 
	else if (ov_delay_time == OV_DELAY_FOUR_SECOND)
	{
			delay |= (1 << 5);
	}
	else if (ov_delay_time == OV_DELAY_EIGHT_SECOND)
	{
			delay = delay | (1 << 5) | (1 << 4);
	} 
	if (uv_delay_time == UV_DELAY_FOUR_SECOND)
	{		
			delay |= (1 << 6);
	} 
	else if (uv_delay_time == UV_DELAY_EIGHT_SECOND)
	{
			delay |= (1 << 7);
	}
	else if (uv_delay_time == UV_DELAY_SIXTEEN_SECOND)
	{
			delay = delay | (1 << 6) | (1 << 7);
	} 	
	I2C_WriteBreq_MsgSBS(PROTECT3_REG,delay);
	
	BAT_ADC_GAIN3 = Get_BAT_ADC_GAIN3(); 
 	ov_val = (uint8_t)( (uint16_t)(OVP_VALUE*1000/BAT_ADC_GAIN3)>>4 );
	I2C_WriteBreq_MsgSBS(OV_TRIP_REG,ov_val);
	
//	uv_val = (uint8_t)( (uint16_t)(UVP_VALUE*1000/BAT_ADC_GAIN3)>>4 );
	temp =  (uint16_t)(UVP_VALUE*1000/BAT_ADC_GAIN3);
	if (temp < 0x1000)
		uv_val = 0x00;
	else 
		uv_val = (uint8_t)( temp >> 4 );
	I2C_WriteBreq_MsgSBS(UV_TRIP_REG,uv_val);
	//I2C_WriteBreq_Mult_MsgSBS(SLAVE_ADDRESS,OV_TRIP_REG,ov_val,uv_val);
	

	protect_reg1 = (scd_delay_time << 3) | (RSNS << 7) | scd_thresh;
	I2C_WriteBreq_MsgSBS(PROTECT1_REG,protect_reg1);
	

	protect_reg2 = (ocd_delay_time << 4) | ocd_thresh;
	I2C_WriteBreq_MsgSBS(PROTECT2_REG,protect_reg2);
	

	I2C_WriteBreq_MsgSBS(SYS_STAT_REG,0x9F);

//	sys_ctl = I2C_GetRead_MsgSBS(SLAVE_ADDRESS,SYS_CTRL2_REG);
//	sys_ctl |= (1<<6); 
	I2C_WriteBreq_MsgSBS(SYS_CTRL2_REG,0X43);
#endif
}

void Get_All_Vol(void)
{
	uint8_t i = 0;
	float BAT_ADC_GAIN = 0;
	float BAT_ADC_GAIN1 = 0;
	float BAT_ADC_GAIN2 = 0;
	float BAT_ADC_GAIN3 = 0;
	volatile float vol[18] = {0};
	volatile uint16_t rd_vol[18] = {0};
	volatile uint8_t vol_index = 0;
	
	
	BAT_ADC_GAIN = Get_BAT_ADC_GAIN();
	BAT_ADC_GAIN1 = Get_BAT_ADC_GAIN1();
	BAT_ADC_GAIN2 = Get_BAT_ADC_GAIN2();
	BAT_ADC_GAIN3 = Get_BAT_ADC_GAIN3();    
	delayMS(1);
		
	#if 0			
	I2C_Read_Mult_BS(SLAVE_ADDRESS,VC1_ADDRESS,i2c_rev_data,36);
	for (i=0;i<36;i=i+2)
	{
		rd_vol[vol_index] = (i2c_rev_data[i]<<8) | (i2c_rev_data[i+1]);
		if(vol_index<2)
		{
			mt9818_data.Volt[vol_index] = rd_vol[vol_index] * BAT_ADC_GAIN / 1000;
		}
		else if(vol_index<4)
		{
			mt9818_data.Volt[vol_index] = rd_vol[vol_index] * BAT_ADC_GAIN1 / 1000;
		}
		else if(vol_index<6)
		{
			mt9818_data.Volt[vol_index] = rd_vol[vol_index] * BAT_ADC_GAIN2 / 1000;
		}
		else 
		{
			mt9818_data.Volt[vol_index] = rd_vol[vol_index] * BAT_ADC_GAIN3 / 1000;	
		}		
		vol_index++;
	}
	#else	
	for(i = 0;i<18;i++)
	{
			rd_vol[i] = (I2C_GetRead_MsgSBS(0x0c+i*2) << 8 ) | (I2C_GetRead_MsgSBS(0x0c+(i*2+1)));
			delayMS(1);
			if(i<2)
			{
				mt9818_data.Volt[i] = rd_vol[i] * BAT_ADC_GAIN / 1000;
			}
			else if(i<4)
			{
				mt9818_data.Volt[i] = rd_vol[i] * BAT_ADC_GAIN1 / 1000;
			}
			else if(i<6)
			{
				mt9818_data.Volt[i] = rd_vol[i] * BAT_ADC_GAIN2 / 1000;
			}
			else 
			{
				mt9818_data.Volt[i] = rd_vol[i] * BAT_ADC_GAIN3 / 1000;			
			}
	}
	#endif
       
	mt9818_data.TotalVolt =( I2C_GetRead_MsgSBS(0x30) << 8 | I2C_GetRead_MsgSBS(0x31) ) * BAT_ADC_GAIN3 * 8 / 1000;
}

//BAT1_2_ADC_GAIN(0x3F)
float Get_BAT_ADC_GAIN(void)
{
	float data = 0;
	float value = 0;
	
	data = I2C_GetRead_MsgSBS(0x3f);
	
	value =  data / 4 + 368;
	
	return value;
}

//BAT3_4_ADC_GAIN(0x40)
float Get_BAT_ADC_GAIN1(void)
{
	float data = 0;
	float value = 0;
	
	data = I2C_GetRead_MsgSBS(0x40);
	
	value =  data / 4 + 368;
	
	return value;
}

//BAT5_6_ADC_GAIN(0x41)
float Get_BAT_ADC_GAIN2(void)
{
	float data = 0;
	float value = 0;
	
	data = I2C_GetRead_MsgSBS(0x41);
	
	value =  data / 4 + 368;
	
	return value;
}

//BAT7_18_ADC_GAIN(0x42)
float Get_BAT_ADC_GAIN3(void)
{
	float data = 0;
	float value = 0;
	
	data = I2C_GetRead_MsgSBS(0x42);
	
	value =  data / 4 + 368;
	
	return value; 
}