#include "aw2013.h"

static uint32_t on_time=0,on_level=0,blink=9;
static struct led_setting led0,led1,led2;
static DEFINE_MUTEX(aw_mutex);

struct i2c_client *aw2013_i2c_client;

static unsigned char LED_ON_FLAG = 0x0;

static HW_DEVICE g_hwctl_device={.init_stage=DEV_NONE_STAGE,};

static struct file_operations hwctl_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= hwctl_unlock_ioctl,
	.open		= hwctl_open,
	.release	= hwctl_release,
	.read		= hwctl_read,
};

static const struct i2c_device_id aw2013_i2c_id[] = {
	{ "AW2013", 0 },
	{ }
};


static struct i2c_board_info __initdata aw2013_i2c_hw={ I2C_BOARD_INFO("AW2013", (AW2013_I2C_ADDRESS_WRITE>>1))};

static struct i2c_driver aw2013_i2c_driver = {
        .driver = 
	   {
                .name   = "AW2013",
        },

        .probe          = AW2013_i2c_probe,
        .remove         = AW2013_i2c_remove,
        .id_table       = aw2013_i2c_id,
};


//*******************************AW2013�����Ƴ���***********************************///
/*
void led_flash_aw2013( unsigned int id )   //id = 0/1/2���ֱ���\u0160LED0 LED1 LED2
{
	unsigned char reg_data;
	unsigned int	reg_buffer[8];
	int j=0;
	//while(1)
		{
	//write_reg(0x00, 0x55);				// Reset 
	hwctl_i2c_write_reg(0x01, 0x01);		// enable LED ��ʹ���ж�			

	hwctl_i2c_write_reg(0x31+id, 0x70|Imax);	//config mode, IMAX 	
	hwctl_i2c_write_reg(0x34+id, 0xff);	// LED0 level,
											
	hwctl_i2c_write_reg(0x37+id*3, Rise_time<<4 |Hold_time);	//led0  t_rise=0.52s  && t_hold=1.04s								
	hwctl_i2c_write_reg(0x38+id*3, Fall_time<<4 |Off_time);	//led0  t_fall=0.52s  && t_off=4.16s
	hwctl_i2c_write_reg(0x39+id*3, Delay_time<<4 |Period_Num);	//led0  t_Delay=0s && cnt=����\u017d�
					
	//read_reg(0x30,1,reg_buffer);
	//reg_data = reg_buffer[0] | (0x1 << id);
	hwctl_i2c_write_reg(0x30, 1<<id);	//led on	
		for (j=0; j < 50000; j++);//����ʱ5us����

		}
}
*/
void hwctl_led_off()//( unsigned int id )
{
    //   printk("==== [aw2013]hwctl_led_off ====\n");
	unsigned char reg_data;
	unsigned int	reg_buffer[8];

	hwctl_i2c_write_reg(0x30, 0);				//led off	
	hwctl_i2c_write_reg(0x01,0);

	}

void hwctl_delay_1us(U16 wTime)   //
{
	udelay(wTime);
}

// Bee-20140417
static BOOL hwctl_i2c_write_reg_org(unsigned char reg,unsigned char data)
{
	BOOL ack=0;
	unsigned char ret;
	unsigned char wrbuf[2];

	wrbuf[0] = reg;
	wrbuf[1] = data;

	ret = i2c_master_send(aw2013_i2c_client, wrbuf, 2);
	if (ret != 2) {
		dev_err(&aw2013_i2c_client->dev,
		"%s: i2c_master_recv() failed, ret=%d\n",
		__func__, ret);
		ack = 1;
	}

	return ack;
}

BOOL hwctl_i2c_write_reg(unsigned char reg,unsigned char data)
{
	BOOL ack=0;
	unsigned char i;
	for (i=0; i<AW2013_I2C_MAX_LOOP; i++)
	{
		ack = hwctl_i2c_write_reg_org(reg,data);
		if (ack == 0) // ack success
			break;
		}
	return ack;
}

unsigned char hwctl_i2c_read_reg(unsigned char regaddr) 
{
	unsigned char rdbuf[1], wrbuf[1], ret, i;

	wrbuf[0] = regaddr;

	for (i=0; i<AW2013_I2C_MAX_LOOP; i++) 
	{
		ret = i2c_master_send(aw2013_i2c_client, wrbuf, 1);
		if (ret == 1)
			break;
	}
	
	ret = i2c_master_recv(aw2013_i2c_client, rdbuf, 1);
	
	if (ret != 1)
	{
	//	printk("**********************   5555   ning hwctl_i2c_read_reg failed  %s \r\n", __func__);
		dev_err(&aw2013_i2c_client->dev,"%s: i2c_master_recv() failed, ret=%d\n",
			__func__, ret);
	}
	
    	return rdbuf[0];
		
}


	
//#endif //aw2013

#if 0//for s5 only 20130220
extern void tpd_toggle_hand_using(int left_right_hand);
#endif

#if 1


int breathlight_master_send(u16 addr, char * buf ,int count)
{
	unsigned char ret;
	
	ret = i2c_master_send(aw2013_i2c_client, buf, count);
	
	if (ret != count) 
	{

	printk("**********************aw2013   8888   ning breathlight_master_send failed  %s \r\n", __func__);
		dev_err(&aw2013_i2c_client->dev,"%s: i2c_master_recv() failed, ret=%d\n",
			__func__, ret);
	}
	return ret;
}

/////////////////////////////for aw2013
void led_flash_aw2013_test( unsigned int id )
{
	char buf[2];
	
	hwctl_i2c_read_reg(0x55);
	
	//printk("hwctl led_flash_aw2013_test \n");	
	buf[0]=0x01;
	buf[1]=0x01;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x31;
	buf[1]=0x71;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x34;
	buf[1]=0xff;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x37;
	buf[1]=0x53;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x38;
	buf[1]=0x55;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x39;
	buf[1]=0x00;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x30;
	buf[1]=0x01;
	breathlight_master_send(0x45,buf,2);	
}

void led_off_aw2013_test(void)
{
	char buf[2];
	buf[0]=0x30;
	buf[1]=0x00;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x01;
	buf[1]=0x00;
	breathlight_master_send(0x45,buf,2);
}

void led_flash_aw2013( unsigned int id )
{
	char buf[2];
printk("==== [aw2013]led_flash_aw2013 id=%d\n",id);   
	
	buf[0]=0x01;
	buf[1]=0x01;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x31+id;
	buf[1]=0x01;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x34+id;
	buf[1]=0xff;
	breathlight_master_send(0x45,buf,2);
/** /
	buf[0]=0x37+id*3;
	buf[1]=0x53;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x38+id*3;
	buf[1]=0x55;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x39+id*3;
	buf[1]=0x00;
	breathlight_master_send(0x45,buf,2);
/**/
	buf[0]=0x30;
	buf[1]=1<<id;
	breathlight_master_send(0x45,buf,2);
	hwctl_delay_1us(8);
}

