#include "hsmeasure.h"

#include "bcdh_gap.h"
#include "bcdh_step.h"

extern QString mPath;
QMap<QString, QString> ioMap;
extern BCDH_step* mpBCDH_step;

void HSMeasure::onCheckBoxIo() //????
{
	Sleep(0);
	dmc_write_outport(mCardNo, 0, 0);
}

bool getIoMapFromCfg()
{
	QString fileName = QCoreApplication::applicationDirPath() +"/cfg/ioMap.csv";

	QFile csvFile(fileName);

	QStringList CSVList;

	if (!csvFile.open(QIODevice::ReadOnly))
	{
		return false;
	}

	QTextStream stream(&csvFile);

	while (!stream.atEnd())
	{
		CSVList.push_back(stream.readLine());
	}
	csvFile.close();

	for (size_t i = 0; i < CSVList.size(); i++)
	{
		QStringList oneLine = CSVList[i].split(",");

		ioMap[oneLine[1]] = oneLine[0];
	}

	return true;
}

bool HSMeasure::ioOut(const QString io, const WORD on_off)
{
	if (io.left(3) != "OUT" || io.size() < 4)
	{
		return false;
	}

	if (io.size() < 5)
	{
		auto a = io.right(1).toInt(0, 16);
		dmc_write_outbit(mCardNo, io.right(1).toInt(0, 16), on_off);
	}
	else
	{
		dmc_write_can_outbit(mCardNo, io.mid(3,1).toInt(), io.right(1).toInt(0,16), on_off);
	}

	return true;
}

short HSMeasure::ioGet(const QString io)
{
	if (io.left(2) == "IN")
	{
		if (3 == io.size())
		{
			return dmc_read_inbit(mCardNo, io.right(1).toInt(0, 16));
		}
		else
		{
			auto a = io.mid(2, 1).toInt();
			auto b = io.right(1).toInt(0, 16);
			return dmc_read_can_inbit(mCardNo, io.mid(2, 1).toInt(), io.right(1).toInt(0, 16));
		}
	}

	else if(io.left(3) == "OUT")
	{
		if (3 == io.size())
		{
			return dmc_read_outbit(mCardNo, io.right(1).toInt(0, 16));
		}
		else
		{
			return dmc_read_can_outbit(mCardNo, io.mid(3, 1).toInt(), io.right(1).toInt(0, 16));
		}
	}

	else
		return -1;
}

