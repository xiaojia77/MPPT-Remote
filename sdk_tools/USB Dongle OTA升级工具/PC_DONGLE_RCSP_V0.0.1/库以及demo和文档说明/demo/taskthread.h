#ifndef TASKTHREAD_H
#define TASKTHREAD_H

#include <functional>
#include <QThread>

class TaskThread : public QThread
{
    Q_OBJECT
public:
    TaskThread(const std::function<void(void)> &body, QObject *parent = nullptr);

    ~TaskThread();

    void run() override;

private:
    std::function<void(void)> m_body;
};

#endif // TASKTHREAD_H
