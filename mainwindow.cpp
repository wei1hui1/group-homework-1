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
#include <algorithm>
#include <numeric>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    memoryValue(0.0),
    historyList(),
    calculationComplete(false),
    historyMode(false),
    currentHistoryIndex(0),
    formatMode(0)
{
    ui->setupUi(this);
    loadHistory();

    // 连接数字按钮
    connect(ui->_7, SIGNAL(clicked()), this, SLOT(onDigitClicked()));
    connect(ui->_8, SIGNAL(clicked()), this, SLOT(onDigitClicked()));
    connect(ui->_9, SIGNAL(clicked()), this, SLOT(onDigitClicked()));
    connect(ui->_4, SIGNAL(clicked()), this, SLOT(onDigitClicked()));
    connect(ui->_5, SIGNAL(clicked()), this, SLOT(onDigitClicked()));
    connect(ui->_6, SIGNAL(clicked()), this, SLOT(onDigitClicked()));
    connect(ui->_1, SIGNAL(clicked()), this, SLOT(onDigitClicked()));
    connect(ui->_2, SIGNAL(clicked()), this, SLOT(onDigitClicked()));
    connect(ui->_3, SIGNAL(clicked()), this, SLOT(onDigitClicked()));
    connect(ui->_0, SIGNAL(clicked()), this, SLOT(onDigitClicked()));
    connect(ui->point, SIGNAL(clicked()), this, SLOT(onDotClicked()));

    // 连接运算符按钮
    connect(ui->multiply, SIGNAL(clicked()), this, SLOT(onOperatorClicked()));
    connect(ui->division, SIGNAL(clicked()), this, SLOT(onOperatorClicked()));
    connect(ui->add, SIGNAL(clicked()), this, SLOT(onOperatorClicked()));
    connect(ui->minus, SIGNAL(clicked()), this, SLOT(onOperatorClicked()));
    connect(ui->FB, SIGNAL(clicked()), this, SLOT(onLeftParenthesisClicked()));
    connect(ui->BB, SIGNAL(clicked()), this, SLOT(onRightParenthesisClicked()));

    // 连接功能按钮
    connect(ui->equal, SIGNAL(clicked()), this, SLOT(onEqualsClicked()));
    connect(ui->AC, SIGNAL(clicked()), this, SLOT(onBackspaceClicked()));
    connect(ui->del, SIGNAL(clicked()), this, SLOT(onDelClicked()));
    connect(ui->record, SIGNAL(clicked()), this, SLOT(onHistoryClicked()));

    // 连接内存功能按钮
    connect(ui->mc, SIGNAL(clicked()), this, SLOT(onMemoryClearClicked()));
    connect(ui->m_add, SIGNAL(clicked()), this, SLOT(onMemoryAddClicked()));
    connect(ui->m_minus, SIGNAL(clicked()), this, SLOT(onMemorySubtractClicked()));
    connect(ui->mr, SIGNAL(clicked()), this, SLOT(onMemoryRecallClicked()));

    // 连接其他功能按钮
    connect(ui->percent, SIGNAL(clicked()), this, SLOT(onPercentClicked()));
    connect(ui->AC, SIGNAL(clicked()), this, SLOT(onClearClicked()));
    
    // 连接导航和功能按钮
    connect(ui->back, SIGNAL(clicked()), this, SLOT(onBackClicked()));
    connect(ui->left, SIGNAL(clicked()), this, SLOT(onLeftClicked()));
    connect(ui->right, SIGNAL(clicked()), this, SLOT(onRightClicked()));
    connect(ui->OK, SIGNAL(clicked()), this, SLOT(onOKClicked()));
    connect(ui->All, SIGNAL(clicked()), this, SLOT(onAllClicked()));
    connect(ui->form, SIGNAL(clicked()), this, SLOT(onFormClicked()));
    connect(ui->square, SIGNAL(clicked()), this, SLOT(onSquareClicked()));
    connect(ui->sqrt, SIGNAL(clicked()), this, SLOT(onSqrtClicked()));

    // 初始化显示
    ui->textEdit->setAlignment(Qt::AlignRight);
    ui->lineEdit->setAlignment(Qt::AlignRight);
    ui->textEdit->setReadOnly(false);
    ui->lineEdit->setReadOnly(true);
    ui->textEdit->setText("");
    ui->lineEdit->setText("");
    
    // 设置样式
    ui->textEdit->setReadOnly(false);
    ui->textEdit->setFocusPolicy(Qt::StrongFocus);
    ui->textEdit->setAcceptRichText(false);
    ui->textEdit->setCursorWidth(2);
    ui->textEdit->setStyleSheet("background-color: white; color: black;");
    
    // 设置字体大小
    QFont textFont = ui->textEdit->font();
    textFont.setPointSize(20);
    ui->textEdit->setFont(textFont);
    
    QFont lineFont = ui->lineEdit->font();
    lineFont.setPointSize(24);
    lineFont.setWeight(QFont::Bold);
    ui->lineEdit->setFont(lineFont);
    
    // 设置焦点
    ui->textEdit->setFocus();
}