void HSMeasure::cylinderMove(const IO_CYLINDER name)	//气缸运动
{
	switch (name)
	{
	case FEED_o:
		ioOut("OUT7", IO_OFF);
		break;

	case FEED_I:
		ioOut("OUT7", IO_ON);
		break;
	case CLAMP1_o:
		ioOut("OUT13", IO_OFF);
		break;
	case CLAMP1_I:
		ioOut("OUT13", IO_ON);
		break;
	case CLAMP2_o:
		ioOut("OUT1B", IO_ON);
		break;
	case CLAMP2_I:	//夹紧
		ioOut("OUT1B", IO_OFF);
		break;

	case ROTATE_o:
		ioOut("OUT2F", IO_OFF);
		ioOut("OUT30", IO_ON);
		break;

	case ROTATE_I:
		ioOut("OUT30", IO_OFF);
		ioOut("OUT2F", IO_ON);
		break;

	case UPDOWN_A_o:
		ioOut("OUT9", IO_OFF);
		ioOut("OUTA", IO_ON);
		ioOut("OUT32", IO_OFF);
		ioOut("OUT33", IO_ON);
		break;
	case UPDOWN_A_I:
		ioOut("OUTA", IO_OFF);
		ioOut("OUT9", IO_ON);
		ioOut("OUT33", IO_OFF);
		ioOut("OUT32", IO_ON);
		break;

	case UPDOWN_B_o :
		ioOut("OUT15", IO_OFF);
		ioOut("OUT16", IO_ON);
		ioOut("OUT34", IO_OFF);
		ioOut("OUT35", IO_ON);
		break;
	case UPDOWN_B_I:
		ioOut("OUT16", IO_OFF);
		ioOut("OUT15", IO_ON);
		ioOut("OUT35", IO_OFF);
		ioOut("OUT34", IO_ON);
		break;

	case UPDOWN_C_1o:
		ioOut("OUT1D", IO_OFF);
		ioOut("OUT1E", IO_ON);
		break;
	case UPDOWN_C_1I:
		ioOut("OUT1E", IO_OFF);
		ioOut("OUT1D", IO_ON);
		break;

	case UPDOWN_C_2o:
		ioOut("OUT1F", IO_OFF);
		ioOut("OUT20", IO_ON);
		break;
	case UPDOWN_C_2I:
		ioOut("OUT20", IO_OFF);
		ioOut("OUT1F", IO_ON);
		break;

	case UPDOWN_C_3o:
		ioOut("OUT21", IO_OFF);
		ioOut("OUT22", IO_ON);
		break;
	case UPDOWN_C_3I:
		ioOut("OUT22", IO_OFF);
		ioOut("OUT21", IO_ON);
		break;


	case UPDOWN_C_4o:
		ioOut("OUT23", IO_OFF);
		ioOut("OUT24", IO_ON);
		break;
	case UPDOWN_C_4I:
		ioOut("OUT24", IO_OFF);
		ioOut("OUT23", IO_ON);
		break;

	case UPDOWN_NG1_o:
		ioOut("OUT29", IO_OFF);
		ioOut("OUT2A", IO_ON);
		break;
	case UPDOWN_NG1_I:
		ioOut("OUT2A", IO_OFF);
		ioOut("OUT29", IO_ON);
		break;

	case UPDOWN_NG2_o:
		ioOut("OUT2B", IO_OFF);
		ioOut("OUT2C", IO_ON);
		break;
	case UPDOWN_NG2_I:
		ioOut("OUT2C", IO_OFF);
		ioOut("OUT2B", IO_ON);
		break;

	case UPDOWN_NG3_o:
		ioOut("OUT2D", IO_OFF);
		ioOut("OUT2E", IO_ON);
		break;
	case UPDOWN_NG3_I:
		ioOut("OUT2E", IO_OFF);
		ioOut("OUT2D", IO_ON);
		break;
		
	default:
		break;
	}
}

short HSMeasure::cylinderCheck(const IO_CYLINDER name)		//气缸到位检查
{
	int re;
	switch (name)
	{

	case FEED_o:
		return ioGet("IN11");
	case FEED_I:
		return ioGet("IN11");
//		return ioGet("IN12");
	case CLAMP1_o:
		return ioGet("IN1C");
	case CLAMP1_I:
		return ioGet("IN1C");

	case CLAMP2_o:
		return ioGet("IN2A");
	case CLAMP2_I:
		return ioGet("IN2A");

	case ROTATE_o:
		return ioGet("IN36");
	case ROTATE_I:
		return ioGet("IN35");

	case UPDOWN_A_o:
		re = ioGet("IN13");
		re = ioGet("IN14");
		re = ioGet("IN15");
		re = ioGet("IN16");
		return ioGet("IN13") || ioGet("IN14") || ioGet("IN15") || ioGet("IN16");

	case UPDOWN_B_o:
		return ioGet("IN1D") || ioGet("IN1E") || ioGet("IN1F") || ioGet("IN20");
	case UPDOWN_C_1o:
		return ioGet("IN25");
	case UPDOWN_C_2o:
		return ioGet("IN26");
	case UPDOWN_C_3o:
		return ioGet("IN27");
	case UPDOWN_C_4o:
		return ioGet("IN28");
	default:
		break;
	}

	return -1;
}

