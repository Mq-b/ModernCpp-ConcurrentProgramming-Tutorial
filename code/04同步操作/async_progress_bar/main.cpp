#include "async_progress_bar.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[]){
    QApplication a(argc, argv);

    auto s = std::to_string(_Thrd_id());
    QMessageBox::information(nullptr, "主线程ID", s.c_str());

    async_progress_bar w;
    w.show();
    return a.exec();
}