void led_flash_aw2013_power_low(void)/////red led
{
      printk("==== [aw2013]led_flash_aw2013_power_low ====\n");    
	unsigned int id =0;/////red led
	char buf[2];

	buf[0]=0x00;
	buf[1]=0x54;/////reset led module
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x01;
	buf[1]=0x01;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x31+id;
	buf[1]=0x71;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x34+id;
	buf[1]=0xff;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x37+id*3;
	buf[1]=0x00;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x38+id*3;
	buf[1]=0x06;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x39+id*3;
	buf[1]=0x00;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x30;
	buf[1]=1<<id;
	breathlight_master_send(0x45,buf,2);
}

void led_flash_aw2013_charging_full(void)
{
 printk("==== [aw2013]led_flash_aw2013_charging_full ====\n");
	unsigned int id = 1;/////green led
	char buf[2];

	buf[0]=0x00;
	buf[1]=0x54;/////reset led module
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x01;
	buf[1]=0x01;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x31+id;
	buf[1]=0x02;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x34+id;
	buf[1]=0xff;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x30;
	buf[1]=1<<id;
	breathlight_master_send(0x45,buf,2);
	
}

void led_flash_aw2013_charging(void)
{
   printk("==== [aw2013]led_flash_aw2013_charging ====\n");
	unsigned int id = 0; //red led
    char buf[2];
	char temp;

    if(blink == 0)
    {
    	buf[0]=0x00;
    	buf[1]=0x55;/////reset led module
    	breathlight_master_send(0x45,buf,2);

    	buf[0]=0x01;
    	buf[1]=0x01;
    	breathlight_master_send(0x45,buf,2);

    	buf[0]=0x31+id;
    	buf[1]=0x02;
    	breathlight_master_send(0x45,buf,2);

    	buf[0]=0x34+id;
    	buf[1]=0xff;
    	breathlight_master_send(0x45,buf,2);

    	buf[0]=0x30;
    	buf[1]=1<<id;
    	breathlight_master_send(0x45,buf,2);
    }

}

void led_flash_aw2013_unanswer_message_incall(void)/////blue led
{      
    printk("==== [aw2013]led_flash_aw2013_unanswer_message_incall ====\n");
    static unsigned int led1=0,led2=0,led3=0; 
    unsigned int color,red,green,blue,id;
    color = (on_level>> 24) & 0xFF;
    if(color) {
    	red = (on_level >> 16) & 0xFF;
    	green = (on_level >> 8) & 0xFF;
    	blue = on_level & 0xFF;
    }else { // color = 0 means turn the LED off
    	red = green = blue = 0;
    }
  printk("==== [aw2013]incall:red=%x,green=%x,blue=%x ====\n",red,green,blue);  
    if(red) {
        led1 = 1;
        id = 0;
       // printk("==== red:led1=%d ====\n",led1);
    }
    if(green) {
        led2 = 1;
        id = 1;
       // printk("==== green:led2=%d ====\n",led2);
    }
    if(blue) {            
        led3 = 1;
        id = 2;
        printk("==== blue:led3=%d ====\n",led3);
    }

//    aw2013_breath_all(led1,led2,led3);

    /*wangli_20140709* /        
	unsigned int id = 2; //blue led
	char buf[2];
	char temp;

	buf[0]=0x00;
	buf[1]=0x54;/////reset led module
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x01;//enable led module
	buf[1]=0x01;
	breathlight_master_send(0x45,buf,2);

    for(i = 0; i < 3; i++)
    {
    	buf[0]=0x31+id[i];//pwm control mode
    	buf[1]=0x73;
    	breathlight_master_send(0x45,buf,2);

    	buf[0]=0x34+id[i];//brightness
    	buf[1]=0xff;
    	breathlight_master_send(0x45,buf,2);

    	buf[0]=0x37+id[i]*3;
    	buf[1]=0x04;
    	breathlight_master_send(0x45,buf,2);

    	buf[0]=0x38+id[i]*3;
    	buf[1]=0x04;
    	breathlight_master_send(0x45,buf,2);

    	buf[0]=0x39+id[i]*3;
    	buf[1]=0x00;
    	breathlight_master_send(0x45,buf,2);

    	buf[0]=0x30;//enable 3 channel
    	buf[1]=1<<id[i];
    	breathlight_master_send(0x45,buf,2);
    }
    /**/
}

void led_flash_aw2013_power_on(void)/////
{
	char buf[2];
	unsigned int id =0;////0 blue led ,1 red,2 green,

	buf[0]=0x01;
	buf[1]=0x01;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x31+id;
	buf[1]=0x73;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x34+id;
	buf[1]=0xff;//0xc8;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x37+id*3;
	buf[1]=0x34;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x38+id*3;
	buf[1]=0x35;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x39+id*3;
	buf[1]=0x03;
	breathlight_master_send(0x45,buf,2);

	buf[0]=0x30;
	buf[1]=1<<id;
	breathlight_master_send(0x45,buf,2);
}

void Flush_led_data(void)
{
	printk("==== In Flush_led_data: LED_ON_FLAG=%d ====\n",LED_ON_FLAG);
	//first if it's charging situation, we skip the other actions
	if(LED_ON_FLAG == CHARGING_FLAG_BIT) 
	{
		printk("==== CALL led_flash_aw2013_charging() ====\n");
        Suspend_led();
		led_flash_aw2013_charging();
		return;
	}
	if(LED_ON_FLAG == TIME_FLAG_BIT) 
	{
		printk("==== CALL hwctl_set_blink() ====\n");
		hwctl_set_blink(on_time);
		return;
	}
	if(LED_ON_FLAG == LED_COLOR_FLAG_BIT) 
	{
		printk("==== CALL aw2013_all_leds_control() ====\n");             
		aw2013_all_leds_control();
		return;
	}	

    
/** /    
	if(LED_ON_FLAG == CALL_FLAG_BIT) 
	{
		printk("==== CALL led_flash_aw2013_unanswer_message_incall() ====\n");             
		led_flash_aw2013_unanswer_message_incall();
		return;
	}
	//if(TST_BIT(LED_ON_FLAG,CHARGING_FULL_FLAG_BIT))
	if(LED_ON_FLAG == CHARGING_FULL_FLAG_BIT) 
	{
		printk("==== CALL led_flash_aw2013_charging_full ====\n");
		led_flash_aw2013_charging_full();
		return;
	}
	//second the bat infor is the priority
	//if(TST_BIT(LED_ON_FLAG,BAT_FLAG_BIT))
	if(LED_ON_FLAG == BAT_FLAG_BIT) 
	{
		printk("==== CALL led_flash_aw2013_power_low ====\n");
		led_flash_aw2013_power_low();
		return;
	}
	//if(TST_BIT(LED_ON_FLAG,MSG_FLAG_BIT))
	if(LED_ON_FLAG == MSG_FLAG_BIT) 
	{
		printk("==== CALL led_flash_aw2013_unanswer_message_incall-message ====\n");
		led_flash_aw2013_unanswer_message_incall();
		return;
	}	
/**/     
}

void Suspend_led(void)
{
	//first if it's charging situation, we skip the other actions
	printk("==== [HWCTL]line521:Suspend_led ====\n");
	hwctl_led_off();
}
#endif


