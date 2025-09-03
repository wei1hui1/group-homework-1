#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QPushButton>

namespace Ui {
class MainWindow;
}

/*!
 * @brief 主窗口类：实现一个带历史记录、内存功能、分数/小数格式切换的计算器。
 *
 * 对外仅暴露 QWidget 的基本生命周期，所有业务逻辑均通过私有槽函数完成。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /*!
     * @brief 构造主窗口
     * @param parent 父窗口指针，默认为 nullptr
     */
    explicit MainWindow(QWidget *parent = nullptr);

    //! 析构函数
    ~MainWindow();

private slots:
    //-------------------------------------------------------------
    // 数字与基本输入
    //-------------------------------------------------------------
    void onDigitClicked();      //!< 数字 0-9 按钮统一入口
    void onDotClicked();        //!< 小数点按钮

    //-------------------------------------------------------------
    // 运算符
    //-------------------------------------------------------------
    void onOperatorClicked();   //!< + - * / 四则运算符
    void onLeftParenthesisClicked();
    void onRightParenthesisClicked();

    //-------------------------------------------------------------
    // 功能按钮
    //-------------------------------------------------------------
    void onEqualsClicked();     //!< 计算并保存结果到历史记录
    void onClearClicked();      //!< 清屏
    void onBackspaceClicked();  //!< 退格
    void onHistoryClicked();    //!< 进入/退出历史记录浏览模式

    // 以下按键仅在历史记录模式下生效
    void onBackClicked();       //!< 退出历史模式
    void onLeftClicked();       //!< 光标左移 / 历史记录上一条
    void onRightClicked();      //!< 光标右移 / 历史记录下一条
    void onOKClicked();         //!< 确认选中历史条目
    void onDelClicked();        //!< 删除当前选中的历史条目
    void onAllClicked();        //!< 与 Del 组合：清空全部历史
    void onFormClicked();       //!< 切换分数/小数/定点/高精度显示
    void onSquareClicked();     //!< 平方
    void onSqrtClicked();       //!< 开平方
    void exitHistoryMode();     //!< 从历史模式返回普通模式

    //-------------------------------------------------------------
    // 内存功能：MC, M+, M-, MR
    //-------------------------------------------------------------
    void onMemoryClearClicked();
    void onMemoryAddClicked();
    void onMemorySubtractClicked();
    void onMemoryRecallClicked();

    //-------------------------------------------------------------
    // 其他功能
    //-------------------------------------------------------------
    void onPercentClicked();    //!< 百分号
    void onToggleSignClicked(); //!< 正负号切换

private:
    Ui::MainWindow *ui;

    double memoryValue;             //!< 当前存储的内存值
    QList<QString> historyList;     //!< 历史记录缓存
    bool calculationComplete;       //!< 最近一次按键是否为“=”
    bool historyMode;               //!< 当前是否处于历史浏览模式
    int  currentHistoryIndex;       //!< 历史记录中光标位置
    int  formatMode;                //!< 0:自动 1:分数 2:两位小数 3:精确小数

    /*!
     * @brief 将 double 格式化为分数字符串
     * @return 形如 "3 1/2" 的分数表示
     */
    QString formatAsFraction(double value);

    /*!
     * @brief 求最大公约数，用于分数化简
     */
    long long gcd(long long a, long long b);

    /*!
     * @brief 更新表达式预览区域
     * @param calcExpr 若为空则使用当前输入框内容
     */
    void updatePreview(QString calcExpr = "");

    /*!
     * @brief 查找与指定位置匹配的括号
     * @return 匹配括号下标，若找不到返回 -1
     */
    int findMatchingParenthesis(const QString& expr, int pos);

    /*!
     * @brief 计算给定表达式的值
     * @param expression 合法的中缀表达式
     * @return 计算结果；若表达式非法返回 qQNaN()
     */
    double calculate(const QString& expression);

    /*!
     * @brief 内部递归求值实现
     */
    double evaluateExpression(QString expr);

    //! 持久化历史记录到磁盘
    void saveHistory();
    //! 从磁盘加载历史记录
    void loadHistory();

    /*!
     * @brief 添加一条记录到历史缓存并刷新显示
     * @param entry 完整表达式 + "=" + 结果
     */
    void addToHistory(const QString& entry);

    /*!
     * @brief 根据当前 formatMode 将数值格式化为显示字符串
     */
    QString formatNumber(double value);

    //! 将 historyList 渲染到历史记录界面
    void displayHistory();

    /*!
     * @brief 将输入框字符串转换为可计算的表达式（去掉千位分隔符等）
     */
    QString formatForCalculation(const QString& expression);

    /*!
     * @brief 将表达式格式化为适合显示的字符串（插入千位分隔符等）
     */
    QString formatForDisplay(const QString& expression);
};

#endif // MAINWINDOW_H

