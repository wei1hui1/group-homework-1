#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>
#include <QList>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QDebug>
#include <QStack>
#include <cmath>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    calculationComplete(false),
    memoryValue(0.0)
{
    ui->setupUi(this);
    loadHistory();

    // 连接数字按钮
    connect(ui->_7, SIGNAL(clicked()), this, SLOT(onDigitClicked()));     // 7
    connect(ui->_8, SIGNAL(clicked()), this, SLOT(onDigitClicked()));     // 8
    connect(ui->_9, SIGNAL(clicked()), this, SLOT(onDigitClicked()));     // 9
    connect(ui->_4, SIGNAL(clicked()), this, SLOT(onDigitClicked()));     // 4
    connect(ui->_5, SIGNAL(clicked()), this, SLOT(onDigitClicked()));     // 5
    connect(ui->_6, SIGNAL(clicked()), this, SLOT(onDigitClicked()));     // 6
    connect(ui->_1, SIGNAL(clicked()), this, SLOT(onDigitClicked()));     // 1
    connect(ui->_2, SIGNAL(clicked()), this, SLOT(onDigitClicked()));     // 2
    connect(ui->_3, SIGNAL(clicked()), this, SLOT(onDigitClicked()));     // 3
    connect(ui->_0, SIGNAL(clicked()), this, SLOT(onDigitClicked()));     // 0
    connect(ui->point, SIGNAL(clicked()), this, SLOT(onDotClicked()));     // .

    // 连接运算符按钮
    connect(ui->multiply, SIGNAL(clicked()), this, SLOT(onOperatorClicked()));  // ×
    connect(ui->division, SIGNAL(clicked()), this, SLOT(onOperatorClicked()));  // ÷
    connect(ui->add, SIGNAL(clicked()), this, SLOT(onOperatorClicked()));       // +
    connect(ui->minus, SIGNAL(clicked()), this, SLOT(onOperatorClicked()));     // -
    connect(ui->FB, SIGNAL(clicked()), this, SLOT(onLeftParenthesisClicked()));  // (
    connect(ui->BB, SIGNAL(clicked()), this, SLOT(onRightParenthesisClicked())); // )

    // 连接功能按钮
    connect(ui->equal, SIGNAL(clicked()), this, SLOT(onEqualsClicked()));       // =
    connect(ui->C, SIGNAL(clicked()), this, SLOT(onBackspaceClicked()));      // C应该只清除当前输入
    connect(ui->del, SIGNAL(clicked()), this, SLOT(onBackspaceClicked()));      // ⌫
    connect(ui->record, SIGNAL(clicked()), this, SLOT(onHistoryClicked()));     // 🕒

    // 连接内存功能按钮
    connect(ui->mc, SIGNAL(clicked()), this, SLOT(onMemoryClearClicked()));     // mc
    connect(ui->m_add, SIGNAL(clicked()), this, SLOT(onMemoryAddClicked()));    // m+
    connect(ui->m_minus, SIGNAL(clicked()), this, SLOT(onMemorySubtractClicked()));// m-
    connect(ui->mr, SIGNAL(clicked()), this, SLOT(onMemoryRecallClicked()));    // mr

    // 连接其他功能按钮
    connect(ui->percent, SIGNAL(clicked()), this, SLOT(onPercentClicked()));    // %
    connect(ui->AC, SIGNAL(clicked()), this, SLOT(onClearClicked()));           // AC应该清空所有内容
    // 移除错误的连接：connect(ui->AC, SIGNAL(clicked()), this, SLOT(onToggleSignClicked()));      // +/- 

    // 初始化显示
    ui->textEdit->setAlignment(Qt::AlignRight);
    ui->lineEdit->setAlignment(Qt::AlignRight);  // 修改为正确的结果栏名称
    ui->textEdit->setReadOnly(true);
    ui->lineEdit->setReadOnly(true);  // 修改为正确的结果栏名称
    ui->textEdit->setText("");
    ui->lineEdit->setText("");
    
    // 设置初始字体大小
    QFont textFont = ui->textEdit->font();
    textFont.setPointSize(20);  // 设置过程栏初始字体大小
    ui->textEdit->setFont(textFont);
    
    QFont lineFont = ui->lineEdit->font();
    lineFont.setPointSize(24);  // 设置结果栏初始字体大小
    ui->lineEdit->setFont(lineFont);
    
    // 加载历史记录
    loadHistory();
}

