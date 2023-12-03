#include "ModuleSelfTest.h"
#include "AED.h"

using namespace aedModel;

bool ModuleSelfTest::FORCE_FAIL = false;

ModuleSelfTest::ModuleSelfTest()
    : QObject(), active(false), result(FAIL_OTHER)
{
    timer.setSingleShot(true);
    timer.setInterval(TEST_TIME);
    connect(&timer, &QTimer::timeout, this, &aedModel::ModuleSelfTest::finishSelfTest);
}

void ModuleSelfTest::reset()
{
    qDebug() << "Self test module is being reset";
    active = false;
    result = FAIL_OTHER;
}

// SLOT
void ModuleSelfTest::startSelfTest(AED * unit)
{
    qDebug() << "Start self test";
    active = true;
    if(FORCE_FAIL)
    {
        result = FAIL_OTHER;
    }
    else if(unit->getBattery() < BATTERY_THRESHHOLD)
    {
        result = FAIL_BATTERY;
    }
    if(unit->getCableState() == AED::UNPLUGGED)
    {
        result = FAIL_CABLE;
    }
    else
    {
        qDebug() << "Result OK, starting timer";
        result = OK;
    }
    timer.start();  // Simulate short delay before reporting test result
}

// SLOT
void ModuleSelfTest::abortSelfTest()
{
    timer.blockSignals(true);
    timer.stop();
    timer.blockSignals(false);
    reset();
}

// SLOT
void ModuleSelfTest::finishSelfTest()
{
    qDebug() << "Self test finished";
    emit signalResult(result);
    reset();
}