static long hwctl_unlock_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	printk("hwctl_ioctl cmd=%x",cmd);

    long err = 0;
	void __user *ptr = (void __user*) arg;
	uint32_t level,time;

    switch(cmd)
    {
        case HW_CTL_IO_FORCE_REFRESH_LEDS:	
			printk("==== [HWCTL]line536:HW_CTL_IO_FORCE_REFRESH_LEDS ====\n");
			Suspend_led();
			break;
		case HW_CTL_IO_SET_BLINK:
            printk("==== [HWCTL]line540:HW_CTL_IO_SET_BLINK ====\n");
			LED_ON_FLAG = TIME_FLAG_BIT;
			if(copy_from_user(&time, ptr, sizeof(level)))
			{
				err = -EFAULT;
				break;
			}
			mutex_lock(&aw_mutex);
			on_time = time;
			mutex_unlock(&aw_mutex);
			printk("==== [HWCTL]line550:on_time = %d ====\n",on_time);
			Flush_led_data();
			break;           
		case HW_CTL_IO_LED_COLOR:
			printk("==== [HWCTL]line554:HW_CTL_IO_LED_COLOR ====\n");
			LED_ON_FLAG = LED_COLOR_FLAG_BIT;
			if(copy_from_user(&level, ptr, sizeof(level)))
			{
				err = -EFAULT;
				break;
			}
			mutex_lock(&aw_mutex);
			on_level = level;
			mutex_unlock(&aw_mutex);
			printk("==== [HWCTL]line564:on_level = %x ====\n",on_level);
			Flush_led_data();
			break;

/** /
		case HW_CTL_IO_EN_CALL_NTF:
			printk("==== [HWCTL]line544:HW_CTL_IO_EN_CALL_NTF ====\n");
			LED_ON_FLAG = CALL_FLAG_BIT;
			if(copy_from_user(&level, ptr, sizeof(level)))
			{
				err = -EFAULT;
				break;
			}
			mutex_lock(&aw_mutex);
			on_level = level;
			mutex_unlock(&aw_mutex);
			printk("==== [HWCTL]line 578:on_level = %x ====\n",on_level);
			hwctl_shut_charging_leds_and_dojobs();
			break;
		case HW_CTL_IO_CHARGING_EN_NTF:	
			printk("==== [HWCTL]line 556:HW_CTL_IO_CHARGING_EN_NTF ====\n");         
			LED_ON_FLAG = CHARGING_FLAG_BIT;
			if(copy_from_user(&level, ptr, sizeof(level)))
			{
				err = -EFAULT;
				break;
			}
			mutex_lock(&aw_mutex);
			on_level = level;
			mutex_unlock(&aw_mutex);
			printk("==== [HWCTL]line 607:on_level = %x ====\n",on_level);            
			Flush_led_data();///charging : red
			break;
        case HW_CTL_IO_TEST:
			printk("this is test and this module has been tested\n");
			break;
		case HW_CTL_IO_ENB_KBD:			
			printk("[HWCTL]Enable keyboard\n");
			break;
		case HW_CTL_IO_DIS_KBD:
			printk("[HWCTL]Disable keyboard\n");
			break;
/**/
/** /		case HW_CTL_IO_EN_MSG_NTF:
			//SET_BIT(LED_ON_FLAG,MSG_FLAG_BIT);
			LED_ON_FLAG = MSG_FLAG_BIT;
			if(copy_from_user(&level, ptr, sizeof(level)))
			{
				err = -EFAULT;
				break;
			}
			mutex_lock(&aw_mutex);
			on_level = level;
			mutex_unlock(&aw_mutex);
			printk("==== [HWCTL]line537:on_level = %x ====\n",on_level);
			hwctl_shut_charging_leds_and_dojobs();
			//Flush_led_data();///message : blue
			break;
		case HW_CTL_IO_DIS_MSG_NTF:			
			CLR_BIT(LED_ON_FLAG,MSG_FLAG_BIT);
			break;
/**/ 
/** /
	    case HW_CTL_IO_DIS_CALL_NTF:
			printk("==== [HWCTL]line 583:HW_CTL_IO_DIS_CALL_NTF ====\n");
			CLR_BIT(LED_ON_FLAG,CALL_FLAG_BIT);
			break;
		case HW_CTL_IO_EN_BAT_NTF:	
			printk("==== [HWCTL]line 587:HW_CTL_IO_EN_BAT_NTF ====\n");           
			//SET_BIT(LED_ON_FLAG,BAT_FLAG_BIT);
			LED_ON_FLAG = BAT_FLAG_BIT;
			Flush_led_data();///battert low:red
			break;
		case HW_CTL_IO_DIS_BAT_NTF:			
			CLR_BIT(LED_ON_FLAG,BAT_FLAG_BIT);
			break;
/**/

/** /
		case HW_CTL_IO_CHARGING_FULL_EN_NTF:	
            printk("==== [HWCTL]line 611:HW_CTL_IO_CHARGING_FULL_EN_NTF ====\n"); 
			//SET_BIT(LED_ON_FLAG,CHARGING_FULL_FLAG_BIT);
			LED_ON_FLAG = CHARGING_FULL_FLAG_BIT;
			Flush_led_data();///charging full : green
			break;	
		case HW_CTL_IO_CHARGING_FULL_DIS_NTF:	
			printk("==== [HWCTL]HW_CTL_IO_CHARGING_FULL_DIS_NTF ====\n"); 
			CLR_BIT(LED_ON_FLAG,CHARGING_FULL_FLAG_BIT);
			break;
		case HW_CTL_IO_CHARGING_DIS_NTF:	
			printk("==== [HWCTL]line 593:HW_CTL_IO_CHARGING_DIS_NTF ====\n");
			CLR_BIT(LED_ON_FLAG,CHARGING_FLAG_BIT);
			hwctl_led_off();
			break;	
		case HW_CTL_IO_LEFT_HAND_NTF:	////added by liyunpen20130219
			//tpd_toggle_hand_using(0);
			//SET_BIT(LED_ON_FLAG,CHARGING_FLAG_BIT);
			break;
		case HW_CTL_IO_RIGHT_HAND_NTF:	////added by liyunpen20130219
			//tpd_toggle_hand_using(1);
			//CLR_BIT(LED_ON_FLAG,CHARGING_FLAG_BIT);
			break;	
/**/
		default:
			break;
	}
	return 0;
}

static int hwctl_open(struct inode *inode, struct file *file)
{ 
   	return 0;
}