MainWindow::~MainWindow()
{
    saveHistory();
    delete ui;
}

void MainWindow::onDigitClicked()
{
    if (historyMode) return;
    
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    QString digit = button->text();
    QTextCursor cursor = ui->textEdit->textCursor();
    QString currentText = ui->textEdit->toPlainText();

    if (calculationComplete) {
        ui->textEdit->setText(digit);
        ui->lineEdit->setText("");
        calculationComplete = false;
        QFont font = ui->textEdit->font();
        font.setPointSize(20);
        font.setWeight(QFont::Normal);
        ui->textEdit->setFont(font);
        ui->textEdit->setStyleSheet("color: black;");
    } else {
        // 在光标位置插入数字
        cursor.insertText(digit);
        ui->textEdit->setTextCursor(cursor);
    }

    // 确保光标可见
    ui->textEdit->ensureCursorVisible();
    updatePreview();
}

void MainWindow::onDotClicked()
{
    if (historyMode) return;
    
    QString currentText = ui->textEdit->toPlainText();
    QTextCursor cursor = ui->textEdit->textCursor();

    if (calculationComplete) {
        ui->textEdit->setText("0.");
        ui->lineEdit->setText("");
        calculationComplete = false;
        QFont font = ui->textEdit->font();
        font.setPointSize(20);
        font.setWeight(QFont::Normal);
        ui->textEdit->setFont(font);
        ui->textEdit->setStyleSheet("color: black;");
    } else {
        // 检查当前数字是否已有小数点
        int cursorPos = cursor.position();
        QString textBeforeCursor = currentText.left(cursorPos);
        
        QRegularExpression regex("\\d*\\.\\d*$");
        if (!regex.match(textBeforeCursor).hasMatch()) {
            cursor.insertText(".");
            ui->textEdit->setTextCursor(cursor);
        }
    }

    ui->textEdit->ensureCursorVisible();
    updatePreview();
}

void MainWindow::onOperatorClicked()
{
    if (historyMode) return;
    
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

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
        operatorText = button->text();
    }

    QTextCursor cursor = ui->textEdit->textCursor();
    QString currentText = ui->textEdit->toPlainText();

    if (calculationComplete) {
        ui->textEdit->setText(ui->lineEdit->text() + operatorText);
        ui->lineEdit->setText("");
        calculationComplete = false;
        QFont font = ui->textEdit->font();
        font.setPointSize(20);
        font.setWeight(QFont::Normal);
        ui->textEdit->setFont(font);
        ui->textEdit->setStyleSheet("color: black;");
    } else {
        // 在光标位置插入运算符
        cursor.insertText(operatorText);
        ui->textEdit->setTextCursor(cursor);
    }

    ui->textEdit->ensureCursorVisible();
    updatePreview();
}