MainWindow::~MainWindow()
{
    saveHistory();
    delete ui;
}

void MainWindow::onDigitClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    QString digit = button->text();
    QString currentText = ui->textEdit->toPlainText();  // 使用toPlainText()而不是text()

    // 如果计算已完成，开始新的计算
    if (calculationComplete) {
        ui->textEdit->setText(digit);
        ui->lineEdit->setText("");
        calculationComplete = false;
        QFont font = ui->textEdit->font();
        font.setPointSize(12);
        font.setWeight(QFont::Normal);
        ui->textEdit->setFont(font);
        ui->textEdit->setStyleSheet("color: black;");
    } else {
        if (currentText == "0" && digit != ".") {
            ui->textEdit->setText(digit);
        } else {
            ui->textEdit->setText(currentText + digit);
        }
    }

    updatePreview();
}

void MainWindow::onDotClicked()
{
    QString currentText = ui->textEdit->toPlainText();  // 使用toPlainText()而不是text()

    // 如果计算已完成，开始新的计算
    if (calculationComplete) {
        ui->textEdit->setText("0.");
        ui->lineEdit->setText("");
        calculationComplete = false;
        // 恢复过程栏样式
        QFont font = ui->textEdit->font();
        font.setPointSize(12);
        font.setWeight(QFont::Normal);
        ui->textEdit->setFont(font);
        ui->textEdit->setStyleSheet("color: black;");
    } else {
        // 检查当前输入中是否已有小数点
        QRegularExpression regex("[0-9]+\\.[0-9]*$");
        if (!regex.match(currentText).hasMatch()) {
            // 保留修改后的版本，移除原代码
            if (currentText.isEmpty() || (!currentText.back().isDigit() && currentText.back() != ')')) {
                ui->textEdit->setText(currentText + "0.");
            }
        }
    }

    updatePreview();
}

void MainWindow::onOperatorClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    // 修复：通过按钮对象指针而不是文本来确定运算符
    QString operatorText;
    if (button == ui->multiply) {
        operatorText = "*";
    } else if (button == ui->division) {
        operatorText = "/";
    } else if (button == ui->add) {
        operatorText = "+";
    } else if (button == ui->minus) {
        operatorText = "-";
    } else {
        // 默认使用按钮文本（如果需要）
        operatorText = button->text();
    }

    QString currentText = ui->textEdit->toPlainText();

    // 如果计算已完成，将结果栏的内容移到过程栏
    if (calculationComplete) {
        ui->textEdit->setText(ui->lineEdit->text() + operatorText);
        ui->lineEdit->setText("");
        calculationComplete = false;
        // 恢复过程栏样式 - 修改为大字体
        QFont font = ui->textEdit->font();
        font.setPointSize(20);  // 从12点改为20点，与构造函数中一致
        font.setWeight(QFont::Normal);
        ui->textEdit->setFont(font);
        ui->textEdit->setStyleSheet("color: black;");
    } else {
        // 检查是否可以添加运算符
        if (!currentText.isEmpty()) {
            QChar lastChar = currentText.back();
            // 如果最后一个字符是运算符，替换它
            if (lastChar == '+' || lastChar == '-' || lastChar == '*' || lastChar == '/') {
                currentText.chop(1);
            }
            ui->textEdit->setText(currentText + operatorText);
        }
    }

    updatePreview();
}

void MainWindow::onLeftParenthesisClicked()
{
    QString currentText = ui->textEdit->toPlainText();

    // 如果计算已完成，将结果栏的内容移到过程栏并添加左括号
    if (calculationComplete) {
        ui->textEdit->setText(ui->lineEdit->text() + "(");
        ui->lineEdit->setText("");
        calculationComplete = false;
        // 恢复过程栏样式 - 使用大字体
        QFont font = ui->textEdit->font();
        font.setPointSize(20);  // 保持与构造函数一致的大字体
        font.setWeight(QFont::Normal);
        ui->textEdit->setFont(font);
        ui->textEdit->setStyleSheet("color: black;");
    } else {
        // 如果当前为空或最后一个字符是运算符或左括号，直接添加左括号
        if (currentText.isEmpty() || currentText.back() == '+' || currentText.back() == '-' || 
            currentText.back() == '*' || currentText.back() == '/' || currentText.back() == '(') {
            ui->textEdit->setText(currentText + "(");
        } else {
            // 否则添加乘号和左括号
            ui->textEdit->setText(currentText + "*(");
        }
    }

    updatePreview();
}