static int hwctl_release(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t hwctl_read(struct file * fp, char __user * to, size_t read_size, loff_t * pos)
{
#define VERSION_NO	"Hardware direct control driver :89.0 by david wang"

	ssize_t copy_size = strlen(VERSION_NO);

	printk("hwctl_read ops");

	return copy_to_user(to,VERSION_NO,copy_size);
}


#if 0//defined(CONFIG_HAS_EARLYSUSPEND)
static void hwdctl_early_suspend(struct early_suspend *h)
{
	Flush_led_data();
}

static void hwdctl_late_resume(struct early_suspend *h)
{
	Suspend_led();
}
#endif


void hwctl_shut_charging_leds_and_dojobs()
{
    printk("==== In hwctl_shut_charging_leds_and_dojobs ====\n");
	Suspend_led();
	CLR_BIT(LED_ON_FLAG,CHARGING_FLAG_BIT);
	CLR_BIT(LED_ON_FLAG,CHARGING_FULL_FLAG_BIT);

	Flush_led_data();
}

/*
//begin-Bee-20140417
static int aw2013_create_sysfs(struct i2c_client *client)
{
	int err;
	struct device *dev = &(client->dev);

	TS_DBG("%s", __func__);
	
	err = device_create_file(dev, &dev_attr_led);
	err = device_create_file(dev, &dev_attr_reg);
	return err;
}
*/

#if 0
void AW2013_Hw_reset(void)
{   
	mt_set_gpio_mode(AW2013_RESET_PIN, GPIO_LED_EN_M_GPIO);
	mt_set_gpio_dir(AW2013_RESET_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(AW2013_RESET_PIN, GPIO_OUT_ONE);
      msleep(5);
	mt_set_gpio_out(AW2013_RESET_PIN, GPIO_OUT_ZERO);
      msleep(9);
	mt_set_gpio_out(AW2013_RESET_PIN, GPIO_OUT_ONE);
      msleep(30);
}
#endif


static BOOL AW2013_i2c_write_reg(unsigned char reg,unsigned char data)
{
	char buf[2];

	buf[0]=reg;
	buf[1]=data;
	breathlight_master_send(0,buf,2);


}



void AW2013_test(void)
{


	 
   printk("AW2013_test_entry=\r\n");
   
   printk("AW2013_i2c_read_reg_0x00=%x\r\n",hwctl_i2c_read_reg(0x00));
   printk("AW2013_i2c_read_reg_0x01=%x\r\n",hwctl_i2c_read_reg(0x01)); 
   printk("AW2013_i2c_read_reg_0x02=%x\r\n",hwctl_i2c_read_reg(0x02));
   printk("AW2013_i2c_read_reg_0x03=%x\r\n",hwctl_i2c_read_reg(0x03)); 
   printk("AW2013_i2c_read_reg_0x04=%x\r\n",hwctl_i2c_read_reg(0x04));
   printk("AW2013_i2c_read_reg_0x05=%x\r\n",hwctl_i2c_read_reg(0x05)); 
   printk("AW2013_i2c_read_reg_0x06=%x\r\n",hwctl_i2c_read_reg(0x06));
   printk("AW2013_i2c_read_reg_0x07=%x\r\n",hwctl_i2c_read_reg(0x07));    
   printk("AW2013_i2c_read_reg_0x12=%x\r\n",hwctl_i2c_read_reg(0x12));
   printk("AW2013_i2c_read_reg_0x13=%x\r\n",hwctl_i2c_read_reg(0x13));   

}


void AW2013_OnOff(BOOL OnOff)  
{   
	char buf[2];
	//Paoma_cnt=0;
//	Breath_cnt=0;
	//mt_set_gpio_mode(AW2013_RESET_PIN, GPIO_LED_EN_M_GPIO);
	//mt_set_gpio_dir(AW2013_RESET_PIN, GPIO_DIR_OUT);
	
	buf[0]=0x7f;
	buf[1]=0x00;
	breathlight_master_send(0x7f,buf,2);
	//mt_set_gpio_out(AW2013_RESET_PIN, GPIO_OUT_ZERO);
      //msleep(5);
	
	//if (OnOff ==1)
	//{  
	   //mt_set_gpio_out(AW2013_RESET_PIN, GPIO_OUT_ONE);
	//}
      msleep(5);
	
}


//-------------------------------------------------------------------------------------------
//������: AW2013_AllOn
//����\u017d˺�������10·��ȫ����ÿ·�ĵ����ֱ�����\u0152��������ơ�
//-------------------------------------------------------------------------------------------
void AW2013_AllOn(void)
{
	printk("-------------------------AW2013_AllOn  Entry ------------------------- \r\n");

	AW2013_OnOff(1);
	
	AW2013_i2c_write_reg(0x12,0x00);   //OUT����Ϊ������ģ�\u0153
	AW2013_i2c_write_reg(0x12,0x00);   //OUT����Ϊ������ģ�\u0153	 
	AW2013_i2c_write_reg(0x13,0x00);   //OUT����Ϊ������ģ�\u0153
	AW2013_i2c_write_reg(0x13,0x00);   //OUT����Ϊ������ģ�\u0153
	AW2013_i2c_write_reg(0x20,0x3f);//OUT0�ڵ��⣬�����\u0152�Ϊ0-255��OUT0~OUT5�ĵ����\u017e����\u017d�Ϊ0x20~0x25. �\u017d0�ر�
	AW2013_i2c_write_reg(0x21,0x3f);
	AW2013_i2c_write_reg(0x22,0x3f);
	AW2013_i2c_write_reg(0x23,0x3f);
	AW2013_i2c_write_reg(0x24,0x3f);
	AW2013_i2c_write_reg(0x25,0x3f);
}

void AW2013_out0_fade(void)
{
	AW2013_i2c_write_reg(0x12,0x00);   //OUT4~5����Ϊ������ģ�\u0153��\u0178����֮ǰ���ù������Բ�Ҫ
	AW2013_i2c_write_reg(0x13,0x00);   //OUT0~3����Ϊ������ģ�\u0153��\u0178����֮ǰ���ù������Բ�Ҫ
	AW2013_i2c_write_reg(0x04,0x03); 	 //OUT4-OUT5��������BLINKģ�\u0153ʹģ?0Ϊblinkģ�\u0153��1ΪFADEģ�\u0153
	AW2013_i2c_write_reg(0x05,0x0e);   //OUT0-OUT3��������BLINKģ�\u0153ʹ�� 0Ϊblinkģ�\u0153��1ΪFADEģ�\u0153?	AW2013_i2c_write_reg(0x15,0x09);   //��\u0153�����ʱ\u0152�������\u0178����֮ǰ���ù������Բ�Ҫ

	AW2013_i2c_write_reg(0x03,0x00);   //�Ȱ�03H��0
	AW2013_i2c_write_reg(0x03,0x01);   //\u017d�0�\u017d�\u01531�������\u0153�����
	//AW2013_i2c_write_reg(0x03,0x00);   //\u017d�1�\u017d�\u01530�������������? 
}

//-------------------------------------------------------------------------------------------
//����ΪAW2013ʵ��\u0152\u017e��Ч���Ĳο\u0152�������û���\u017e�\u0178���\u0152���Ч����Ҫ�\u017d������
//-------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------
//������: AW2013_init_pattern
//����\u017d˺�������6·��ȫ����Ϊ��������BLINK ģ�\u0153
//-------------------------------------------------------------------------------------------
void AW2013_init_pattern(void) 
{
 //AW2013���\u0178��Ч��ʵ�֡�6·��������
	printk("-------------------------AW2013_init_pattern  Entry ------------------------- \r\n");

	// AW2013_OnOff(1);
	 AW2013_i2c_write_reg(0x12,0x00);   //OUT4~5����Ϊ������ģ�\u0153
	 AW2013_i2c_write_reg(0x12,0x00);   //OUT4~5����Ϊ������ģ�\u0153	 
	 AW2013_i2c_write_reg(0x13,0x00);   //OUT0~3����Ϊ������ģ�\u0153
	 AW2013_i2c_write_reg(0x13,0x00);   //OUT0~3����Ϊ������ģ�\u0153
	 AW2013_i2c_write_reg(0x04,0x03);   //OUT4-OUT5��������BLINKģ�\u0153ʹ��
	 AW2013_i2c_write_reg(0x05,0x0f);   //OUT0-OUT3��������BLINKģ�\u0153ʹ��
	 
	 AW2013_i2c_write_reg(0x15,0x1b); 	 //��\u0153�����ʱ\u0152�����
	 AW2013_i2c_write_reg(0x16,0x12);	 //ȫ��ȫ��ʱ\u0152����� 
	  	
	 AW2013_i2c_write_reg(0x17,0x00);	 //OUT0������ʱ  
	 AW2013_i2c_write_reg(0x18,0);	//OUT1������ʱ 
	 AW2013_i2c_write_reg(0x19,0);	//OUT2������ʱ   
	 AW2013_i2c_write_reg(0x1a,0);  //OUT3������ʱ 
	 AW2013_i2c_write_reg(0x1b,0);  //OUT4������ʱ 
	 AW2013_i2c_write_reg(0x1c,0);  //OUT5������ʱ 
	 
	 AW2013_i2c_write_reg(0x14,0xff);//��������ʹ�� 

	 AW2013_i2c_write_reg(0x11,0x82);	//���\u0152������������������\u017d����
	 AW2013_i2c_write_reg(0x11,0x82);	

	 //�����������������ģ�\u0153����������з\u0153�\u0153����
	//AW2013_i2c_write_reg(0x14,0x00);//�ر���������ʹ��
	//AW2013_i2c_write_reg(0x20,0x3f);//OUT0�ڵ��⣬�����\u0152�Ϊ0-255��OUT0~OUT5�ĵ����\u017e����\u017d�Ϊ0x20~0x25. �\u017d0�ر�
	//AW2013_i2c_write_reg(0x21,0x3f);
	//AW2013_i2c_write_reg(0x22,0x3f);
	//AW2013_i2c_write_reg(0x23,0x3f);
	//AW2013_i2c_write_reg(0x24,0x3f);
	//AW2013_i2c_write_reg(0x25,0x3f);

	 
	 AW2013_test();  

	 
}

//-------------------------------------------------------------------------------------------
//������: AW2013_Marquee
//�����Ч��
//-------------------------------------------------------------------------------------------

#if 0   //  ning 

void AW2013_Marquee_loop(void)
{
	switch(Paoma_cnt)
	{
		case 0:			
			AW2013_i2c_write_reg(0x25,0x00); //�ر���һ�ŵ�
			AW2013_i2c_write_reg(0x20,0x3f); //\u017d�һ�ŵ�
			break;
		case 1:
			AW2013_i2c_write_reg(0x20,0x00); //�ر���һ�ŵ�
			AW2013_i2c_write_reg(0x21,0x3f); //\u017d�һ�ŵ�
			break;			
		case 2:
			AW2013_i2c_write_reg(0x21,0x00); 
			AW2013_i2c_write_reg(0x22,0x3f); 
			break;	
		case 3:
			AW2013_i2c_write_reg(0x22,0x00); 
			AW2013_i2c_write_reg(0x23,0x3f); 
			break;
		case 4:
			AW2013_i2c_write_reg(0x23,0x00); 
			AW2013_i2c_write_reg(0x24,0x3f); 
			break;	
		case 5:
			AW2013_i2c_write_reg(0x24,0x00); 
			AW2013_i2c_write_reg(0x25,0x3f); 
			break;	
		default:
			break;
	}
	if(Paoma_cnt<5)	
		Paoma_cnt++;
	else
		Paoma_cnt=0;
	GPTI_StartItem(AW2013_timer_ID,12,AW2013_Marquee_loop,NULL);

	


	
}

#endif

void hwctl_set_blink(uint32_t time)
{
    switch(time)
    {
        case 1499:
            blink = 0x03;
            break;
        case 1000:
            blink = 0x02;
            break;
        case 500:
            blink = 0x00;
            break;
        case 125:
            blink = 0x03;
            break;
        default:
            blink = 9;
            break;
    }
/** /    
    if(time == 1499)
    {
        blink = 0x03;    
    }
    else if(time == 1000)
    {
        blink = 0x02;
    }
    else if(time == 500)
    {
        blink = 0x00;
    }
    else
    {
        blink = 9;      
    }
/**/
    printk("==== [aw2013]line939:hwctl_set_blink blink=%d ====\n",blink);
}

void aw2013_get_leds_status(struct led_setting *led0,struct led_setting *led1,struct led_setting *led2)
{
    printk("==== [aw2013]line943:aw2013_get_leds_status ====\n");
    unsigned int color,red,green,blue;

    led0->flag = 0;
    led1->flag = 0;
    led2->flag = 0;
    color = (on_level>> 24) & 0xFF;
    if(color) {
    	red = (on_level >> 16) & 0xFF;
    	green = (on_level >> 8) & 0xFF;
    	blue = on_level & 0xFF;
    }else {
    	red = green = blue = 0;
    }
    printk("==== [aw2013]aw2013_get_leds_status:red=%x,green=%x,blue=%x ====\n",red,green,blue);  
    if(red) {
        led0->flag = 1;
        led0->level = red;
     //   printk("==== red:led0=%d ====\n",led0);
    }
    if(green) {
        led1->flag = 1;
        led1->level = green;
     //   printk("==== green:led1=%d ====\n",led1);
    }
    if(blue) {            
        led2->flag = 1;
        led2->level = blue;
       // printk("==== blue:led2=%d ====\n",led2);
    }
}

void aw2013_all_leds_control()
{
    printk("==== [aw2013]line971:aw2013_all_leds_control ====\n");
    
    aw2013_get_leds_status(&led0,&led1,&led2);

   // printk("==== [aw2013]aw2013_all_leds_control:led0=%d,led1=%d,led2=%d ====\n",led0,led1,led2);
    
    aw2013_breath_all(led0,led1,led2);
}

#if 0
void AW2013_breath_all(int led0,int led1,int led2)  //led on=0x01   ledoff=0x00
{  

	//write_reg(0x00, 0x55);				// Reset 
	hwctl_i2c_write_reg(0x01, 0x01);		// enable LED ��ʹ���ж�		

	hwctl_i2c_write_reg(0x31, Imax|0x70);	//config mode, IMAX = 5mA	
	hwctl_i2c_write_reg(0x32, Imax|0x70);	//config mode, IMAX = 5mA	
	hwctl_i2c_write_reg(0x33, Imax|0x70);	//config mode, IMAX = 5mA	

	hwctl_i2c_write_reg(0x34, 0xff);	// LED0 level,
	hwctl_i2c_write_reg(0x35, 0xff);	// LED1 level,
	hwctl_i2c_write_reg(0x36, 0xff);	// LED2 level,
											
	hwctl_i2c_write_reg(0x37, Rise_time<<4 | Hold_time);	//led0  ����ʱ\u0152䣬����ʱ\u0152���\u0161							
	hwctl_i2c_write_reg(0x38, Fall_time<<4 | Off_time);	       //led0 ��\u0153�ʱ\u0152䣬�ر�ʱ\u0152���\u0161
	hwctl_i2c_write_reg(0x39, Delay_time<<4| Period_Num);   //led0  �����ӳ�ʱ\u0152䣬����������\u0161

	hwctl_i2c_write_reg(0x3a, Rise_time<<4 | Hold_time);	//led1����ʱ\u0152䣬����ʱ\u0152���\u0161								
	hwctl_i2c_write_reg(0x3b, Fall_time<<4 | Off_time);	       //led1 ��\u0153�ʱ\u0152䣬�ر�ʱ\u0152���\u0161
	hwctl_i2c_write_reg(0x3c, Delay_time<<4| Period_Num);   //led1  �����ӳ�ʱ\u0152䣬����������\u0161

	hwctl_i2c_write_reg(0x3d, Rise_time<<4 | Hold_time);	//led2  ����ʱ\u0152䣬����ʱ\u0152���\u0161				
	hwctl_i2c_write_reg(0x3e, Fall_time<<4 | Off_time);	       //led2 ��\u0153�ʱ\u0152䣬�ر�ʱ\u0152���\u0161
	hwctl_i2c_write_reg(0x3f, Delay_time<<4| Period_Num);    //�����ӳ�ʱ\u0152䣬����������\u0161

	hwctl_i2c_write_reg(0x30, led2<<2|led1<<1|led0);	       //led on=0x01 ledoff=0x00	
	hwctl_delay_1us(8);//����ʱ5us����
}
#else
void aw2013_breath_all(struct led_setting led0,struct led_setting led1,struct led_setting led2)  //led on=0x01   ledoff=0x00
{
    printk("==== [aw2013]line951:AW2013_breath_all blink=%d ====\n",blink);
    
	hwctl_i2c_write_reg(0x00, 0x55);		// Reset 
	hwctl_i2c_write_reg(0x01, 0x01);		// enable LED ��ʹ���ж�		

    if(blink == 9)
    {
    	if(led0.flag == 1)
    	{
            hwctl_i2c_write_reg(0x31, Imax|0x03);   //pwm IMAX = 5mA 
            hwctl_i2c_write_reg(0x34, 0xff);	// LED0 level,
    	}
    	if(led1.flag== 1)
    	{

            hwctl_i2c_write_reg(0x32, Imax|0x03);   //pwm IMAX = 5mA 
            if((led0.flag== 1) && (led2.flag!= 1))
            {   hwctl_i2c_write_reg(0x35, 0xa5);	// LED1 level,
            }
            else
            {
               hwctl_i2c_write_reg(0x35, 0xff); 
            }
    	}
    	if(led2.flag== 1)
    	{
            hwctl_i2c_write_reg(0x33, Imax|0x03);   //pwm IMAX = 5mA 
            hwctl_i2c_write_reg(0x36, 0xff);	// LED2 level,
    	}
    }
    else
    {
    	hwctl_i2c_write_reg(0x31, Imax|0x70);	//config mode, IMAX = 5mA	
    	hwctl_i2c_write_reg(0x32, Imax|0x70);	//config mode, IMAX = 5mA	
    	hwctl_i2c_write_reg(0x33, Imax|0x70);	//config mode, IMAX = 5mA	

    	hwctl_i2c_write_reg(0x34, 0xff);	// LED0 level,
    	hwctl_i2c_write_reg(0x35, 0xff);	// LED1 level,
    	hwctl_i2c_write_reg(0x36, 0xff);	// LED2 level,
    											
    	hwctl_i2c_write_reg(0x37, Rise_time<<4 | blink);	//led0  ����ʱ\u0152䣬����ʱ\u0152���\u0161							
    	hwctl_i2c_write_reg(0x38, Fall_time<<4 | blink);	       //led0 ��\u0153�ʱ\u0152䣬�ر�ʱ\u0152���\u0161
    	hwctl_i2c_write_reg(0x39, Delay_time<<4| Period_Num);   //led0  �����ӳ�ʱ\u0152䣬����������\u0161

    	hwctl_i2c_write_reg(0x3a, Rise_time<<4 | blink);	//led1����ʱ\u0152䣬����ʱ\u0152���\u0161								
    	hwctl_i2c_write_reg(0x3b, Fall_time<<4 | blink);	       //led1 ��\u0153�ʱ\u0152䣬�ر�ʱ\u0152���\u0161
    	hwctl_i2c_write_reg(0x3c, Delay_time<<4| Period_Num);   //led1  �����ӳ�ʱ\u0152䣬����������\u0161

    	hwctl_i2c_write_reg(0x3d, Rise_time<<4 | blink);	//led2  ����ʱ\u0152䣬����ʱ\u0152���\u0161				
    	hwctl_i2c_write_reg(0x3e, Fall_time<<4 | blink);	       //led2 ��\u0153�ʱ\u0152䣬�ر�ʱ\u0152���\u0161
    	hwctl_i2c_write_reg(0x3f, Delay_time<<4| Period_Num);    //�����ӳ�ʱ\u0152䣬����������\u0161
    }

	hwctl_i2c_write_reg(0x30, led2.flag<<2 | led1.flag<<1 | led0.flag);	       //led on=0x01 ledoff=0x00	
	hwctl_delay_1us(8);//����ʱ5us����
}
#endif

void AW2013_Marquee(void)
{
	printk("-------------------------AW2013_Marquee  Entry ------------------------- \r\n");

	AW2013_OnOff(1);
	AW2013_i2c_write_reg(0x12,0x00);   //OUT4~5����Ϊ������ģ�\u0153
	AW2013_i2c_write_reg(0x12,0x00);   //OUT4~5����Ϊ������ģ�\u0153	 
	AW2013_i2c_write_reg(0x13,0x00);   //OUT0~3����Ϊ������ģ�\u0153
	AW2013_i2c_write_reg(0x13,0x00);   //OUT0~3����Ϊ������ģ�\u0153
	//AW2013_Marquee_loop();
}


//-------------------------------------------------------------------------------------------
//������: AW2013_ComingCall
//�\u017d��ʱ��Ч����6·��������������Ƶ��\u0153Ͽ�
//-------------------------------------------------------------------------------------------
void AW2013_ComingCall(void)
{
	printk("-------------------------AW2013_ComingCall  Entry ------------------------- \r\n");

	AW2013_OnOff(1);
	AW2013_i2c_write_reg(0x12,0x00);   //OUT4~5����Ϊ������ģ�\u0153
	AW2013_i2c_write_reg(0x12,0x00);   //OUT4~5����Ϊ������ģ�\u0153	 
	AW2013_i2c_write_reg(0x13,0x00);   //OUT0~3����Ϊ������ģ�\u0153
	AW2013_i2c_write_reg(0x13,0x00);   //OUT0~3����Ϊ������ģ�\u0153
	
	 AW2013_i2c_write_reg(0x04,0x03);   //OUT4-OUT5��������BLINKģ�\u0153ʹ��
	 AW2013_i2c_write_reg(0x05,0x0f);   //OUT0-OUT3��������BLINKģ�\u0153ʹ��	 
	 AW2013_i2c_write_reg(0x15,0x12); 	 //��\u0153�����ʱ\u0152�����
	 AW2013_i2c_write_reg(0x16,0x09);	 //ȫ��ȫ��ʱ\u0152����� 	  	 	 
	 AW2013_i2c_write_reg(0x14,0x3f);//��������ʹ�� 	 
	 AW2013_i2c_write_reg(0x11,0x82);	
}
//-------------------------------------------------------------------------------------------
//������: AW2013_MissingCall
//���\u017d\u0153��\u017d��ʱ��Ч����ֻ��2 ·�����������������𡣺���Ƶ��\u0153�����������ȫ��ʱ\u0152�
//-------------------------------------------------------------------------------------------
void AW2013_MissingCall(void)
{

	printk("-------------------------AW2013_MissingCall  Entry ------------------------- \r\n");

	AW2013_OnOff(1);
	AW2013_i2c_write_reg(0x12,0x00);   //OUT4~5����Ϊ������ģ�\u0153
	AW2013_i2c_write_reg(0x12,0x00);   //OUT4~5����Ϊ������ģ�\u0153	 
	AW2013_i2c_write_reg(0x13,0x00);   //OUT0~3����Ϊ������ģ�\u0153
	AW2013_i2c_write_reg(0x13,0x00);   //OUT0~3����Ϊ������ģ�\u0153
	
	 AW2013_i2c_write_reg(0x04,0x03);   //OUT4-OUT5��������BLINKģ�\u0153ʹ��
	 AW2013_i2c_write_reg(0x05,0x0f);   //OUT0-OUT3��������BLINKģ�\u0153ʹ��	 
	 AW2013_i2c_write_reg(0x15,0x1b); 	 //��\u0153�����ʱ\u0152�����
	 AW2013_i2c_write_reg(0x16,0x19);	 //ȫ��ȫ��ʱ\u0152����� 	  	 	 
	 AW2013_i2c_write_reg(0x14,0x03);//��������ʹ�� 	 
	 AW2013_i2c_write_reg(0x11,0x83);	
}
//-------------------------------------------------------------------------------------------
//������: AW2013_ComingMsg
//�\u017d����ʱ��Ч����1·��������������Ƶ��\u0153Ͽ�
//-------------------------------------------------------------------------------------------
void AW2013_ComingMsg(void)
{

	printk("-------------------------AW2013_ComingMsg Entry ------------------------- \r\n");

	AW2013_OnOff(1);
	AW2013_i2c_write_reg(0x12,0x00);   //OUT4~5����Ϊ������ģ�\u0153
	AW2013_i2c_write_reg(0x12,0x00);   //OUT4~5����Ϊ������ģ�\u0153	 
	AW2013_i2c_write_reg(0x13,0x00);   //OUT0~3����Ϊ������ģ�\u0153
	AW2013_i2c_write_reg(0x13,0x00);   //OUT0~3����Ϊ������ģ�\u0153
	
	 AW2013_i2c_write_reg(0x04,0x03);   //OUT4-OUT5��������BLINKģ�\u0153ʹ��
	 AW2013_i2c_write_reg(0x05,0x0f);   //OUT0-OUT3��������BLINKģ�\u0153ʹ��	 
	 AW2013_i2c_write_reg(0x15,0x09); 	 //��\u0153�����ʱ\u0152�����
	 AW2013_i2c_write_reg(0x16,0x10);	 //ȫ��ȫ��ʱ\u0152����� 	  	 	 
	 AW2013_i2c_write_reg(0x14,0x01);    //��������ʹ�� 	 
	 AW2013_i2c_write_reg(0x11,0x82);	
}
//-------------------------------------------------------------------------------------------
//������: AW2013_MissingMsg
//���\u017d\u0153�  ����ʱ��Ч����ֻ��һ·��������������Ƶ��\u0153�����������ȫ��ʱ\u0152�
//-------------------------------------------------------------------------------------------
void AW2013_MissingMsg (void)
{
	printk("-------------------------AW2013_MissingMsg Entry ------------------------- \r\n");
	AW2013_OnOff(1);
	AW2013_i2c_write_reg(0x12,0x00);   //OUT4~5����Ϊ������ģ�\u0153
	AW2013_i2c_write_reg(0x12,0x00);   //OUT4~5����Ϊ������ģ�\u0153	 
	AW2013_i2c_write_reg(0x13,0x00);   //OUT0~3����Ϊ������ģ�\u0153
	AW2013_i2c_write_reg(0x13,0x00);   //OUT0~3����Ϊ������ģ�\u0153
	
	 AW2013_i2c_write_reg(0x04,0x03);   //OUT4-OUT5��������BLINKģ�\u0153ʹ��
	 AW2013_i2c_write_reg(0x05,0x0f);   //OUT0-OUT3��������BLINKģ�\u0153ʹ��	 
	 AW2013_i2c_write_reg(0x15,0x09); 	 //��\u0153�����ʱ\u0152�����
	 AW2013_i2c_write_reg(0x16,0x20);	 //ȫ��ȫ��ʱ\u0152����� 	  	 	 
	 AW2013_i2c_write_reg(0x14,0x01);//��������ʹ�� 	 
	 AW2013_i2c_write_reg(0x11,0x82);	
}

//-------------------------------------------------------------------------------------------
//������: AW2013_GPIO_LED
//����GPIO 6789�\u017d����LED����\u0152�����������\u017e�������ʵ�ֻ��\u0178�ĺ����\u0160��
//-------------------------------------------------------------------------------------------
void AW2013_GPIO_LED(void)
{

	printk("-------------------------AW2013_GPIO_LED Entry ------------------------- \r\n");

	AW2013_OnOff(1);
	AW2013_i2c_write_reg(0x12,0xff);   //OUT4��5����ΪGPIOģ�\u0153
	AW2013_i2c_write_reg(0x12,0xff);   //OUT4��5����ΪGPIOģ�\u0153
	AW2013_i2c_write_reg(0x13,0x00);   //OUT0~3����Ϊ������ģ�\u0153
	AW2013_i2c_write_reg(0x13,0x00);   //OUT0~3����Ϊ������ģ�\u0153

	//�������\u0153���\u017e������OUT4~5 ���\u017eߵ��\u0153��
	//�û���\u017e�\u0178���Ҫ�\u017d����ĳ\u0152\u017e·��\u0152�Ϊ\u017eߣ����\u0161��timer�\u017dѭ����\u017d�\u0153��Ҫ��Ч��
	AW2013_i2c_write_reg(0x11,0x10);   //OUT6 ~9 ����Ϊ����ģ�\u0153
	AW2013_i2c_write_reg(0x02,0xff);   //OUT4~5���\u017eߵ��\u0153��
	
	 AW2013_i2c_write_reg(0x04,0x03);	//OUT4-OUT5��������BLINKģ�\u0153ʹ��
	 AW2013_i2c_write_reg(0x05,0x0f);	//OUT0-OUT3��������BLINKģ�\u0153ʹ��	 
	 AW2013_i2c_write_reg(0x15,0x12);	 //��\u0153�����ʱ\u0152�����
	 AW2013_i2c_write_reg(0x16,0x09);	 //ȫ��ȫ��ʱ\u0152����� 			 
	 AW2013_i2c_write_reg(0x14,0x3f);	 //��������ʹ�� 	 
	 AW2013_i2c_write_reg(0x11,0x92);     //���\u0152����������ע��	OUT6 ~9 ��������Ϊ����ģ�\u0153
}


//-------------------------------------------------------------------------------------------
//-----------AW2013 ���\u0178��Ч�����\u0161�� end----------------------------------------------
//-------------------------------------------------------------------------------------------



void AW2013_init(void)    
{	
	//AW2013_Hw_reset();
		
	AW2013_i2c_write_reg(0x12,0x00);   //P0������Ϊ������ģ�\u0153
	AW2013_i2c_write_reg(0x13,0x00);   //P1������Ϊ������ģ�\u0153
	
	AW2013_i2c_write_reg(0x20,0x3f);//OUT0�ڵ��⣬�����\u0152�Ϊ0-255��OUT0~OUT5�ĵ����\u017e����\u017d�Ϊ0x20~0x2f. �\u017d0�ر�
	AW2013_i2c_write_reg(0x21,0x3f);
	AW2013_i2c_write_reg(0x22,0x3f);
	AW2013_i2c_write_reg(0x23,0x3f);
	AW2013_i2c_write_reg(0x24,0x3f);
	AW2013_i2c_write_reg(0x25,0x3f);
	
	AW2013_test();  //��ȡAW2013�ڲ�\u0152�\u017d�����ֵ���\u017d���\u017d\u0153�ȥ��ֵ�Ƿ�һ�£���\u017d��ж�I2C\u0153ӿ��Ƿ��\u0161��
}




static int  AW2013_i2c_probe(struct i2c_client *client,
				      const struct i2c_device_id *id)
{
	printk("AW2013_i2c_probe:OK");


	aw2013_i2c_client = client;



	printk("****************** ningshuigen    AW2013_i2c_probe addr %x   " , aw2013_i2c_client->addr   );


	AW2013_init();

      // AW2013_init_pattern();



      AW2013_AllOn();

//	led_flash_aw2013_test(0);///z

	
//	led_flash_aw2013_power_low();
//	led_flash_aw2013_charging_full();
//	led_flash_aw2013_charging();
//	led_flash_aw2013_unanswer_message_incall();
	//aw2013_create_sysfs(client);

	return 0;
}

static int  AW2013_i2c_remove(struct i2c_client *client)
{
	aw2013_i2c_client = NULL;
	return 0;
}


static int __init AW2013_Driver_Init(void) 
{
	int ret;


	printk("********************    99999999999    ning AW2013_Driver_Init:start");

//	#err
	i2c_register_board_info(AW2013_I2C_BUS_NUM, &aw2013_i2c_hw, 1);
		 
	ret = i2c_add_driver(&aw2013_i2c_driver);

	
	printk("AW2013_Driver_Init:start \n");
	
	if(0!=alloc_chrdev_region(&g_hwctl_device.hwctl_dev_no,HW_DEVICE_MINOR,HW_DEVICE_COUNT,HW_DEVICE_NAME))
	{
		printk("hwctl_driver_alloc chrdev region:fail");
		printk("hwctl_driver_alloc chrdev region:fail \n");
		goto init_error;
		goto init_error;
		goto init_error;
		goto init_error;
	}	

	
	printk("hwctl_driver_alloc chrdev region:OK");
	
	g_hwctl_device.init_stage = DEV_ALLOC_REGION;
	
	printk("hwctl_driver_alloc chrdev region:OK1");
	g_hwctl_device.hw_cdev = cdev_alloc();
	printk("hwctl_driver_alloc chrdev region:OK2");
    	g_hwctl_device.hw_cdev->owner = THIS_MODULE;
    	printk("hwctl_driver_alloc chrdev region:OK3");
    	g_hwctl_device.hw_cdev->ops = &hwctl_fops; 
    	printk("hwctl_driver_alloc chrdev region:OK4");
    	ret = cdev_add(g_hwctl_device.hw_cdev, g_hwctl_device.hwctl_dev_no, 1);//
	printk("hwctl cdev_add_ret_is %d \n",ret);
	if(ret)
	{
		printk("hwctl_driver add cdev error\n");		
		goto init_error;
	}
	g_hwctl_device.init_stage = DEV_ADD_CDEV;
	g_hwctl_device.hw_class = class_create(THIS_MODULE, HW_DEVICE_NAME);
	
	g_hwctl_device.init_stage = DEV_ALLOC_CLASS;

	// if we want auto creat device node, we must call this
	g_hwctl_device.hw_device = device_create(g_hwctl_device.hw_class, NULL, g_hwctl_device.hwctl_dev_no, NULL, HW_DEVICE_NAME); 
	g_hwctl_device.init_stage = DEV_INIT_ALL;
	#if 0
	led_flash_aw2013_test(0);///z
	led_flash_aw2013_power_low();
	led_flash_aw2013_charging_full();
	led_flash_aw2013_charging();
	led_flash_aw2013_unanswer_message_incall();
	#endif

#if 0//def CONFIG_HAS_EARLYSUSPEND
	g_hwctl_device.early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 20;
	g_hwctl_device.early_suspend.suspend = hwdctl_early_suspend;
	g_hwctl_device.early_suspend.resume = hwdctl_late_resume;
	register_early_suspend(&g_hwctl_device.early_suspend);
#endif
	return 0;

init_error:
	if(g_hwctl_device.init_stage == DEV_ALLOC_REGION)
		unregister_chrdev_region(g_hwctl_device.hwctl_dev_no,1);
	return (-1);
}



/* should never be called */
static void __exit AW2013_Driver_Exit(void) 
{
	if(g_hwctl_device.init_stage == DEV_INIT_ALL)
	{
		device_del(g_hwctl_device.hw_device);
		class_destroy(g_hwctl_device.hw_class);
		cdev_del(g_hwctl_device.hw_cdev);
		unregister_chrdev_region(g_hwctl_device.hwctl_dev_no,1);
	}
		i2c_del_driver(&aw2013_i2c_driver);
}

module_init(AW2013_Driver_Init);
module_exit(AW2013_Driver_Exit);
MODULE_DESCRIPTION("Linux HW direct control driver");
MODULE_AUTHOR("David.wang(softnow@live.cn)");
MODULE_LICENSE("GPL");