void HSMeasure::sensorOut(const IO_SENSOR name, const short on_off)
{
	switch (name)
	{
	case RedLight:		ioOut("OUT0", on_off);		break;
	case YellowLight:	ioOut("OUT1", on_off);		break;
	case GreenLight:	ioOut("OUT2", on_off);		break;
	case Buzzer:		ioOut("OUT3", on_off);		break;
	case LineUnloadOkRun: ioOut("OUT4", on_off);		break;

	case FixtureVacuum:	ioOut("OUT6", on_off);	break;
	
	case Vacuum1:		ioOut("OUTB", on_off);		break;
	case Vacuum2:		ioOut("OUT10", on_off);		break;
	case Vacuum3:		ioOut("OUT11", on_off);		break;
	case Vacuum4:		ioOut("OUT12", on_off);		break;

	case Vacuum5:		ioOut("OUT17", on_off);		break;
	case Vacuum6:		ioOut("OUT18", on_off);		break;
	case Vacuum7:		ioOut("OUT19", on_off);		break;
	case Vacuum8:		ioOut("OUT1A", on_off);		break;

	case Vacuum9:		ioOut("OUT25", on_off);		break;
	case Vacuum10:		ioOut("OUT26", on_off);		break;
	case Vacuum11:		ioOut("OUT27", on_off);		break;
	case Vacuum12:		ioOut("OUT28", on_off);		break;
	
	default:
		break;
	}
}

short HSMeasure::sensorIn(const IO_SENSOR name)
{
	
	switch (name)
	{
	case START:			return ioGet("IN0");	
	case STOP:			return ioGet("IN1");
	case RESET:			return ioGet("IN2");
	case EMGSTOP:			return ioGet("IN3");
	case CONFIRM:			return ioGet("IN4");		
	case SWITCH_MODE:		return ioGet("IN5");
	
	case	LightCurtain:	return ioGet("IN33");
	
	case Fiber1:	return ioGet("IN6");
	case Fiber2:	return ioGet("IN7");
	case Fiber3:	return ioGet("IN8");
	case Fiber4:	return ioGet("IN9");
	
	case Fiber5:	return ioGet("IN37");
	case Fiber6:	return ioGet("IN38");
	case Fiber7:	return ioGet("IN39");
	case Fiber8:	return ioGet("IN3A");

	case Opposite1: return ioGet("IN2E");
	case Opposite2: return ioGet("IN2F");
	case Opposite3: return ioGet("IN30");

	case FixtureVacuum:	return ioGet("IN10");
	
	case Vacuum1:		return ioGet("INA");
	case Vacuum2:		return ioGet("INB");
	case Vacuum3:		return ioGet("INC");
	case Vacuum4:		return ioGet("IND");
						
	case Vacuum5:		return ioGet("IN17");
	case Vacuum6:		return ioGet("IN18");
	case Vacuum7:		return ioGet("IN19");
	case Vacuum8:		return ioGet("IN1A");
						
	case Vacuum9:		return ioGet("IN21");
	case Vacuum10:		return ioGet("IN22");
	case Vacuum11:		return ioGet("IN23");
	case Vacuum12:		return ioGet("IN24");


	default:
		break;
	}

	return -1;
}

