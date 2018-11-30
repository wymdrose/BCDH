

#pragma once

#include <QStringList>
#include <QFile>
//#include <boost/shared_ptr.hpp>
#include <QTextStream>

namespace _FILECLASS
{
	class CSV_FILE
	{
	public:
		CSV_FILE(QString filePath)
		{
			mpCsvFile = std::shared_ptr<QFile>(new QFile(filePath));
			mpCsvFile->open(QIODevice::ReadWrite | QIODevice::Append);
		}

		~CSV_FILE(void)
		{
			mpCsvFile->close();
		}
		

	public:
		bool append(const QStringList& tLine)
		{
			if (!mpCsvFile->isOpen())	{return false;}

		  //if (!mpCsvFile->resize(0))
							
			Q_FOREACH(QString str, tLine)
			{
			  //mpCsvFile->write(str.toAscii());
				mpCsvFile->write(str.toLatin1());
				mpCsvFile->write(QString(",").toLatin1());
			}

			mpCsvFile->write(QString("\n").toLatin1());
			
			mpCsvFile->flush();
			
			return true;
		}



		void get(std::vector<QStringList>& vLineAll)
		{
			mpCsvFile->seek(0);

			QTextStream tStream(mpCsvFile.get());

			QStringList CSVList;

			while (!tStream.atEnd())
			{
				QStringList oneLine = tStream.readLine().split(",");
				vLineAll.push_back(oneLine);
			}
		}

		
		QStringList get()
		{
			QStringList tList;
			tList.clear();

			mpCsvFile->seek(0);

			QTextStream tStream(mpCsvFile.get());

			while (!tStream.atEnd())
			{
				tList.push_back(tStream.readLine());
			}

			return tList;
		}
		
		QString getLast()
		{
			QStringList tList;
			tList.clear();

			mpCsvFile->seek(0);

			QTextStream tStream(mpCsvFile.get());

			while (!tStream.atEnd())
			{
				tList.push_back(tStream.readLine());
			}

			return tList[tList.size() - 1];
		}

	private:
		std::shared_ptr<QFile>    mpCsvFile;
	};







};



