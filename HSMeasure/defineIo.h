
#pragma once

#define IO_A_NUM 4		//һ����ר��IO����
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
	FEED_o, FEED_I,						//��������
	ROTATE_o, ROTATE_I,					//��ת����
	CLAMP1_o, CLAMP1_I,					//�н�����1
	CLAMP2_o, CLAMP2_I,					//�н�����2	
	UPDOWN_A_o, UPDOWN_A_I,				//��������1~4
	UPDOWN_B_o, UPDOWN_B_I,				//��������5~8
	UPDOWN_C_1o, UPDOWN_C_1I, UPDOWN_C_2o, UPDOWN_C_2I, UPDOWN_C_3o, UPDOWN_C_3I, UPDOWN_C_4o, UPDOWN_C_4I,	//��������9~12
	UPDOWN_NG1_o, UPDOWN_NG1_I, UPDOWN_NG2_o, UPDOWN_NG2_I, UPDOWN_NG3_o, UPDOWN_NG3_I						//NG�������
};



enum IO_SENSOR		//����ź�
{
	RedLight,			//���
	YellowLight,
	GreenLight,
	Buzzer,				//������
	FixtureVacuum,		//�ξ߿����
	
	LightCurtain,		//��Ļ�ź�
	
	LineUnloadOkRun,	//OK������

	Fiber1, Fiber2, Fiber3, Fiber4,		//�����ź�
	Fiber5, Fiber6, Fiber7, Fiber8,

	Opposite1, Opposite2, Opposite3,			//���䴫����

	Vacuum1, Vacuum2, Vacuum3, Vacuum4,
	Vacuum5, Vacuum6, Vacuum7, Vacuum8,
	Vacuum9, Vacuum10, Vacuum11, Vacuum12,	//���

	START,				//�����ź�
	STOP,
	RESET,
	EMGSTOP,
	CONFIRM,			//�˹�ȷ�ϰ�ť
	SWITCH_MODE			//���������л�
};