void HSMeasure::initUiIo()
{
	getIoMapFromCfg();

	//专用IO
	for (size_t i = 0; i < AXIS_NUM; i++)
	{
		for (size_t j = 0; j < IO_A_NUM; j++)
		{
			pBitInS[i][j] = findChild<QCheckBox*>("checkBoxInA" + QString::number(i + 1) + "_" + QString::number(j + 1));
			pBitOutS[i][j] = findChild<QCheckBox*>("checkBoxOutA" + QString::number(i + 1) + "_" + QString::number(j + 1));
			
			connect(pBitOutS[i][j], &QCheckBox::clicked, [this, i, j]()
			{
				/*
				int b = dmc_axis_io_status(mCardNo, i);

				dmc_write_outport(mCardNo, 1, 0xFFFFFFFF);

				dmc_write_erc_pin(mCardNo, i, 1);
				dmc_write_erc_pin(mCardNo, i, 0);
				int a = dmc_read_erc_pin(mCardNo, i);
				*/
			//	dmc_write_erc_pin(mCardNo, i, 1);
				
				switch (j)
				{
				case 0:	//
					pBitOutS[i][j]->isChecked() ? dmc_write_sevon_pin(mCardNo, i, IO_ON) : dmc_write_sevon_pin(mCardNo, i, IO_OFF);
					break;
				case 1: 
					pBitOutS[i][j]->isChecked() ? dmc_write_erc_pin(mCardNo, i, IO_ON) : dmc_write_erc_pin(mCardNo, i, IO_OFF);
					
					break;
				case 2:
					
					break;
				case 3:
					break;
				
				default:
					break;
				}
			});
		
		}
	}

	pBitInS[0][0]->setText(QStringLiteral("X1轴（CCD轴）左极限"));
	pBitInS[0][1]->setText(QStringLiteral("X1轴（CCD轴）右极限"));
	pBitInS[0][2]->setText(QStringLiteral("X1轴（CCD轴）原点"));
	pBitInS[0][3]->setText(QStringLiteral("X1轴（CCD轴）报警"));

	pBitInS[1][0]->setText(QStringLiteral("X2轴（激光轴）左极限"));
	pBitInS[1][1]->setText(QStringLiteral("X2轴（激光轴）右极限"));
	pBitInS[1][2]->setText(QStringLiteral("X2轴（激光轴）原点"));
	pBitInS[1][3]->setText(QStringLiteral("X2轴（激光轴）报警"));

	pBitInS[2][0]->setText(QStringLiteral("X3轴（皮带步进）左极限"));
	pBitInS[2][1]->setText(QStringLiteral("X3轴（皮带步进）右极限"));
	pBitInS[2][2]->setText(QStringLiteral("X3轴（皮带步进）原点"));
	pBitInS[2][3]->setText(QStringLiteral("X3轴（皮带步进）报警"));

	pBitInS[3][0]->setText(QStringLiteral("Y1轴（上下料轴）左极限"));
	pBitInS[3][1]->setText(QStringLiteral("Y1轴（上下料轴）右极限"));
	pBitInS[3][2]->setText(QStringLiteral("Y1轴（上下料轴）原点"));
	pBitInS[3][3]->setText(QStringLiteral("Y1轴（上下料轴）报警"));

	pBitInS[4][0]->setText(QStringLiteral("Y2轴（上下料轴）左极限"));
	pBitInS[4][1]->setText(QStringLiteral("Y2轴（上下料轴）右极限"));
	pBitInS[4][2]->setText(QStringLiteral("Y2轴（上下料轴）原点"));
	pBitInS[4][3]->setText(QStringLiteral("Y2轴（上下料轴）报警"));
	//
	pBitOutS[0][0]->setText(QStringLiteral("X1轴（CCD轴）伺服使能"));
	pBitOutS[0][1]->setText(QStringLiteral("X1轴（CCD轴）伺服报警复位"));
	pBitOutS[0][2]->setText(QStringLiteral("X1轴（CCD轴）伺服脉冲输出"));
	pBitOutS[0][3]->setText(QStringLiteral("X1轴（CCD轴）伺服方向"));
								   
	pBitOutS[1][0]->setText(QStringLiteral("X2轴（激光轴）伺服使能"));
	pBitOutS[1][1]->setText(QStringLiteral("X2轴（激光轴）伺服报警复位"));
	pBitOutS[1][2]->setText(QStringLiteral("X2轴（激光轴）伺服脉冲输出"));
	pBitOutS[1][3]->setText(QStringLiteral("X2轴（激光轴）伺服方向"));
							  
	pBitOutS[2][0]->setText(QStringLiteral("X3轴（皮带步进）伺服使能"));
	pBitOutS[2][1]->setText(QStringLiteral("X3轴（皮带步进）伺服报警复位"));
	pBitOutS[2][2]->setText(QStringLiteral("X3轴（皮带步进）伺服脉冲输出"));
	pBitOutS[2][3]->setText(QStringLiteral("X3轴（皮带步进）伺服方向"));
							   
	pBitOutS[3][0]->setText(QStringLiteral("Y1轴（搬运轴）伺服使能"));
	pBitOutS[3][1]->setText(QStringLiteral("Y1轴（搬运轴）伺服报警复位"));
	pBitOutS[3][2]->setText(QStringLiteral("Y1轴（搬运轴）伺服脉冲输出"));
	pBitOutS[3][3]->setText(QStringLiteral("Y1轴（搬运轴）伺服方向"));
							   
	pBitOutS[4][0]->setText(QStringLiteral("Y2轴（上下料轴）伺服使能"));
	pBitOutS[4][1]->setText(QStringLiteral("Y2轴（上下料轴）伺服报警复位"));
	pBitOutS[4][2]->setText(QStringLiteral("Y2轴（上下料轴）伺服脉冲输出"));
	pBitOutS[4][3]->setText(QStringLiteral("Y2轴（上下料轴）伺服方向"));


	//通用IO 主板
	for (size_t i = 0; i < IO_BIT_NUM; i++)
	{
		pBitInM[i] = findChild<QCheckBox*>("checkBoxInM_" + QString::number(i + 1));
		pBitOutM[i] = findChild<QCheckBox*>("checkBoxOutM_" + QString::number(i + 1));
		
		pBitInM[i]->setText("[IN" + QString::number(i, 16).toUpper() + "] " + ioMap.value("IN" + QString::number(i, 16).toUpper()));
		pBitOutM[i]->setText("[OUT" + QString::number(i, 16).toUpper() + "] " + ioMap.value("OUT" + QString::number(i, 16).toUpper()));

		connect(pBitOutM[i], &QCheckBox::clicked, [this, i]()
		{
			ioMutex.lock();
			pBitOutM[i]->isChecked() ? dmc_write_outbit(mCardNo, i, IO_ON) : dmc_write_outbit(mCardNo, i, IO_OFF);
			ioMutex.unlock();
		});	
	}

	//CAN
	for (size_t i = 0; i < IO_EX_NUM; i++)
	{
		for (size_t j = 0; j < IO_BIT_NUM; j++)
		{
			pBitInG[i][j] = findChild<QCheckBox*>("checkBoxInG" + QString::number(i + 1) + "_" + QString::number(j + 1));
			pBitOutG[i][j] = findChild<QCheckBox*>("checkBoxOutG" + QString::number(i + 1) + "_" + QString::number(j + 1));
	
			
			pBitInG[i][j]->setText("[IN" + QString("%1").arg(i + 1) + QString::number(j, 16).toUpper() + "] " + ioMap.value("IN" + QString("%1").arg(i + 1) + QString::number(j, 16).toUpper()));
			pBitOutG[i][j]->setText("[OUT" + QString("%1").arg(i + 1) + QString::number(j, 16).toUpper() + "] " + ioMap.value("OUT" + QString("%1").arg(i + 1) + QString::number(j, 16).toUpper()));

			connect(pBitOutG[i][j], &QCheckBox::clicked, [this, i, j]()
			{
				ioMutex.lock();
				auto re = pBitOutG[i][j]->isChecked() ? dmc_write_can_outbit(mCardNo, i + 1, j, IO_ON) : dmc_write_can_outbit(mCardNo, i + 1, j, IO_OFF);
				ioMutex.unlock();
			});
		}
	}
	
}

