#pragma once

#include <QtWidgets/QMainWindow>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <thread>
#include <future>
#include <chrono>
#include "ui_async_progress_bar.h"

using namespace std::chrono_literals;

class async_progress_bar : public QMainWindow{
    Q_OBJECT

public:
    async_progress_bar(QWidget *parent = nullptr);
    ~async_progress_bar();

    void task(){
        future = std::async(std::launch::async, [=] {
            QMetaObject::invokeMethod(this, [this] {
                // 这里显示的线程 ID 就是主线程，代表这些任务就是在主线程，即 UI 线程执行
                QMessageBox::information(nullptr, "线程ID", std::to_string(_Thrd_id()).c_str());
                button->setEnabled(false);
                progress_bar->setRange(0, 1000);
                button->setText("正在执行...");
            });
            for (int i = 0; i <= 1000; ++i) {
                std::this_thread::sleep_for(10ms);
                QMetaObject::invokeMethod(this, [this, i] {
                    progress_bar->setValue(i);
                });
            }
            QMetaObject::invokeMethod(this, [this] {
                button->setText("start");
                button->setEnabled(true);
            });
            // 不在 invokeMethod 中获取线程 ID，这里显示的是子线程的ID
            auto s = std::to_string(_Thrd_id());
            QMetaObject::invokeMethod(this, [=] {
                QMessageBox::information(nullptr, "线程ID", s.c_str());
            });
        });
    }
private:
    QString progress_bar_style =
        "QProgressBar {"
        "    border: 2px solid grey;"
        "    border-radius: 5px;"
        "    background-color: lightgrey;"
        "    text-align: center;"  // 文本居中
        "    color: #000000;"      // 文本颜色
        "}"
        "QProgressBar::chunk {"
        "    background-color: #7FFF00;"
        "    width: 10px;"         // 设置每个进度块的宽度
        "    font: bold 14px;"     // 设置进度条文本字体
        "}";
    QString button_style =
        "QPushButton {"
        "    text-align: center;"  // 文本居中
        "}";
    QProgressBar* progress_bar{};
    QPushButton* button{};
    QPushButton* button2{};
    Ui::async_progress_barClass ui{};
    std::future<void> future;
};
