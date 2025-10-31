# Question 11 更新日志

## v10.1.1 --> v11.0.1

### 功能改进

1. 为 `Student` 对象添加 ID、type、gender、properties 属性
2. 为 `ClassGroup` 对象添加 description 属性
3. `ClassGroup` 对象支持使用 data 或 xlsx 工作簿 导入（注：不再支持使用在图形界面中使用 data 直接导入）
4. `ClassGroup` 对象支持导出为 data 或 xlsx 工作簿（注：不再支持使用在图形界面中使用 data 直接导出）
5. `Question` 对象支持加权抽取
6. `Question` 对象可基于学生 type 调整权重

## v11.0.1 --> v11.0.2

1. 修复了内核函数 `Student Question::questionTraversal(const QString& dataPath, const QString& customID)`和内核函数 `QVector<Student> Question::getNTravQuedVec(const QString& dataPath) const`映射到 GUI 时对具有 `QK::StudentType::Excluded` 属性的学生展示以及抽取异常的问题
2. 修复了空白班级进行 `questionTraversal(...)` 时导致程序异常崩溃的问题

3. 为 `ClassGroup` 添加 `ID` 哈希表以优化性能，哈希表不被存入文件，而是直接生成

4. 为 `ClassGroup` 添加人数上限：`65535`。因为过大的数量不仅可能导致程序无响应，也有可能导致内存溢出。

5. 在 班级管理器 中添加检索功能

6. 在 新班级 窗口中，未点击保存按钮的班级将不被保存到 `Question` 对象和数据文件夹中。

7. 使用 遍历模式 时，属性 `QK::StudentType` 无效。即具有标签 `QK::StudentType::Excluded` 属性的学生也可以被抽取到。

## v11.0.2 --> v11.0.3

1. 修复了在“新班级”窗口中直接保存班级导致空白 `ClassGroup` 对象产生的问题

2. 引入 `QuestionMode` 和 `QuestionModeWithWeight` 枚举类，用于实现不同的抽取功能，彼时原先的若干接口被 `QVector<Student> question(quint32 count, QK::QuestionMode mode)` 和 `QVector<Student> questionWithWeight(quint32 count, QK::QuestionModeWithWeight mode)` 替代，暂时不移除相关代码，但是它们已被标记为`[[deprecated]]`（已废弃）

   **预计将在未来彻底移除已废弃的接口**

3. 当“重置遍历”成功后，班级将再次锁定。

## v11.0.3 --> v11.0.4 & Kernel v7.0.2 --> v7.1.0

1. 彻底移除 **废弃代码**
2. 修复了添加班级后异常出现的报错提示
3. 添加功能：导出当前抽取结果。此功能将当前抽取结果导出为 **Xlsx 表格文件**。
4. 代码整理：将多数槽函数从 lambda 表达式转换为普通槽函数。部分由于逻辑简便，并未修改。
5. 注意：对于仅调用内核的代码，也应当包含 GUI 库

## v11.0.4 --> v11.0.5 & Kernel v7.1.0 --> v7.1.1

1. 移除内核中的自定义异常，异常情况将由 `QMessagebox::critical()` 显示

2. 对于仅调用内核但未包含 GUI 库的代码，将不再产生 `QMessageBox::critical()`，而是抛出异常（**注意**：内核代码："Question.h", "Question.cpp", "ClassGroup.h", "ClassGroup.cpp", "Student.h", "Student.cpp", "global.h", "sha512.h"）
3. 同时对于某些调用 `QMessageBox::critical()` 的代码，为了给函数提供一个合理的返回值，则会使用函数中的静态变量作为“无效返回值”。未来这个功能将会使用 `std::optional()` 改写。鉴于这种改写将会影响大部分的代码，因此老式的接口将会被暂时保留。

## v11.0.5 --> v11.0.6

1. 修复了已知问题
2. 移除宏常量
3. 添加了密码可见模式按钮。默认为不可见 

## v11.0.6 --> v11.0.7 & Kernel v7.1.1 --> v7.1.2

1. 使用枚举类 `XlsxIEState` 管理 Xlsx 文档的读取成功与读取失败的状态
2. 将班级存储上限由 `65535` 改为 `9999`
3. 修复了“导出抽取结果”功能无法正常完全导出数据的问题，并修改了相关内核接口。

## v11.0.7 --> v11.1.0 & Kernel v7.1.2 --> v7.2.0

1. 新增遥控系统，可通过 `Arduino` 开发板 + 红外接收器接收指定遥控器的信号，实现普通模式下的随机抽取。

## v11.1.0 --> v11.1.1 & Kernel v7.2.0 -> v7.3.0

1. 新增历史记录系统，可保存最近100条历史记录。并支持将历史记录导出为 Xlsx 表格。

## v11.1.1 --> v11.1.2 & Kernel v7.3.0 -> v7.3.1

1. 将 `QK::Question` 类中的 `m_currentPtr` 使用 `std::optional` 进行封装，重命名为 `m_crtPtrOptional`，并对使用此指针的位置进行了 `has_value()` 检查。未改变接口。用户依然可以使用 `ClassGroup* getCurrentPtr()` 获取当前班级的指针`return m_crtPtrOptional.value()`。但如果条件 `m_crtPtrOptional.has_value()` 为假，则返回 `nullptr` 并给出警告 `qWarning`。如果启用了 `QMessageBox` 则弹出警告对话框。  

## v11.1.2 --> v11.1.3 & Kernel v7.3.1 ->v7.3.2

1. 优化了遍历抽取的功能。拆分为多个函数，增强维护性。接口不变。同时旧的函数已被弃用。您可以通过调整宏 `TRAVERSAL_QUESTION_VERSION` 的值来启用这些旧版本函数。新旧版本在功能上暂时无差异。

## v11.1.3 --> v11.1.4

1. 当看板数据过多时，会出现省略显示的情况。现使用 ToolTip 来暂时替代。鼠标悬停在图标上时，会显示每列数据的表头内容。（权重分布和性别分布图不会显示内容，只显示标题）。
