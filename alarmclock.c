#include "STC15F2K60S2.H"
/*******************
引脚定义
*******************/
sbit SEL0=P2^0;			//定义引脚
sbit SEL1=P2^1;
sbit SEL2=P2^2;
sbit SEL3=P2^3;
sbit key1 = P3^2;					//按键1引脚
sbit key2 = P3^3;					//按键2引脚
sbit sbtbeep = P3 ^ 4;  //蜂鸣器
sbit sbtVib = P2 ^ 4;     //振动传感器

/*******************
变量定义
*******************/
#define uint unsigned int
#define ulint unsigned long

int key1_state = 0;			//0：key1未按下			1：key1已按下
int key2_state = 0;			//0：key2未按下			1：key1已按下

uint time=0;				//延时
uint flag=1;				//标志位，区分光和温度

uint l=0;						//执行光的次数
uint t=0;						//执行温度的次数
ulint suml=0;				//光AD值得总和
ulint sumt=0;				//温度AD值的总和

uint date_temp;
int temp=0;					//温度值
uint abs_temp;			//温度绝对值
uint light=0;				//光

uint dark=0;				//黑夜计数
uint white=0;				//白天计数
uint state=0;				//当前状态

int num = 0;				//音调

uint ring=0;				//根据光照实现开关
uint ringopen=1;		//根据按键控制开关

//设置用于显示温度的三个变量
uint temp_bai=0;		//百位
uint temp_shi=0;		//十位
uint temp_ge =0;		//个位

//数码管上显示0-F
char segtable[]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,
								 0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71};
//温度值对应表
int code tempdata[]={239,197,175,160,150,142,135,129,124,120,116,113,109,107,104,101, 
										  99, 97, 95, 93, 91, 90, 88, 86, 85, 84, 82, 81, 80, 78, 77, 76, 
										  75, 74, 73, 72, 71, 70, 69, 68, 67, 67, 66, 65, 64, 63, 63, 62, 
										  61, 61, 60, 59, 58, 58, 57, 57, 56, 55, 55, 54, 54, 53, 52, 52, 
										  51, 51, 50, 50, 49, 49, 48, 48, 47, 47, 46, 46, 45, 45, 44, 44, 
										  43, 43, 42, 42, 41, 41, 41, 40, 40, 39, 39, 38, 38, 38, 37, 37, 
										  36, 36, 36, 35, 35, 34, 34, 34, 33, 33, 32, 32, 32, 31, 31, 31, 
										  30, 30, 29, 29, 29, 28, 28, 28, 27, 27, 27, 26, 26, 26, 25, 25,
										  24, 24, 24, 23, 23, 23, 22, 22, 22, 21, 21, 21, 20, 20, 20, 19, 
										  19, 19, 18, 18, 18, 17, 17, 16, 16, 16, 15, 15, 15, 14, 14, 14, 
										  13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 9, 9, 9, 8, 8, 8, 7, 
										  7, 7, 6, 6,5, 5, 54,4, 3, 3,3, 2, 2, 1, 1, 1, 0, 0, -1, -1, -1, 
										  -2, -2, -3, -3, -4, -4, -5, -5, -6, -6, -7, -7, -8, -8, -9, -9, 
										  -10, -10, -11, -11, -12, -13, -13, -14, -14, -15, -16, -16, -17, 
										  -18, -19, -19, -20, -21, -22, -23, -24, -25, -26, -27, -28, -29, 
										  -30, -32, -33, -35, -36, -38, -40, -43, -46, -50, -55, -63, 361};
void Delay(int n);     //延时函数
void weixuan(char i);  //数码管位的选择
void SEG_Display();    //显示温度，光度
void InitADC_temp();   //初始化温度ADC
void InitADC_light();  //初始化光ADC
void date_processtemp();


/*****************
函数名称Delay
函数说明：延时函数
入口参数：int n
出口参数：无
备注：
*****************/	
void Delay(int n)			
{
	int x;
	while(n--)
	{
		x=60;
		while(x--);
	}
}

/*****************
函数名称weixuan
函数说明：数码管位的选择
入口参数：char i
出口参数：无
备注：
*****************/
void weixuan(char i)	
{
	SEL2=i/4;
	SEL1=i%4/2;
	SEL0=i%2;
}

/*****************
函数名称SEG_Display
函数说明：显示温度，光度
入口参数：无
出口参数：无
备注：
*****************/
void SEG_Display()  
{
  //用于设置温度的位数
	if(temp<0)
	{
		P0=0;
		weixuan(0);				//若温度为负，则首位显示‘-‘
		P0=0x40;
		Delay(10);
	}
	else
	{
		P0=0;
		weixuan(5);
		P0=segtable[temp_bai];
		Delay(10);
	}
	
	P0=0;
	weixuan(6);
	P0=segtable[temp_shi];
	Delay(10);
	
	P0=0;
	weixuan(7);
	P0=segtable[temp_ge];
	Delay(10);
	
	P0=0;
	weixuan(1);					//显示闹钟开关
	P0=segtable[ringopen];
	Delay(10);
	
	P0=0;
	weixuan(3);					//显示闹铃音调
	P0=segtable[num];
	Delay(10); 
}

/*****************
函数名称T_L
函数说明：初始化
入口参数：无
出口参数：无
备注：
*****************/
void T_L()
{
	P0M1=0x00;						//设置P0为推挽模式，点亮数码管
	P0M0=0xff;
	P2M1=0x00;
	P2M0=0xff;						//将P2^3设置为推挽模式，其余为准双向口模式
	SEL3=0;								//熄灭LED灯
	
	IE=0xa8;							//EA=1打开总中断，EADC=1允许A/D转化中断，ET1=1允许T1中断	
	TMOD=0x01;						//使用定时器1，16位不可重装载模式，TH1、TL1全用
	TH1=(65535-1000)/256;//高8位赋初值，定时40000周期
	TL1=(65535-1000)%256;//低8位赋初值
	TR1=1;								//启动定时器1	
}