void HSMeasure::onIoTimer()
{
	//专用IO
	for (size_t i = 0; i < 5; i++)
	{
		DWORD inAxisS = dmc_axis_io_status(mCardNo, i);

		if (inAxisS & EL_L){ pBitInS[i][0]->setChecked(true);}	else{ pBitInS[i][0]->setChecked(false);}
		if (inAxisS & EL_R){ pBitInS[i][1]->setChecked(true); }	else{ pBitInS[i][1]->setChecked(false);}
		if (inAxisS & ORG) {pBitInS[i][2]->setChecked(true); colorSignal(pBitInS[i][2], "QCheckBox{color:green}");}	else{pBitInS[i][2]->setChecked(false); colorSignal(pBitInS[i][2], "QCheckBox{color:}");}
		if (inAxisS & ALM) {pBitInS[i][3]->setChecked(true); colorSignal(pBitInS[i][3], "QCheckBox{color:red}");}			else{pBitInS[i][3]->setChecked(false); colorSignal(pBitInS[i][3], "QCheckBox{color:}");}
	}
	
	
	/*
	DWORD inport0 = dmc_read_inport(mCardNo, 0);
	DWORD inport1 = dmc_read_inport(mCardNo, 1);

	(inport0 & EL0_L) ? pBitInS[0][1]->setChecked(false) : pBitInS[0][0]->setChecked(true);
	(inport0 & EL0_R) ? pBitInS[0][2]->setChecked(false) : pBitInS[0][1]->setChecked(true);
	(inport1 & ORG0) ?  pBitInS[0][3]->setChecked(false) : pBitInS[0][2]->setChecked(true);
	(inport1 & ALM0) ?  pBitInS[0][4]->setChecked(false) : pBitInS[0][3]->setChecked(true);

	(inport0 & EL1_L) ? pBitInS[1][1]->setChecked(false) : pBitInS[1][0]->setChecked(true);
	(inport0 & EL1_R) ? pBitInS[1][2]->setChecked(false) : pBitInS[1][1]->setChecked(true);
	(inport1 & ORG1) ?  pBitInS[1][3]->setChecked(false) : pBitInS[1][2]->setChecked(true);
	(inport1 & ALM1) ?  pBitInS[1][4]->setChecked(false) : pBitInS[1][3]->setChecked(true);

	(inport0 & EL2_L) ? pBitInS[2][1]->setChecked(false) : pBitInS[2][0]->setChecked(true);
	(inport0 & EL2_R) ? pBitInS[2][2]->setChecked(false) : pBitInS[2][1]->setChecked(true);
	(inport1 & ORG2) ? pBitInS[2][3]->setChecked(false) : pBitInS[2][2]->setChecked(true);
	(inport1 & ALM2) ? pBitInS[2][4]->setChecked(false) : pBitInS[2][3]->setChecked(true);

	(inport0 & EL3_L) ? pBitInS[3][1]->setChecked(false) : pBitInS[3][0]->setChecked(true);
	(inport0 & EL3_R) ? pBitInS[3][2]->setChecked(false) : pBitInS[3][1]->setChecked(true);
	(inport1 & ORG3) ? pBitInS[3][3]->setChecked(false) : pBitInS[3][2]->setChecked(true);
	(inport1 & ALM3) ? pBitInS[3][4]->setChecked(false) : pBitInS[3][3]->setChecked(true);

	(inport0 & EL4_L) ? pBitInS[4][1]->setChecked(false) : pBitInS[4][0]->setChecked(true);
	(inport0 & EL4_R) ? pBitInS[4][2]->setChecked(false) : pBitInS[4][1]->setChecked(true);
	(inport1 & ORG4) ? pBitInS[4][3]->setChecked(false) : pBitInS[4][2]->setChecked(true);
	(inport1 & ALM4) ? pBitInS[4][4]->setChecked(false) : pBitInS[4][3]->setChecked(true);
	*/

	//主板通用IO
	for (size_t i = 0; i < IO_BIT_NUM; i++)
	{
		ioMutex.lock();
		
		if(dmc_read_inbit(mCardNo, i))
		{
			pBitInM[i]->setChecked(false); colorSignal(pBitInM[i], "QCheckBox{color:}");
		} 
		else 
		{
			pBitInM[i]->setChecked(true); colorSignal(pBitInM[i], "QCheckBox{color:green}");
		}
		
		if (dmc_read_outbit(mCardNo, i))
		{ 
			pBitOutM[i]->setChecked(false); colorSignal(pBitOutM[i], "QCheckBox{color:}");
		}
		else 
		{ 
			pBitOutM[i]->setChecked(true); colorSignal(pBitOutM[i], "QCheckBox{color:green}");
		}

		ioMutex.unlock();
	}

	//扩展模块IO
	for (size_t i = 0; i < IO_EX_NUM; i++)
	{
		for (size_t j = 0; j < IO_BIT_NUM; j++)
		{		
			ioMutex.lock();

			if(dmc_read_can_inbit(mCardNo, i + 1, j) )
			{
				pBitInG[i][j]->setChecked(false);	colorSignal(pBitInG[i][j], "QCheckBox{color:}");
			}
			else
			{
				pBitInG[i][j]->setChecked(true); 	colorSignal(pBitInG[i][j], "QCheckBox{color:green}");
			}

			if(dmc_read_can_outbit(mCardNo, i + 1, j))
			{ 
				pBitOutG[i][j]->setChecked(false);	colorSignal(pBitOutG[i][j], "QCheckBox{color:}");
			}
			else
			{
				pBitOutG[i][j]->setChecked(true);	 colorSignal(pBitOutG[i][j], "QCheckBox{color:green}");
			}

			ioMutex.unlock();
		}
	}

	if (1 == dmc_read_can_inbit(mCardNo, 3, 11))
		colorSignal(ui.pushButton_laserSafePosition, "QPushButton:{background:lightgreen}");
	else
		colorSignal(ui.pushButton_laserSafePosition, "QPushButton:{background:}");
		

//    mpBCDH_step->ShowStepValueToView();
	QApplication::processEvents();
}

