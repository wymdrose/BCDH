
#include "hsmeasure.h"
#include "Lib/login.hpp"

void HSMeasure::login()
{
	connect(ui.pushButtonPower, &QCheckBox::clicked, [this]()
	{
		bool loginFlag = false;		

		int index = ui.comboBoxPower->currentIndex();

		InputDlg Dialog(loginFlag, index);

		Dialog.exec();

		if (true == loginFlag && 0 == index)
		{
			ui.tabWidget->setTabEnabled(0, true);
			ui.tabWidget->setTabEnabled(1, false);
			ui.tabWidget->setTabEnabled(2, false);
		}

		if (true == loginFlag && 1 == index)
		{
			ui.tabWidget->setTabEnabled(0, true);
			ui.tabWidget->setTabEnabled(1, true);
			ui.tabWidget->setTabEnabled(2, false);
		}

		if (true == loginFlag && 2 == index)
		{
			ui.tabWidget->setTabEnabled(0, true);
			ui.tabWidget->setTabEnabled(1, true);
			ui.tabWidget->setTabEnabled(2, true);
		}

	});

}