/*****************
函数名称InitADC_temp
函数说明：初始化温度ADC
入口参数：无
出口参数：无
备注：
*****************/
void InitADC_temp()			 
{
	P1ASF=0xff;						//将P1口作为模拟功能A/D使用
	ADC_RES=0;						//寄存器ADC_RES和ADC_RESL保存A/D转化结果
	ADC_RESL=0;						//初始赋值0
	ADC_CONTR=0x8b;				//ADC_POWER=1打开A/D转换器电源;ADC_START=1启动模拟转换器ADC;CHS=011选择P1^3作为A/D输入使用
	CLK_DIV=0x20;					//ADRJ=1：ADC_RES[1:0]存放高2位ADC结果，ADC_RESL[7:0]存放低8位ADC结果
}

/*****************
函数名称InitADC_light
函数说明：初始化光ADC
入口参数：无
出口参数：无
备注：
*****************/
void InitADC_light()	 
{
	P1ASF=0xff;
	ADC_RES=0;
	ADC_RESL=0;
	ADC_CONTR=0x8c;				//CHS=100选择P1^4作为A/D输入使用
	CLK_DIV=0x20;
}

/*****************
函数名称date_processtemp
函数说明：取出温度的百位、十位、个位
入口参数：无
出口参数：无
备注：
*****************/
void date_processtemp()
{
	if(temp<0)
		abs_temp=-temp;
	else
		abs_temp=temp;
	temp_bai=abs_temp%1000/100;
	temp_shi=abs_temp%100/10;
	temp_ge =abs_temp%10;
}

/*****************
函数名称Timer1_Routine
函数说明：定时器1中断
入口参数：无
出口参数：无
备注：
*****************/
void Timer1_Routine() interrupt 3	//3为定时器1中断编号
{

	if(flag==1)
	{
		InitADC_light();		//初始化光
	}
	else
	{
		InitADC_temp();			//初始化温度
	}
	flag=-flag;

	if(ringopen==1){			//判断按键开关
		if(light<40){				//判断之前是否黑夜
			if(dark<2000){
			dark++;}					//利用dark进行计数
			state=0;					//状态为黑夜
		}else{
			if(white<2000){   //判断现在是否白天
			white++;}					//利用white进行计数
			state=1;					//状态为白天
		}	
		if(white==2000&&dark==2000&&state==1){//判断从黑夜进入白天且当前为白天
			dark=0;						//dark white清零且打开开关
			white=0;
			ring=1;
		}
	}
	
	if(ring==1&&ringopen==1){//光照开关且按键开关为开时蜂鸣器启动
		sbtbeep = ~sbtbeep;
	}else{
		sbtbeep = 0;
	}
	if(sbtVib == 0)
	{
		Delay(100);					   	//延时消除抖动
		if(sbtVib == 0)
		{
			ring=0;						//通过振动关闭闹钟
		}
	}
}

/*****************
函数名称adc_isr
函数说明：AD中断
入口参数：无
出口参数：无
备注：
*****************/
void adc_isr() interrupt 5 using 1
{
  time++;
	IE=0x00;							//关闭中断
	
	if(time>2000)      //取多次值求平均值减小误差
	{
		if(flag==1)										//此时flag=1执行温度部分
		{	
			date_temp=(sumt+t/2)/t;			//四舍五入
			temp=tempdata[date_temp-1]; //查找对应的AD的温度值
			sumt=0;
			t=0;
			time=0;
			date_processtemp();	
		}

		if(flag==-1)									//此时flag=-1执行光部分
		{
			light=(suml+l/2)/l;					//四舍五入
			suml=0;
			l=0;
			time=0;
		}    
	}

	if(flag==1)
	{
		//对应温度的数据处理
		t++;
		date_temp=(ADC_RES*256+ADC_RESL)/4;//由10位AD值转换为8位AD值
		sumt+=date_temp;						//求t次AD值的和

	}
	if(flag==-1)
	{
		//处理光部分的数据
		l++;
		suml+=ADC_RES*256+ADC_RESL;//求l次AD值的和
	}

	ADC_CONTR&=~0X10;  					 //转换完成后，ADC_FLAG清零
	ADC_CONTR|=0X08;	 					 //转换完成后，ADC_START赋1
	IE=0xa8;					 					 //打开中断
}

/*****************
函数名称main
函数说明：主函数
入口参数：无
出口参数：无
备注：
*****************/
void main()
{
	T_L();
	while(1)
	{
		if(key1==0)
		{
			if(key1_state == 0)				//判断按键1是否按下，已经按下了就不进行括号内操作
			{
				Delay(10);					 	  //延时消除抖动
				if(key1==0)
				{	
					num++;				   			//声调改变
					if(num==10){
						num = 0;
						TH1 = (65535-1000)/256;
					}
					TH1 = (65535-1000)/256-num;				//减小重装值，从而减小定时器中断（蜂鸣器振动）频率
					key1_state = 1;				//设置按键1为已按下
				}
			}
		}
		else
			key1_state = 0;
		
		if(key2==0)
		{
			if(key2_state == 0)				//判断按键2是否按下，已经按下了就不进行括号内操作
			{
				Delay(10);					   	//延时消除抖动
				if(key2==0)
				{
					if(ringopen==1&&ring==0){ringopen=0;}		
					else{ringopen=1;}		//蜂鸣器开关
					key2_state = 1;				//设置按键3为已按下
				}
			}
		}
		else
			key2_state = 0;
		
		SEG_Display();
		
	}
}