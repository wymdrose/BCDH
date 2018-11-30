
#pragma once

#define IO_A_NUM 4		//一个轴专用IO数量
#define	IO_EX_NUM 3
#define IO_BIT_NUM 16
#define IO_ON	0
#define IO_OFF	1

#define ALM  (1<<0)
#define EL_R (1<<1)
#define EL_L (1<<2)
#define ORG  (1<<4)


#define EL0_R (1<<16)
#define EL1_R (1<<17)
#define EL2_R (1<<18)
#define EL3_R (1<<19)
#define EL4_R (1<<20)

#define EL0_L (1<<24)
#define EL1_L (1<<25)
#define EL2_L (1<<26)
#define EL3_L (1<<27)
#define EL4_L (1<<28)

#define ORG0 (1<<0)
#define ORG1 (1<<1)
#define ORG2 (1<<2)
#define ORG3 (1<<3)
#define ORG4 (1<<4)


#define ALM0 (1<<8)
#define ALM1 (1<<9)
#define ALM2 (1<<10)
#define ALM3 (1<<11)
#define ALM4 (1<<12)



enum IO_CYLINDER
{
	FEED_o, FEED_I,						//送料气缸
	ROTATE_o, ROTATE_I,					//旋转气缸
	CLAMP1_o, CLAMP1_I,					//夹紧气缸1
	CLAMP2_o, CLAMP2_I,					//夹紧气缸2	
	UPDOWN_A_o, UPDOWN_A_I,				//升降气缸1~4
	UPDOWN_B_o, UPDOWN_B_I,				//升降气缸5~8
	UPDOWN_C_1o, UPDOWN_C_1I, UPDOWN_C_2o, UPDOWN_C_2I, UPDOWN_C_3o, UPDOWN_C_3I, UPDOWN_C_4o, UPDOWN_C_4I,	//升降气缸9~12
	UPDOWN_NG1_o, UPDOWN_NG1_I, UPDOWN_NG2_o, UPDOWN_NG2_I, UPDOWN_NG3_o, UPDOWN_NG3_I						//NG标记气缸
};



enum IO_SENSOR		//输出信号
{
	RedLight,			//红灯
	YellowLight,
	GreenLight,
	Buzzer,				//蜂鸣器
	FixtureVacuum,		//治具开真空
	
	LightCurtain,		//光幕信号
	
	LineUnloadOkRun,	//OK线启动

	Fiber1, Fiber2, Fiber3, Fiber4,		//光纤信号
	Fiber5, Fiber6, Fiber7, Fiber8,

	Opposite1, Opposite2, Opposite3,			//对射传感器

	Vacuum1, Vacuum2, Vacuum3, Vacuum4,
	Vacuum5, Vacuum6, Vacuum7, Vacuum8,
	Vacuum9, Vacuum10, Vacuum11, Vacuum12,	//真空

	START,				//启动信号
	STOP,
	RESET,
	EMGSTOP,
	CONFIRM,			//人工确认按钮
	SWITCH_MODE			//生产复检切换
};
