#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QPushButton>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 数字按钮
    void onDigitClicked();
    void onDotClicked();
    
    // 运算符按钮
    void onOperatorClicked();
    void onLeftParenthesisClicked();
    void onRightParenthesisClicked();
    
    // 功能按钮
    void onEqualsClicked();
    void onClearClicked();
    void onBackspaceClicked();
    void onHistoryClicked();
    
    // 内存功能
    void onMemoryClearClicked();
    void onMemoryAddClicked();
    void onMemorySubtractClicked();
    void onMemoryRecallClicked();
    
    // 其他功能
    void onPercentClicked();
    void onToggleSignClicked();

private:
    Ui::MainWindow *ui;
    double memoryValue; // 存储内存值
    QList<QString> historyList; // 历史记录
    bool calculationComplete; // 标记计算是否完成
    
    // 辅助方法
    int findMatchingParenthesis(const QString& expr, int pos);
    double calculate(const QString& expression);
    double evaluateExpression(QString expr);
    void saveHistory();
    void loadHistory();
    void addToHistory(const QString& entry);
    void updatePreview(); // 实时更新预览
    QString formatNumber(double value); // 格式化数字显示
};

#endif // MAINWINDOW_H