void MainWindow::onRightParenthesisClicked()
{
    QString currentText = ui->textEdit->toPlainText();

    // 计算左右括号数量
    int leftCount = currentText.count('(');
    int rightCount = currentText.count(')');

    if (leftCount > rightCount) {
        // 如果最后一个字符是数字或右括号，直接添加右括号
        if (!currentText.isEmpty() && (currentText.back().isDigit() || currentText.back() == ')')) {
            ui->textEdit->setText(currentText + ")");
        }
    }

    updatePreview();
}

void MainWindow::onEqualsClicked()
{
    QString expression = ui->textEdit->toPlainText();

    if (expression.isEmpty()) return;

    try {
        double result = calculate(expression);
        QString resultStr = formatNumber(result);

        // 添加到历史记录
        addToHistory(expression + " = " + resultStr);

        // 更新显示
        ui->lineEdit->setText(resultStr);
        calculationComplete = true;

        // 改变过程栏样式
        QFont font = ui->textEdit->font();
        font.setPointSize(20);  // 增大过程栏字体，从10点改为12点
        font.setWeight(QFont::Light);
        ui->textEdit->setFont(font);
        ui->textEdit->setStyleSheet("color: gray;");

        // 改变结果栏样式（显示器）
        QFont resultFont = ui->lineEdit->font();
        resultFont.setPointSize(24);  // 增大结果栏字体，从16点改为24点
        resultFont.setWeight(QFont::Bold);
        ui->lineEdit->setFont(resultFont);
        ui->lineEdit->setStyleSheet("color: black;");
    } catch (const QString& error) {
        QMessageBox::warning(this, "计算错误", error);
    }
}

void MainWindow::onClearClicked()
{
    ui->textEdit->clear();
    ui->lineEdit->clear();
    calculationComplete = false;

    // 恢复过程栏样式 - 修改为大字体
    QFont font = ui->textEdit->font();
    font.setPointSize(20);  // 从12点改为20点
    font.setWeight(QFont::Normal);
    ui->textEdit->setFont(font);
    ui->textEdit->setStyleSheet("color: black;");

    // 恢复结果栏样式 - 修改为大字体
    QFont resultFont = ui->lineEdit->font();
    resultFont.setPointSize(24);  // 从12点改为24点
    resultFont.setWeight(QFont::Normal);
    ui->lineEdit->setFont(resultFont);
    ui->lineEdit->setStyleSheet("color: black;");
}

void MainWindow::onBackspaceClicked()
{
    QString currentText = ui->textEdit->toPlainText();
    if (!currentText.isEmpty()) {
        currentText.chop(1);
        ui->textEdit->setText(currentText);
        updatePreview();
    }
}

void MainWindow::onHistoryClicked()
{
    if (historyList.isEmpty()) {
        QMessageBox::information(this, "历史记录", "没有历史记录");
        return;
    }

    QString historyText;
    for (const QString& entry : historyList) {
        historyText += entry + "\n";
    }

    QMessageBox::information(this, "历史记录", historyText);
}

void MainWindow::onMemoryClearClicked()
{
    memoryValue = 0.0;
}

void MainWindow::onMemoryAddClicked()
{
    try {
        if (calculationComplete) {
            memoryValue += ui->lineEdit->text().toDouble();
        } else if (!ui->lineEdit->text().isEmpty()) {
            memoryValue += ui->lineEdit->text().toDouble();
        } else if (!ui->textEdit->toPlainText().isEmpty()) {
            memoryValue += calculate(ui->textEdit->toPlainText());
        }
    } catch (const QString&) {
        // 忽略错误
    }
}

