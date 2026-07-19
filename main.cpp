#include "mainwindow.h"
//Qt应用程序头文件
#include <QApplication>
//argc：命令行参数个数，argv：命令行参数字符串数组
int main(int argc, char *argv[])
{//QApplication是应用程序类，application是对象，应用程序对象有且只有一个
    //必须把main函数的参数传给application对象
    QApplication application(argc, argv);

    MainWindow window;//qt提供的MainWindow类
    window.show();//显示窗口
//application.exec()应用程序消息循环，相当于while
    return application.exec();//X从这返回
}
/*快捷键
运行代码Ctrl+r
编译Ctrl+b
注释Ctrl+/
缩放字体Ctrl+滚轮
查找/替换Ctrl+f（整体）
整行移动代码Ctrl+Shift+↑↓
自动对齐Ctrl+i
在同名头文件和源文件之间切换：F4
快速添加函数定义：鼠标移动到声明那一行，按Alt+Enter
修改变量名，并应用到所有Ctrl+Shift+r
书签功能：快速跳转代码：Ctrl+M（添加/删除标签）
        查找并移动到下一个标签：Ctrl + .
查看帮助文档：
第一种：Qt Creator 查看 F1
第二种：独立的帮助文档程序查看
*/