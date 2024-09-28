#include <QCoreApplication>
#include <QThreadPool>
#include <QRunnable>
#include <QDebug>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    QThreadPool* threadPool = QThreadPool::globalInstance();

    // 线程池最大线程数
    qDebug() << threadPool->maxThreadCount();

    for (int i = 0; i < 20; ++i) {
        threadPool->start([i]{
            qDebug() << QString("thread id %1").arg(i);
        });
    }
    // 当前活跃线程数 10
    qDebug() << threadPool->activeThreadCount();

    app.exec();
}