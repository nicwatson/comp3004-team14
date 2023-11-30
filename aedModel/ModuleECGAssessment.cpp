#include "ModuleECGAssessment.h"

using namespace aedModel;

ModuleECGAssessment::ModuleECGAssessment(aedGui::LCDDisplay* l) : active(false), rhythm(VENT_FIB), lcdDisplay(l)
{
    // Initialize the timer that simulates the 5 seconds of analysis time
    timer = new QTimer(this);

    // Get the path of the directory containing the ECG data (data is in csv files)
    QDir parentDir = QDir::current();
    parentDir.cdUp();
    QString csvDirPath = parentDir.path() + "/comp3004-team14/assets/";

    // Iterate over all the csv files and load them into class attributes (to save time, no need to read from file each time the startaAsessment slot is called)
    QStringList csvList = {"/v_fib.csv", "/v_tachy.csv", "/non_shockable.csv"};
    for (int i = 0; i < csvList.length(); i++)
    {
        readCSVFile(csvDirPath, csvList[i]);
    }
}

ModuleECGAssessment::~ModuleECGAssessment()
{
    if (timer != nullptr) { delete timer; }
}


// When a signal is received from AED to this slot, the assessment will begin.
void ModuleECGAssessment::startAssessment()
{
    // If an assessment is in progress, can't start a new one
    if (active == true) return;

    // Set to true as assessment is in progress
    active = true;
    lcdDisplay->setPrompt("Don't touch patient. Analysing");

    // Start the 5-second timer to simulate analysis. Once timer runs out, the appropriate function is called, depending on which rhythm is shockable.
    timer->setInterval(5000);
    timer->setSingleShot(true);
    if (rhythm == VENT_FIB)
    {
        // This causes a vent fib graph to be plotted to the gui
        lcdDisplay->setGraphData(&ventFibXData, &ventFibYData);
        lcdDisplay->plotGraphData();

        // When the timer runs out, send a shockable signal (this'll be received by the AED)
        connect(timer, &QTimer::timeout, this, &ModuleECGAssessment::sendShockableSignal);
    } else if (rhythm == VENT_TACHY)
    {
        // This causes a vent tachy graph to be plotted to the gui
        lcdDisplay->setGraphData(&ventTachyXData, &ventTachyYData);
        lcdDisplay->plotGraphData();

        // When the timer runs out, send a shockable signal (this'll be received by the AED)
         connect(timer, &QTimer::timeout, this, &ModuleECGAssessment::sendShockableSignal);
    } else
    {
        // This causes a nonshockable graph to be plotted to the gui
        lcdDisplay->setGraphData(&nonShockableXData, &nonShockableYData);
        lcdDisplay->plotGraphData();

        // When the timer runs out, send anon shockable signal (this'll be received by the AED)
        connect(timer, &QTimer::timeout, this, &ModuleECGAssessment::sendNonShockableSignal);
    }
    timer->start();
}

// When a signal is received to this slot, the assessment should end immediately (this'll happen in case of error)
void ModuleECGAssessment::endAssessment()
{
    // Stops the analysis (the 5-second timer) prematurely. If-else statement checks wwhich timer to stop.
    if (rhythm == VENT_FIB || rhythm == VENT_TACHY)
    {
        disconnect(timer, &QTimer::timeout, this, &ModuleECGAssessment::sendShockableSignal);
    } else
    {
        disconnect(timer, &QTimer::timeout, this, &ModuleECGAssessment::sendNonShockableSignal);
    }

    // Cause the current graph to be removed from the gui
    lcdDisplay->clearGraphData();

    // I wasnt sure what to do if analysis interrupted
    lcdDisplay->setPrompt("");

    // An asssessment is no longer in process
    active = false;
}

// Iterate over all the csv files and load them into class attributes (to save time, no need to read from file each time the startaAsessment slot is called)
void ModuleECGAssessment::readCSVFile(QString fileDirectory, QString fileName)
{
    QFile CSVFile(fileDirectory + fileName);
    if (CSVFile.open(QIODevice::ReadWrite))
    {
        QTextStream Stream(&CSVFile);
        while(Stream.atEnd() == false)
        {
            // Read the current line (i.e. row) in the CSV
            QString lineData = Stream.readLine();
            // Split the data in the row
            QStringList data = lineData.split(",");
            for (int i = 0; i < data.length(); i++)
            {
                if (fileName == "/v_fib.csv")
                {
                    ventFibXData.push_back(data[0].toDouble()); // The datum in the 1st column of the current row is the x-coordinate
                    ventFibYData.push_back(data[1].toDouble()); // The datum in the 2nd column of the current row is the y-coordinate
                } else if (fileName == "/v_tachy.csv")
                {
                    ventTachyXData.push_back(data[0].toDouble()); // The datum in the 1st column of the current row is the x-coordinate
                    ventTachyYData.push_back(data[1].toDouble()); // The datum in the 2nd column of the current row is the y-coordinate
                } else if (fileName == "/non_shockable.csv")
                {
                    nonShockableXData.push_back(data[0].toDouble()); // The datum in the 1st column of the current row is the x-coordinate
                    nonShockableYData.push_back(data[1].toDouble()); // The datum in the 2nd column of the current row is the y-coordinate
                }
            }
        }
    }
    CSVFile.close();

}

// When the timer has run out, this function is called. The assessment is ended and a shockable signnal emitted
void ModuleECGAssessment::sendShockableSignal()
{
    endAssessment();
    lcdDisplay->setPrompt("Shock Advised");
    emit signalShockable();

}

// When the timer has run out, this function is called. The assessment is ended and a non-shockable signnal emitted
void ModuleECGAssessment::sendNonShockableSignal()
{
    endAssessment();
    lcdDisplay->setPrompt("No Shock Advised");
    emit signalNotShockable();
}
