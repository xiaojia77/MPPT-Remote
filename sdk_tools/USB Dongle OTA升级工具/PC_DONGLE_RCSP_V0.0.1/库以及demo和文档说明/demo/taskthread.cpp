#include "taskthread.h"

TaskThread::TaskThread(const std::function<void()> &body, QObject *parent)
    : QThread(parent), m_body(body)
{}

TaskThread::~TaskThread() {}

void TaskThread::run()
{
    m_body();
}
