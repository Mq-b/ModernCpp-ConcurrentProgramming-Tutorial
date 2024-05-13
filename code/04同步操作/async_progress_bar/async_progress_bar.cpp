#include "async_progress_bar.h"

async_progress_bar::async_progress_bar(QWidget *parent)
    : QMainWindow{ parent }, progress_bar{ new QProgressBar(this) },
    button{ new QPushButton("start",this) },button2{ new QPushButton("测试",this) } {
    ui.setupUi(this);

    progress_bar->setStyleSheet(progress_bar_style);
    progress_bar->setRange(0, 1000);

    button->setMinimumSize(100, 50);
    button->setMaximumWidth(100);
    button->setStyleSheet(button_style);
    button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    button2->setMinimumSize(100, 50);
    button2->setMaximumWidth(100);
    button2->setStyleSheet(button_style);
    button2->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(progress_bar);
    layout->addWidget(button, 0, Qt::AlignHCenter);
    layout->addWidget(button2, 0, Qt::AlignHCenter);
    // 设置窗口布局为垂直布局管理器
    centralWidget()->setLayout(layout);

    connect(button, &QPushButton::clicked, this, &async_progress_bar::task);
    connect(button2, &QPushButton::clicked, []{
        QMessageBox::information(nullptr, "测试", "没有卡界面！");
    });
}

async_progress_bar::~async_progress_bar()
{}
