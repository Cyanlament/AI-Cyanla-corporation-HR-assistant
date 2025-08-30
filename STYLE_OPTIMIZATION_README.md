# HospAI 跨平台样式优化说明

## 概述

本次优化主要解决了 Qt 应用在不同操作系统（Windows、macOS、Ubuntu/Linux）上显示不一致的问题，通过统一的样式管理系统确保在各平台上都有良好的用户体验。

## 主要优化内容

### 1. 跨平台字体优化

**问题**: 原来的字体设置主要针对 Windows 环境，在 Ubuntu 上可能显示不佳。

**解决方案**:
- 为不同平台设置合适的字体优先级：
  - **Windows**: Microsoft YaHei UI → Microsoft YaHei → Segoe UI
  - **macOS**: PingFang SC → Helvetica Neue → San Francisco
  - **Linux/Ubuntu**: Ubuntu → Noto Sans CJK SC → Source Han Sans SC → WenQuanYi Micro Hei

- 针对不同平台调整字体大小：
  - Windows: 9pt（适配高DPI）
  - macOS: 13pt（适配Retina显示）
  - Linux: 10pt（平衡清晰度和美观）

### 2. 统一颜色方案

**更新前** (Ubuntu偏向):
```cpp
QString primary = "#E95420";        // Ubuntu 橙色
QString secondary = "#77216F";      // Ubuntu 紫色
```

**更新后** (跨平台友好):
```cpp
QString primary = "#007ACC";        // 现代蓝色
QString secondary = "#6C757D";      // 中性灰
QString background = "#F8F9FA";     // 极浅灰
```

### 3. 控件样式统一

#### 按钮样式
- 统一圆角：6px
- 一致的内边距：8px 16px
- 清晰的悬停效果
- 适当的禁用状态

#### 输入框样式
- 统一边框样式：1px solid
- 焦点高亮效果：蓝色边框 + 阴影
- 一致的内边距：8px 12px

#### 选项卡样式
- 现代化的选项卡设计
- 清晰的选中状态指示
- 统一的最小宽度：80px

### 4. 高DPI显示支持

```cpp
void UIStyleManager::optimizeForHighDPI(QApplication *app)
{
    // Qt6 中高DPI缩放默认开启
    // 只在Qt5中需要手动设置属性
    
    // 检测屏幕DPI并微调字体大小
    if (scale > 1.5) {
        // 高DPI模式下字体放大10%
        font.setPointSize(qRound(font.pointSize() * 1.1));
    }
}
```

### 5. 系统样式表优化

**新增特性**:
- 全局文本选择颜色统一
- 改进的滚动条样式
- 更好的工具提示外观
- 一致的边框和阴影效果

## 使用方法

### 1. 在应用启动时应用样式

```cpp
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 应用跨平台优化的全局样式
    UIStyleManager::setupFonts();
    UIStyleManager::applyGlobalStyleSheet(&app);
    
    // ... 应用代码
    
    return app.exec();
}
```

### 2. 为特定控件应用样式

```cpp
// 按钮样式
UIStyleManager::applyButtonStyle(button, "primary");    // 主要按钮
UIStyleManager::applyButtonStyle(button, "secondary");  // 次要按钮
UIStyleManager::applyButtonStyle(button, "success");    // 成功按钮
UIStyleManager::applyButtonStyle(button, "warning");    // 警告按钮
UIStyleManager::applyButtonStyle(button, "error");      // 错误按钮

// 输入框样式
UIStyleManager::applyLineEditStyle(lineEdit);

// 文本编辑器样式
UIStyleManager::applyTextEditStyle(textEdit);

// 分组框样式
UIStyleManager::applyGroupBoxStyle(groupBox);
```

### 3. 运行样式演示

```bash
cd demo_build
./StyleDemo
```

## 兼容性

- **Qt版本**: Qt5.15+ 和 Qt6.x
- **操作系统**: 
  - Windows 10/11
  - macOS 10.15+
  - Ubuntu 18.04+
  - 其他主流Linux发行版

## 效果展示

### 主要改进

1. **字体渲染**: 在各平台都使用最优字体，提升中文显示效果
2. **颜色对比**: 改善文字和背景的对比度，提升可读性
3. **控件一致性**: 按钮、输入框等控件在各平台表现一致
4. **高DPI支持**: 自动适配高分辨率显示器
5. **现代化外观**: 扁平化设计，符合现代UI趋势

### 技术特点

- **响应式设计**: 自动适配不同屏幕尺寸和DPI
- **无障碍友好**: 良好的颜色对比度和字体大小
- **性能优化**: 样式表经过优化，减少渲染开销
- **易于维护**: 集中的样式管理，便于后续调整

## 注意事项

1. 首次运行时会在控制台输出字体选择信息，用于调试
2. 在高DPI显示器上会自动调整字体大小
3. 如需自定义颜色，请修改 `UIStyleManager::ColorScheme` 结构体
4. 建议在不同平台上测试确保效果符合预期

## 文件结构

```
src/views/common/
├── UIStyleManager.h         # 样式管理器头文件
├── UIStyleManager.cpp       # 样式管理器实现
└── ...

style_demo.cpp               # 样式演示程序
demo_CMakeLists.txt         # 演示程序构建文件
```

这次优化确保了 HospAI 应用在 Windows、macOS 和 Ubuntu 上都能提供一致、美观的用户界面体验。 