void HSMeasure::showValueToUi(const FIX_VALUE& tFixValue)
{
	static unsigned int lineNo = 0;		//行数

	
	for (size_t i = 0; i <= SlotposNo4; i++)
	{
		QStringList oneLine;

		for (size_t j = 0; j <= GapJ; j++)
		{
			oneLine.push_back(QString("%1").arg(tFixValue.plug[i].gap[j]));
		}

		for (size_t j = 0; j <= StepJ; j++)
		{
			oneLine.push_back(QString("%1").arg(tFixValue.plug[i].step[j]));
		}

		switch (tFixValue.plug[i].ok_ng)
		{

		case 0:
			oneLine.push_back("OK");
		case 1:
			oneLine.push_back("GAP NG");
		case 2:
			oneLine.push_back("STEP NG");
		case 3:
			oneLine.push_back("GAP&STEP NG");
		default:
			break;
		}

		
		for (size_t j = 0; j < oneLine.size(); j++)
		{
			model_result->setItem(lineNo, j, new QStandardItem(oneLine[j]));
		}
		
		lineNo++;
	}
	
	QApplication::processEvents();
}

void HSMeasure::showLogToUi()
{
	static unsigned int lineNo = 0;

	_FILECLASS::CSV_FILE tCsv(mPath + "/Log/result_" + QDate::currentDate().toString("yyyy-MM-dd") + ".csv");

	QStringList oneLine = tCsv.getLast().split(",");

	for (size_t j = 0; j < oneLine.size(); j++)
	{
		model_result->setItem(lineNo, j, new QStandardItem(oneLine[j]));
	}

	lineNo+=4;

	QApplication::processEvents();
}


