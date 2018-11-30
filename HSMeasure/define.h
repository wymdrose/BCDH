
#pragma once

#define AXIS_NUM  5
#define CAN_OPEN 1
#define CAN_CLOSE 0
#define USA_UP_VAL  0.080
#define DEFAUL_HOME_TIME_OUT  100000
#define DEFAUL_MOVE_TIME_OUT  100000
#define AUTO_MOVE_TIME_OUT  5000
//#define CYLINDER_DELAY		2000

#define DMC_SERV_ENABLE  0
#define DMC_SERV_DISABLE  1
#define LASER_VALUE_NUM 800


enum MS_RESULT_TYPE
{
	MsResultOk, MsResultNgGap, MsResultNgStep, MsResultNgBoth, MsSlotNull
};

enum SLOTPOS_NO
{
	SlotposNo1, SlotposNo2, SlotposNo3, SlotposNo4
};

enum GAP_NO
{
	GapA, GapB, GapC, GapD, GapE, GapF, GapG, GapH, GapI, GapJ
};

enum LASER_AXIS_POS
{
	laserPos1, laserPos2, laserPos3, laserPos4, laserPos5, laserPos6, laserPos7, laserPos8, laserPos9, laserPos10, laserPos11
};

enum LASER_NO
{
	laserNo1, laserNo2, laserNo3, laserNo4, laserNo5, laserNo6
};


enum STEP_NO
{
	StepA, StepB, StepC, StepD, StepE, StepF, StepG, StepH, StepI, StepJ
};


struct ONE_PLUG_VALUE
{
	float gap[GapJ + 1];
	float step[StepJ + 1];
	int ok_ng;
};

struct FIX_VALUE
{
	ONE_PLUG_VALUE plug[SlotposNo4 + 1];
};


struct SPEED_FROM_INI
{
	double  ccdHomeSpeed;
	double	laserHomeSpeed;
	double	platHomeSpeed;
	double	rotateHomeSpeed;
	double	unloadHomeSpeed;

	double	ccdJogSpeed;
	double	laserJogSpeed;
	double	platJogSpeed;
	double	rotateJogSpeed;
	double	unloadJogSpeed;
	int		jogPulse;

	double	ccdAutoSpeed;
	double	laserAutoSpeed;
	double	rotateAutoSpeed;
	double	platAutoSpeed;
	double	unloadAutoSpeed;
};

struct POSION_FROM_INI
{
	long LaserSafePosition;
	long loadPositionY1;
	long unloadPositionY1;
	long loadPositionY2;
	long unloadPositionY2;
	long stepPositionY2;
	long ngPositionY2;
	long okPositionY2;
	long ngBreakPositionHome;
	long ngBreakPositionBegin;
	long ngBreakPositionSpace;

	int basePositionLaser;

	int basePositionX1;
	int basePositionY1;
	int basePositionX2;
	int basePositionY2;
};