void MainWindow::onMemorySubtractClicked()
{
    try {
        if (calculationComplete) {
            memoryValue -= ui->lineEdit->text().toDouble();
        } else if (!ui->lineEdit->text().isEmpty()) {
            memoryValue -= ui->lineEdit->text().toDouble();
        } else if (!ui->textEdit->toPlainText().isEmpty()) {
            memoryValue -= calculate(ui->textEdit->toPlainText());
        }
    } catch (const QString&) {
        // 忽略错误
    }
}

void MainWindow::onMemoryRecallClicked()
{
    ui->textEdit->setText(formatNumber(memoryValue));
    calculationComplete = false;
    updatePreview();

    // 恢复过程栏样式
    QFont font = ui->textEdit->font();
    font.setPointSize(12);
    font.setWeight(QFont::Normal);
    ui->textEdit->setFont(font);
    ui->textEdit->setStyleSheet("color: black;");
}

void MainWindow::onPercentClicked()
{
    try {
        // 如果计算已完成，将结果栏的内容转换为百分比
        if (calculationComplete) {
            double value = ui->lineEdit->text().toDouble();
            value /= 100.0;
            ui->textEdit->setText(formatNumber(value));
            ui->lineEdit->setText("");
            calculationComplete = false;
            // 恢复过程栏样式 - 使用大字体
            QFont font = ui->textEdit->font();
            font.setPointSize(20);  // 保持与构造函数一致的大字体
            font.setWeight(QFont::Normal);
            ui->textEdit->setFont(font);
            ui->textEdit->setStyleSheet("color: black;");
        } else {
            // 原来的行为：处理过程栏的内容
            QString currentText = ui->textEdit->toPlainText();
            if (!currentText.isEmpty()) {
                double value = currentText.toDouble();
                value /= 100.0;
                ui->textEdit->setText(formatNumber(value));
            }
        }
        updatePreview();
    } catch (const QString&) {
        // 忽略错误
    }
}

void MainWindow::onToggleSignClicked()
{
    QString currentText = ui->textEdit->toPlainText();
    if (!currentText.isEmpty()) {
        if (currentText.startsWith('-')) {
            currentText.remove(0, 1);
        } else {
            currentText.prepend('-');
        }
        ui->textEdit->setText(currentText);
        updatePreview();
    }
}

int MainWindow::findMatchingParenthesis(const QString& expr, int pos)
{
    if (expr[pos] != '(') return -1;

    int count = 1;
    for (int i = pos + 1; i < expr.length(); i++) {
        if (expr[i] == '(') count++;
        else if (expr[i] == ')') count--;

        if (count == 0) return i;
    }

    return -1; // 没有找到匹配的右括号
}

double MainWindow::calculate(const QString& expression)
{
    // 首先检查表达式是否平衡
    int leftCount = expression.count('(');
    int rightCount = expression.count(')');
    if (leftCount != rightCount) {
        throw QString("括号不匹配");
    }

    // 处理括号
    QString expr = expression;
    int pos = expr.indexOf('(');
    while (pos != -1) {
        int endPos = findMatchingParenthesis(expr, pos);
        if (endPos == -1) {
            throw QString("括号不匹配");
        }

        // 计算括号内的表达式
        QString subExpr = expr.mid(pos + 1, endPos - pos - 1);
        double subResult = evaluateExpression(subExpr);

        // 替换括号及其内容为计算结果
        expr.replace(pos, endPos - pos + 1, formatNumber(subResult));

        pos = expr.indexOf('(');
    }

    // 计算没有括号的表达式
    return evaluateExpression(expr);
}