void HSMeasure::saveStepToLog(std::array<std::array<float, StepJ + 1>, SlotposNo4 + 1> stepValues)
{

	_FILECLASS::CSV_FILE tCsv(mPath + "/Log/step_" + QDate::currentDate().toString("yyyy-MM-dd") + ".csv");

	QStringList tList;

	for (size_t i = 0; i < SlotposNo4 + 1; i++)
	{
		tList.clear();
		for (size_t j = 0; j < StepJ + 1; j++)
		{
			tList.push_back(QString("%1").arg(stepValues[i][j]));
		}

		tList.push_back(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

		tCsv.append(tList);
	}
}

void HSMeasure::saveGapToLog(std::array<std::array<float, StepJ + 1>, SlotposNo4 + 1> gapValues)
{
	_FILECLASS::CSV_FILE tCsv(mPath + "/Log/gap_" + QDate::currentDate().toString("yyyy-MM-dd") + ".csv");

	QStringList tList;

	for (size_t i = 0; i < SlotposNo4 + 1; i++)
	{
		tList.clear();
		for (size_t j = 0; j < StepJ + 1; j++)
		{
			tList.push_back(QString("%1").arg(gapValues[i][j]));
		}

		tList.push_back(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

		tCsv.append(tList);
	}
}

void HSMeasure::saveValueToLog(const FIX_VALUE& fixValue)
{
	_FILECLASS::CSV_FILE tCsv(mPath + "/Log/result_" + QDate::currentDate().toString("yyyy-MM-dd") + ".csv");

	QStringList tList;

	for (size_t i = 0; i < SlotposNo4 + 1; i++)
	{
		tList.clear();

		if (false == flagSlot[i])
		{
			for (size_t j = 0; j < GapJ + StepJ + 2; j++)
			{
				tList.push_back("#");
			}		
		}
		else
		{
			for (size_t j = 0; j < GapJ + 1; j++)
			{
				tList.push_back(QString("%1").arg(fixValue.plug[i].gap[j]));
			}

			for (size_t j = 0; j < StepJ + 1; j++)
			{
				tList.push_back(QString("%1").arg(fixValue.plug[i].step[j]));
			}
		}

		tList.push_back(QString("%1").arg(fixValue.plug[i].ok_ng));

		tList.push_back(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

		tCsv.append(tList);
	}

}



