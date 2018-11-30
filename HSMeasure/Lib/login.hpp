
#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QStringList>
#include <QMessageBox>

class InputDlg : public QDialog
{
	const QString  passWord1 = "111111";
	const QString  passWord2 = "222222";
	const QString  passWord3 = "333333";

public:
	InputDlg(bool& loginFlag, const int passIndex, QWidget* parent = 0) : mLoginFlag(loginFlag), mPassIndex(passIndex)
	{
		setWindowTitle("password:");
		tEdit = new QLineEdit;
		tEdit->setEchoMode(QLineEdit::EchoMode::Password);
		tBtn = new QPushButton;
		tBtn->setText("Login");
		tLayout = new QGridLayout(this);

		tLayout->addWidget(tEdit, 0, 0);
		tLayout->addWidget(tBtn, 0, 1);

		mPassword.push_back(passWord1);
		mPassword.push_back(passWord2);
		mPassword.push_back(passWord3);

		connect(tBtn, &QPushButton::clicked, [this]()
		{
			if (tEdit->text() == mPassword[mPassIndex])
			{
				mLoginFlag = true;
				this->close();
			}
			else
			{
				QMessageBox::warning(this, "", "password Error.");
				mLoginFlag = false;
			}
		});
	}

	private slots:

private:
	QLineEdit *tEdit;
	QPushButton *tBtn;
	QGridLayout *tLayout;
	const int mPassIndex;
	QStringList mPassword;
	bool& mLoginFlag;
};

