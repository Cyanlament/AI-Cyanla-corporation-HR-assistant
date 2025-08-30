# HospAI 新功能使用指南

本文档介绍了三个新实现的功能模块：HospitalNavigationWidget（医院导航）、ChatStorage（聊天存储）和 UIStyleManager（UI 样式管理）。

## 1. 医院导航系统 (HospitalNavigationWidget)

### 功能特点

- 🗺️ 虚拟医院平面图显示
- 🖱️ 点击科室名称查看导航路径
- 📍 红色路径指示和箭头标记
- 📋 详细的文字导航说明
- 🔄 清除路径功能

### 使用方法

```cpp
#include "src/views/common/HospitalNavigationWidget.h"

// 创建导航组件
HospitalNavigationWidget *navigation = new HospitalNavigationWidget(parent);

// 添加到布局中
layout->addWidget(navigation);

// 可选：设置自定义地图
navigation->setHospitalMap("path/to/hospital_map.png");

// 可选：添加自定义科室
navigation->addDepartment("新科室",
                         QPointF(x, y),           // 位置
                         QRectF(x, y, w, h),      // 点击区域
                         {point1, point2, ...},   // 路径点
                         "导航说明文字");
```

### 内置科室

- 内科、外科、儿科
- 药房、挂号处、急诊科
- 每个科室都有预设的导航路径和说明

## 2. 聊天记录存储 (ChatStorage)

### 功能特点

- 💾 SQLite 数据库存储
- 📤 插入/查询/删除聊天记录
- 🔍 多种查询方式（用户、时间范围等）
- 📊 分页查询支持
- 📄 JSON 格式导出
- 🚫 完善的错误处理机制

### 数据库结构

```sql
CREATE TABLE chat_messages (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    sender TEXT NOT NULL,
    receiver TEXT NOT NULL,
    message TEXT NOT NULL,
    timestamp DATETIME NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

### 使用方法

```cpp
#include "src/core/ChatStorage.h"

// 创建并初始化存储
ChatStorage *storage = new ChatStorage(parent);
if (!storage->initialize()) {
    qWarning() << "初始化失败:" << storage->getLastError();
}

// 插入消息
storage->insertMessage("患者001", "客服001", "你好，我需要帮助");

// 查询消息
QList<Message> messages = storage->getMessagesBetweenUsers("患者001", "客服001");

// 分页查询
QList<Message> recentMessages = storage->getMessagesWithPagination(0, 20);

// 导出数据
QString jsonData = storage->exportAllToJson();
storage->exportToFile("/path/to/export.json", messages);

// 统计信息
int totalCount = storage->getTotalMessageCount();
QStringList users = storage->getAllUsers();

// 连接信号
connect(storage, &ChatStorage::messageInserted,
        this, &MyClass::onMessageInserted);
```

### Message 结构体

```cpp
struct Message {
    int id;
    QString sender;
    QString receiver;
    QString message;
    QDateTime timestamp;

    // JSON 转换
    QJsonObject toJson() const;
    static Message fromJson(const QJsonObject &json);
};
```

## 3. UI 样式管理 (UIStyleManager)

### 功能特点

- 🎨 现代化扁平设计风格
- 🌈 统一的颜色方案
- 🔧 便捷的样式应用方法
- 💫 阴影效果支持
- 📏 统一的布局间距
- 🖼️ 多种按钮和文本样式

### 颜色方案

```cpp
UIStyleManager::ColorScheme colors = {
    .primary = "#3498db",        // 主色调-柔和蓝色
    .primaryDark = "#2980b9",    // 深蓝色
    .secondary = "#95a5a6",      // 次要色-灰色
    .background = "#ecf0f1",     // 背景色-浅灰
    .surface = "#ffffff",        // 表面色-白色
    .text = "#2c3e50",          // 文字色-深灰蓝
    .textSecondary = "#7f8c8d",  // 次要文字色
    .success = "#27ae60",       // 成功色-绿色
    .warning = "#f39c12",       // 警告色-橙色
    .error = "#e74c3c",         // 错误色-红色
    .border = "#bdc3c7",        // 边框色
    .shadow = "rgba(0,0,0,0.1)" // 阴影色
};
```

### 使用方法

```cpp
#include "src/views/common/UIStyleManager.h"

// 应用全局样式（在主程序中调用一次）
UIStyleManager::applyGlobalStyleSheet(qApp);

// 应用按钮样式
QPushButton *btn = new QPushButton("按钮");
UIStyleManager::applyButtonStyle(btn, "primary");  // primary, secondary, success, warning, error

// 应用标签样式
QLabel *label = new QLabel("标题");
UIStyleManager::applyLabelStyle(label, "title");   // title, subtitle, normal, caption, success, warning, error

// 应用框架样式（带阴影效果）
QFrame *frame = new QFrame();
UIStyleManager::applyFrameStyle(frame, true);

// 应用其他控件样式
UIStyleManager::applyLineEditStyle(lineEdit);
UIStyleManager::applyTextEditStyle(textEdit);
UIStyleManager::applyGroupBoxStyle(groupBox);
UIStyleManager::applyScrollAreaStyle(scrollArea);

// 创建阴影效果
QGraphicsDropShadowEffect *shadow = UIStyleManager::createShadowEffect(widget);
widget->setGraphicsEffect(shadow);

// 应用容器间距
UIStyleManager::applyContainerSpacing(containerWidget);
```

## 4. 完整示例程序

项目中包含了一个完整的示例程序 `ExampleUsageWidget`，展示了所有三个功能的使用方法：

```cpp
#include "src/views/common/ExampleUsageWidget.h"

// 创建示例程序
ExampleUsageWidget *example = new ExampleUsageWidget();
example->show();
```

示例程序包含三个选项卡：

1. **🗺️ 医院导航** - 展示医院导航系统
2. **💬 聊天存储** - 演示聊天记录的增删改查
3. **🎨 样式演示** - 展示各种 UI 样式效果

## 5. 编译和运行

### 依赖要求

- Qt 5.15+ 或 Qt 6.x
- QtWidgets 模块
- QtSql 模块

### 编译步骤

```bash
mkdir build
cd build
cmake ..
make
```

### 运行示例

编译完成后，可以在主程序中使用这些新组件，或者直接运行 ExampleUsageWidget 查看效果。

## 6. 注意事项

1. **数据库位置**：ChatStorage 会在用户数据目录创建 SQLite 数据库文件
2. **样式应用**：建议在应用程序启动时调用 `UIStyleManager::applyGlobalStyleSheet()`
3. **内存管理**：所有组件都支持 Qt 的父子对象内存管理
4. **信号连接**：ChatStorage 提供信号机制，便于监听数据变化
5. **错误处理**：所有操作都有相应的错误检查和处理机制

## 7. 自定义扩展

### 添加新的按钮样式

可以在 UIStyleManager 中添加新的按钮变体：

```cpp
// 在 getButtonStyleSheet 方法中添加新的 variant
else if (variant == "custom") {
    return baseStyle + customStyleString;
}
```

### 扩展聊天存储功能

可以继承 ChatStorage 类添加更多功能：

```cpp
class ExtendedChatStorage : public ChatStorage {
    // 添加自定义查询方法
    QList<Message> getMessagesByKeyword(const QString &keyword);
};
```

### 自定义医院地图

可以使用真实的医院平面图替换默认的虚拟地图：

```cpp
navigation->setHospitalMap("path/to/real_hospital_map.png");
// 然后添加对应的科室位置信息
```