double MainWindow::evaluateExpression(QString expr)
{
    // 处理负数
    if (expr.startsWith('-')) {
        expr.prepend('0');
    }
    expr.replace("(-", "(0-");

    // 分割表达式为数字和运算符 - 修复：使用ASCII字符
    QRegularExpression re("(\\d+\\.\\d+|\\d+|[+\\-*/])");
    QRegularExpressionMatchIterator it = re.globalMatch(expr);

    QList<QString> tokens;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        tokens.append(match.captured(1));
    }

    // 检查表达式是否有效
    if (tokens.isEmpty()) {
        throw QString("表达式为空");
    }

    // 修复：使用ASCII字符比较
    if (tokens.first() == "+" || tokens.first() == "*" || tokens.first() == "/") {
        throw QString("表达式无效");
    }

    // 修复：使用ASCII字符比较
    if (tokens.last() == "+" || tokens.last() == "-" || tokens.last() == "*" || tokens.last() == "/") {
        throw QString("表达式无效");
    }

    // 第一步：处理乘除
    QList<double> numbers;
    QList<QString> ops;

    numbers.append(tokens[0].toDouble());

    // 修复：添加索引边界检查
    for (int i = 1; i < tokens.size(); i += 2) {
        if (i >= tokens.size() || i+1 >= tokens.size()) {
            throw QString("表达式格式错误");
        }
        
        QString op = tokens[i];
        double num = tokens[i+1].toDouble();

        // 修复：使用ASCII字符比较
        if (op == "*") {
            numbers.last() *= num;
        } else if (op == "/") {
            if (num == 0) {
                throw QString("除数不能为零");
            }
            numbers.last() /= num;
        } else {
            numbers.append(num);
            ops.append(op);
        }
    }

    // 第二步：处理加减
    // 修复：添加索引边界检查
    if (numbers.isEmpty()) {
        throw QString("计算错误");
    }
    
    double result = numbers[0];
    for (int i = 0; i < ops.size(); i++) {
        if (i+1 >= numbers.size()) {
            throw QString("表达式格式错误");
        }
        
        // 修复：移除重复的条件判断
        if (ops[i] == "+") {
            result += numbers[i+1];
        } else if (ops[i] == "-") {
            result -= numbers[i+1];
        }
    }

    return result;
}

void MainWindow::saveHistory()
{
    QFile file("e:/Projects/calculator/Calculator/build-debug/history.json");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "无法打开历史记录文件进行写入";
        return;
    }

    QJsonObject jsonObj;
    QJsonArray jsonArray;

    for (const QString& entry : historyList) {
        jsonArray.append(entry);
    }

    jsonObj["history"] = jsonArray;
    QJsonDocument jsonDoc(jsonObj);

    file.write(jsonDoc.toJson());
    file.close();
}

void MainWindow::loadHistory()
{
    QFile file("e:/Projects/calculator/Calculator/build-debug/history.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开历史记录文件进行读取";
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
    QJsonObject jsonObj = jsonDoc.object();
    QJsonArray jsonArray = jsonObj["history"].toArray();

    historyList.clear();
    for (const QJsonValue& value : jsonArray) {
        historyList.append(value.toString());
    }

    file.close();
}

void MainWindow::addToHistory(const QString& entry)
{
    // 限制历史记录数量
    const int maxHistory = 100;
    if (historyList.size() >= maxHistory) {
        historyList.removeLast();  // 移除最旧的记录
    }
    historyList.prepend(entry);  // 添加到列表开头，而不是末尾
    saveHistory();
}

void MainWindow::updatePreview()
{
    QString expression = ui->textEdit->toPlainText();

    // 检查表达式是否可以直接计算
    if (expression.isEmpty()) {
        ui->lineEdit->setText("");
        return;
    }

    // 检查最后一个字符是否是运算符
    QChar lastChar = expression.back();
    // 修复：使用ASCII字符比较
    if (lastChar == '+' || lastChar == '-' || lastChar == '*' || lastChar == '/') {
        ui->lineEdit->setText("");
        return;
    }

    try {
        double result = calculate(expression);
        QString resultStr = formatNumber(result);

        // 设置预览样式
        ui->lineEdit->setText(resultStr);
        QFont font = ui->lineEdit->font();
        font.setPointSize(24);  // 增大预览字体，从12点改为24点
        font.setWeight(QFont::Bold);  // 同时加粗字体
        ui->lineEdit->setFont(font);
        ui->lineEdit->setStyleSheet("color: gray;");
    } catch (const QString&) {
        ui->lineEdit->setText("");
    }
}

QString MainWindow::formatNumber(double value)
{
    // 移除尾部的.0
    if (value == floor(value)) {
        return QString::number(static_cast<long long>(value));
    } else {
        // 保留必要的小数位数
        QString str = QString::number(value, 'g', 15);
        // 移除尾部的0
        int dotPos = str.indexOf('.');
        if (dotPos != -1) {
            while (str.back() == '0') {
                str.chop(1);
            }
            // 如果最后一个字符是小数点，也移除
            if (str.back() == '.') {
                str.chop(1);
            }
        }
        return str;
    }
}