void MainWindow::onLeftParenthesisClicked()
{
    if (historyMode) return;
    
    QTextCursor cursor = ui->textEdit->textCursor();
    QString currentText = ui->textEdit->toPlainText();

    if (calculationComplete) {
        ui->textEdit->setText(ui->lineEdit->text() + "(");
        ui->lineEdit->setText("");
        calculationComplete = false;
        QFont font = ui->textEdit->font();
        font.setPointSize(20);
        font.setWeight(QFont::Normal);
        ui->textEdit->setFont(font);
        ui->textEdit->setStyleSheet("color: black;");
    } else {
        cursor.insertText("(");
        ui->textEdit->setTextCursor(cursor);
    }

    ui->textEdit->ensureCursorVisible();
    updatePreview();
}

void MainWindow::onRightParenthesisClicked()
{
    if (historyMode) return;
    
    QTextCursor cursor = ui->textEdit->textCursor();
    QString currentText = ui->textEdit->toPlainText();

    int leftCount = currentText.count('(');
    int rightCount = currentText.count(')');

    if (leftCount > rightCount) {
        cursor.insertText(")");
        ui->textEdit->setTextCursor(cursor);
    }

    ui->textEdit->ensureCursorVisible();
    updatePreview();
}

void MainWindow::onEqualsClicked()
{
    if (historyMode) return;
    
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

        // 改变样式
        QFont font = ui->textEdit->font();
        font.setPointSize(20);
        font.setWeight(QFont::Light);
        ui->textEdit->setFont(font);
        ui->textEdit->setStyleSheet("color: gray;");

        QFont resultFont = ui->lineEdit->font();
        resultFont.setPointSize(24);
        resultFont.setWeight(QFont::Bold);
        ui->lineEdit->setFont(resultFont);
        ui->lineEdit->setStyleSheet("color: black;");
    } catch (const QString& error) {
        QMessageBox::warning(this, "Calculation Error", error);
    }
}

void MainWindow::onClearClicked()
{
    if (historyMode) {
        exitHistoryMode();
        return;
    }
    
    ui->textEdit->clear();
    ui->lineEdit->clear();
    calculationComplete = false;

    QFont font = ui->textEdit->font();
    font.setPointSize(20);
    font.setWeight(QFont::Normal);
    ui->textEdit->setFont(font);
    ui->textEdit->setStyleSheet("color: black;");

    QFont resultFont = ui->lineEdit->font();
    resultFont.setPointSize(24);
    resultFont.setWeight(QFont::Normal);
    ui->lineEdit->setFont(resultFont);
    ui->lineEdit->setStyleSheet("color: black;");
}

void MainWindow::onBackspaceClicked()
{
    if (historyMode) return;
    
    QTextCursor cursor = ui->textEdit->textCursor();
    if (cursor.position() > 0) {
        cursor.deletePreviousChar();
        ui->textEdit->setTextCursor(cursor);
        updatePreview();
    }
    ui->textEdit->ensureCursorVisible();
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

    QFont font = ui->textEdit->font();
    font.setPointSize(20);
    font.setWeight(QFont::Normal);
    ui->textEdit->setFont(font);
    ui->textEdit->setStyleSheet("color: black;");
}

