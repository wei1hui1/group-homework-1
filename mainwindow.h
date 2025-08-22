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
    void onBackClicked();       // 新增: 回溯或退出历史记录
    void onLeftClicked();       // 新增: 光标左移或历史记录上移
    void onRightClicked();      // 新增: 光标右移或历史记录下移
    void onOKClicked();         // 新增: 确认
    void onDelClicked();        // 新增: 删除单条历史记录
    void onAllClicked();        // 新增: 与del组合删除所有历史记录
    void onFormClicked();       // 新增: 切换分数/小数显示格式
    void onSquareClicked();     // 新增: 平方运算
    void onSqrtClicked();       // 新增: 开方运算
    void exitHistoryMode(); // 从历史记录模式退出
    
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
    bool historyMode; // 标记是否在历史记录模式
    int currentHistoryIndex; // 当前选中的历史记录索引
    int formatMode; // 显示格式模式 (0: 自动, 1: 分数, 2: 两位小数, 3: 精确小数)
    QString formatAsFraction(double value);
    long long gcd(long long a, long long b);
    void updatePreview(QString calcExpr = ""); // 合并为一个函数
    // 辅助方法
    int findMatchingParenthesis(const QString& expr, int pos);
    double calculate(const QString& expression);
    double evaluateExpression(QString expr);
    void saveHistory();
    void loadHistory();
    void addToHistory(const QString& entry);
    QString formatNumber(double value); // 格式化数字显示
    void displayHistory(); // 显示历史记录
    // 添加新的函数声明
    QString formatForDisplay(const QString& expression);
    QString formatForCalculation(const QString& expression);
}; // 确保这个大括号后面没有多余的exitHistoryMode声明
#endif // MAINWINDOW_H