void MainWindow::onPercentClicked()
{
    if (historyMode) return;
    
    try {
        if (calculationComplete) {
            double value = ui->lineEdit->text().toDouble();
            value /= 100.0;
            ui->textEdit->setText(formatNumber(value));
            ui->lineEdit->setText("");
            calculationComplete = false;
            QFont font = ui->textEdit->font();
            font.setPointSize(20);
            font.setWeight(QFont::Normal);
            ui->textEdit->setFont(font);
            ui->textEdit->setStyleSheet("color: black;");
        } else {
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
    if (historyMode) return;
    
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

    return -1;
}

double MainWindow::calculate(const QString& expression)
{
    QString processedExpr = expression;
    
    // 处理平方运算
    QRegularExpression squareRegex("(\\d+\\.\\d+|\\d+)\\^2");
    QRegularExpressionMatchIterator squareIt = squareRegex.globalMatch(processedExpr);
    while (squareIt.hasNext()) {
        QRegularExpressionMatch match = squareIt.next();
        double num = match.captured(1).toDouble();
        double squared = num * num;
        processedExpr.replace(match.capturedStart(), match.capturedLength(), QString::number(squared));
    }
    
    // 处理开方运算
    QRegularExpression sqrtRegex("sqrt\\((\\d+\\.\\d+|\\d+)\\)");
    QRegularExpressionMatchIterator sqrtIt = sqrtRegex.globalMatch(processedExpr);
    while (sqrtIt.hasNext()) {
        QRegularExpressionMatch match = sqrtIt.next();
        double num = match.captured(1).toDouble();
        if (num < 0) {
            throw QString("Cannot sqrt negative number");
        }
        double sqrtResult = sqrt(num);
        processedExpr.replace(match.capturedStart(), match.capturedLength(), QString::number(sqrtResult));
    }

    int leftCount = processedExpr.count('(');
    int rightCount = processedExpr.count(')');
    if (leftCount != rightCount) {
        throw QString("Parentheses not matched");
    }

    QString expr = processedExpr;
    int pos = expr.indexOf('(');
    while (pos != -1) {
        int endPos = findMatchingParenthesis(expr, pos);
        if (endPos == -1) {
            throw QString("Parentheses not matched");
        }

        QString subExpr = expr.mid(pos + 1, endPos - pos - 1);
        double subResult = evaluateExpression(subExpr);

        expr.replace(pos, endPos - pos + 1, QString::number(subResult));
        pos = expr.indexOf('(');
    }

    return evaluateExpression(expr);
}

double MainWindow::evaluateExpression(QString expr)
{
    if (expr.startsWith('-')) {
        expr.prepend('0');
    }
    expr.replace("(-", "(0-");

    QRegularExpression re("(\\d+\\.\\d+|\\d+|[+\\-*/])");
    QRegularExpressionMatchIterator it = re.globalMatch(expr);

    QList<QString> tokens;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        tokens.append(match.captured(1));
    }

    if (tokens.isEmpty()) {
        throw QString("Empty expression");
    }

    if (tokens.first() == "+" || tokens.first() == "*" || tokens.first() == "/") {
        throw QString("Invalid expression");
    }

    if (tokens.last() == "+" || tokens.last() == "-" || tokens.last() == "*" || tokens.last() == "/") {
        throw QString("Invalid expression");
    }

    QList<double> numbers;
    QList<QString> ops;

    numbers.append(tokens[0].toDouble());

    for (int i = 1; i < tokens.size(); i += 2) {
        if (i >= tokens.size() || i+1 >= tokens.size()) {
            throw QString("Expression format error");
        }
        
        QString op = tokens[i];
        double num = tokens[i+1].toDouble();

        if (op == "*") {
            numbers.last() *= num;
        } else if (op == "/") {
            if (num == 0) {
                throw QString("Division by zero");
            }
            numbers.last() /= num;
        } else {
            numbers.append(num);
            ops.append(op);
        }
    }

    if (numbers.isEmpty()) {
        throw QString("Calculation error");
    }
    
    double result = numbers[0];
    for (int i = 0; i < ops.size(); i++) {
        if (i+1 >= numbers.size()) {
            throw QString("Expression format error");
        }
        
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
    QFile file("history.json");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Cannot open history file for writing";
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
    QFile file("history.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open history file for reading";
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
    const int maxHistory = 20;
    if (historyList.size() >= maxHistory) {
        historyList.removeLast();
    }
    historyList.prepend(entry);
    saveHistory();
}

void MainWindow::displayHistory()
{
    if (historyList.isEmpty()) {
        ui->textEdit->setPlainText("no history record");
        return;
    }
    
    QString displayText;
    int displayCount = qMin(3, historyList.size()); // 最多显示3条记录
    int startIndex = qMax(0, currentHistoryIndex - 1); // 以当前选中项为中心
    
    // 调整起始索引，确保能显示3条记录
    if (startIndex + displayCount > historyList.size()) {
        startIndex = historyList.size() - displayCount;
    }
    
    for (int i = startIndex; i < startIndex + displayCount; ++i) {
        QString entry = historyList[i];
        if (i == currentHistoryIndex) {
            // 当前选中的记录加粗显示
            displayText += "> " + entry + "\n";
        } else {
            // 其他记录正常显示
            displayText += "  " + entry + "\n";
        }
    }
    
    // 移除最后一个换行符
    if (!displayText.isEmpty()) {
        displayText.chop(1);
    }
    
    
    
    ui->textEdit->setPlainText(displayText);
    ui->textEdit->setReadOnly(true);
    ui->lineEdit->clear();
}

void MainWindow::exitHistoryMode()
{
    if (historyMode) {
        historyMode = false;
        // 保留textEdit中的表达式
        // ui->textEdit->clear();
        ui->lineEdit->clear();
        calculationComplete = false;
        ui->textEdit->setReadOnly(false);
        
        QFont textFont = ui->textEdit->font();
        textFont.setPointSize(20);
        textFont.setWeight(QFont::Normal);
        ui->textEdit->setFont(textFont);
        ui->textEdit->setStyleSheet("color: black;");
        
        ui->textEdit->setFocus();
        updatePreview(); // 更新预览
    }
}

void MainWindow::updatePreview(QString calcExpr)
{
    if (historyMode) return;
    
    QString expression;
    
    if (calcExpr.isEmpty()) {
        expression = ui->textEdit->toPlainText();
    } else {
        expression = calcExpr;
    }

    if (expression.isEmpty()) {
        ui->lineEdit->setText("");
        return;
    }

    QChar lastChar = expression.back();
    if (lastChar == '+' || lastChar == '-' || lastChar == '*' || lastChar == '/') {
        ui->lineEdit->setText("");
        return;
    }

    try {
        double result = calculate(expression);
        QString resultStr = formatNumber(result);

        ui->lineEdit->setText(resultStr);
        QFont font = ui->lineEdit->font();
        font.setPointSize(24);
        font.setWeight(QFont::Bold);
        ui->lineEdit->setFont(font);
        ui->lineEdit->setStyleSheet("color: gray;");
    } catch (const QString&) {
        ui->lineEdit->setText("");
    }
}

void MainWindow::onBackClicked()
{
    if (historyMode) {
        exitHistoryMode();
    } else if (!historyList.isEmpty()) {
        QString latestEntry = historyList[0];
        int equalsPos = latestEntry.indexOf('=');
        if (equalsPos != -1) {
            QString expression = latestEntry.left(equalsPos).trimmed();
            QString result = latestEntry.mid(equalsPos + 1).trimmed();
            
            ui->textEdit->setText(expression);
            ui->lineEdit->setText(result);
            
            QFont textFont = ui->textEdit->font();
            textFont.setPointSize(20);
            textFont.setWeight(QFont::Normal);
            ui->textEdit->setFont(textFont);
            ui->textEdit->setStyleSheet("color: black;");
            
            QFont lineFont = ui->lineEdit->font();
            lineFont.setPointSize(24);
            lineFont.setWeight(QFont::Bold);
            ui->lineEdit->setFont(lineFont);
            ui->lineEdit->setStyleSheet("color: gray;");
            
            calculationComplete = true;
        }
    }
}

void MainWindow::onLeftClicked()
{
    if (historyMode) {
        if (currentHistoryIndex > 0) {
            currentHistoryIndex--;
            displayHistory();
        }
    } else {
        QTextCursor cursor = ui->textEdit->textCursor();
        if (cursor.position() > 0) {
            cursor.movePosition(QTextCursor::Left);
            ui->textEdit->setTextCursor(cursor);
        }
    }
    ui->textEdit->ensureCursorVisible();
}

void MainWindow::onRightClicked()
{
    if (historyMode) {
        if (currentHistoryIndex < historyList.size() - 1) {
            currentHistoryIndex++;
            displayHistory();
        }
    } else {
        QTextCursor cursor = ui->textEdit->textCursor();
        if (cursor.position() < ui->textEdit->toPlainText().length()) {
            cursor.movePosition(QTextCursor::Right);
            ui->textEdit->setTextCursor(cursor);
        }
    }
    ui->textEdit->ensureCursorVisible();
}

void MainWindow::onOKClicked()
{
    if (historyMode) {
        if (ui->lineEdit->text() == "Press OK to confirm delete") {
            if (!historyList.isEmpty() && currentHistoryIndex >= 0 && currentHistoryIndex < historyList.size()) {
                // 删除选中的历史记录
                QString selectedEntry = historyList[currentHistoryIndex];
                historyList.removeAt(currentHistoryIndex);
                saveHistory();
                
                // 修复索引调整逻辑
                if (historyList.isEmpty()) {
                    currentHistoryIndex = -1; // 如果列表为空
                } else if (currentHistoryIndex >= historyList.size()) {
                    currentHistoryIndex = historyList.size() - 1; // 如果删除的是最后一项
                }
                // 如果删除的是中间项，currentHistoryIndex会自动指向后一项
                
                displayHistory();
                ui->lineEdit->setText("Deleted: " + selectedEntry);
                
                // 2秒后清除提示
                QTimer::singleShot(2000, this, [this]() {
                    if (ui->lineEdit->text().startsWith("Deleted: ")) {
                        ui->lineEdit->clear();
                    }
                });
            }
        } else if (ui->lineEdit->text() == "Confirm delete all history?") {
            // 删除所有历史记录
            historyList.clear();
            saveHistory();
            displayHistory();
            ui->lineEdit->setText("All records deleted");
            
            // 2秒后清除提示
            QTimer::singleShot(2000, this, [this]() {
                if (ui->lineEdit->text() == "All records deleted") {
                    ui->lineEdit->clear();
                }
            });
        } else {
            // 在历史模式下按OK，将选中的记录加载到计算器
            if (!historyList.isEmpty() && currentHistoryIndex >= 0 && currentHistoryIndex < historyList.size()) {
                QString selectedEntry = historyList[currentHistoryIndex];
                int equalsPos = selectedEntry.indexOf('=');
                if (equalsPos != -1) {
                    QString expression = selectedEntry.left(equalsPos).trimmed();
                    QString result = selectedEntry.mid(equalsPos + 1).trimmed();
                    
                    ui->textEdit->setText(expression);
                    ui->lineEdit->setText(result);
                    calculationComplete = true;
                    
                    exitHistoryMode();
                }
            }
        }
    } else if (calculationComplete) {
        ui->textEdit->setText(ui->lineEdit->text());
        ui->lineEdit->clear();
        calculationComplete = false;
        
        QFont font = ui->textEdit->font();
        font.setPointSize(20);
        font.setWeight(QFont::Normal);
        ui->textEdit->setFont(font);
        ui->textEdit->setStyleSheet("color: black;");
    }
}

void MainWindow::onDelClicked()
{
    if (historyMode) {
        ui->lineEdit->setText("Press OK to confirm delete");
    } else {
        QTextCursor cursor = ui->textEdit->textCursor();
        if (cursor.position() > 0) {
            cursor.deletePreviousChar();
            ui->textEdit->setTextCursor(cursor);
            updatePreview();
        }
    }
    ui->textEdit->ensureCursorVisible();
}

void MainWindow::onAllClicked()
{
    if (historyMode) {
        ui->lineEdit->setText("Confirm delete all history?");
    }
}

void MainWindow::onFormClicked()
{
    formatMode = (formatMode + 1) % 4;
    
    QString modeText;
    switch (formatMode) {
    case 0:
        modeText = "Auto";
        break;
    case 1:
        modeText = "2 Decimals";
        break;
    case 2:
        modeText = "6 Decimals";
        break;
    case 3:
        modeText = "Fraction";
        break;
    }
    ui->lineEdit->setText("Format: " + modeText);

    // 如果当前有计算结果，重新格式化显示
    if (calculationComplete && !ui->lineEdit->text().isEmpty()) {
        QString currentResult = ui->lineEdit->text();
        bool ok;
        double result = currentResult.toDouble(&ok);
        if (ok) {
            ui->lineEdit->setText(formatNumber(result));
        }
    }
    
    // 2秒后清除提示
    QTimer::singleShot(2000, this, [this]() {
        if (ui->lineEdit->text().startsWith("Format: ")) {
            ui->lineEdit->clear();
        }
    });
}

QString MainWindow::formatNumber(double value)
{
    switch (formatMode) {
    case 1: // 两位小数
        return QString::number(value, 'f', 2);
    case 2: // 六位小数
        return QString::number(value, 'f', 6);
    case 3: // 分数格式
        return formatAsFraction(value);
    default: // 自动格式
        QString str = QString::number(value, 'g', 15);
        int dotPos = str.indexOf('.');
        if (dotPos != -1) {
            while (str.back() == '0' && str.size() > dotPos + 1) {
                str.chop(1);
            }
            if (str.back() == '.') {
                str.chop(1);
            }
        }
        return str;
    }
}

QString MainWindow::formatAsFraction(double value)
{
    // 处理整数情况
    if (value == std::floor(value)) {
        return QString::number(static_cast<long long>(value));
    }
    
    // 限制分母大小以避免过长的分数
    const long long maxDenominator = 1000000;
    long long numerator = static_cast<long long>(std::round(value * maxDenominator));
    long long denominator = maxDenominator;
    
    // 计算最大公约数
    long long commonDivisor = gcd(numerator, denominator);
    
    // 约分
    numerator /= commonDivisor;
    denominator /= commonDivisor;
    
    return QString::number(numerator) + "/" + QString::number(denominator);
}

long long MainWindow::gcd(long long a, long long b)
{
    a = std::abs(a);
    b = std::abs(b);
    while (b != 0) {
        long long temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

void MainWindow::onHistoryClicked()
{
    if (historyMode) {
        exitHistoryMode();
    } else {
        historyMode = true;
        currentHistoryIndex = 0;
        displayHistory();
    }
}

void MainWindow::onSquareClicked()
{
    if (historyMode) return;
    
    if (calculationComplete) {
        // 如果计算已完成，直接对结果进行平方运算
        double result = ui->lineEdit->text().toDouble();
        double squaredResult = result * result;
        ui->textEdit->setText(QString::number(result) + "^2");
        ui->lineEdit->setText(formatNumber(squaredResult));
        calculationComplete = false;
    } else {
        // 在光标位置插入平方符号
        QTextCursor cursor = ui->textEdit->textCursor();
        cursor.insertText("^2");
        ui->textEdit->setTextCursor(cursor);
        updatePreview();
    }
    ui->textEdit->ensureCursorVisible();
}

void MainWindow::onSqrtClicked()
{
    if (historyMode) return;
    
    if (calculationComplete) {
        // 如果计算已完成，直接对结果进行开方运算
        double result = ui->lineEdit->text().toDouble();
        if (result >= 0) {
            double sqrtResult = sqrt(result);
            ui->textEdit->setText("sqrt(" + QString::number(result) + ")");
            ui->lineEdit->setText(formatNumber(sqrtResult));
        } else {
            ui->lineEdit->setText("Invalid input");
        }
        calculationComplete = false;
    } else {
        // 在光标位置插入开方符号
        QTextCursor cursor = ui->textEdit->textCursor();
        cursor.insertText("sqrt(");
        ui->textEdit->setTextCursor(cursor);
        updatePreview();
    }
    ui->textEdit->ensureCursorVisible();
}