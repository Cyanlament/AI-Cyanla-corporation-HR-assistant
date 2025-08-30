#!/bin/bash

# HospAI 聊天功能演示启动脚本

echo "======================================"
echo "      HospAI 聊天功能演示"
echo "======================================"
echo ""

# 编译程序（如果需要）
if [ ! -f "chat_demo_build/ChatDemo" ]; then
    echo "正在编译聊天演示程序..."
    cd chat_demo_build
    make
    cd ..
    echo "编译完成！"
    echo ""
fi

echo "启动选项："
echo "1. 启动单个演示窗口"
echo "2. 启动患者模拟器"
echo "3. 启动客服模拟器"
echo "4. 启动多个演示窗口 (患者+客服)"
echo ""

read -p "请选择启动方式 (1-4): " choice

case $choice in
    1)
        echo "启动单个演示窗口..."
        cd chat_demo_build
        ./ChatDemo &
        ;;
    2)
        echo "启动患者模拟器..."
        cd chat_demo_build
        ./ChatDemo &
        sleep 2
        echo "提示：点击「模拟患者登录」按钮开始聊天"
        ;;
    3)
        echo "启动客服模拟器..."
        cd chat_demo_build
        ./ChatDemo &
        sleep 2
        echo "提示：点击「模拟客服登录」按钮查看等待接入的会话"
        ;;
    4)
        echo "启动多个演示窗口..."
        echo "启动患者端..."
        cd chat_demo_build
        ./ChatDemo &
        PATIENT_PID=$!
        sleep 2
        
        echo "启动客服端..."
        ./ChatDemo &
        STAFF_PID=$!
        sleep 2
        
        echo ""
        echo "======================================"
        echo "多用户聊天演示已启动！"
        echo ""
        echo "患者端 PID: $PATIENT_PID"
        echo "客服端 PID: $STAFF_PID"
        echo ""
        echo "使用说明："
        echo "1. 在第一个窗口点击「模拟患者登录」"
        echo "2. 点击「开始咨询」按钮发起聊天"
        echo "3. 在第二个窗口点击「模拟客服登录」"
        echo "4. 双击等待列表中的会话接入对话"
        echo "5. 现在可以在两个窗口间进行实时聊天！"
        echo ""
        echo "按 Ctrl+C 结束所有进程"
        echo "======================================"
        
        # 等待用户按Ctrl+C
        trap "echo '正在关闭所有进程...'; kill $PATIENT_PID $STAFF_PID 2>/dev/null; exit" INT
        wait
        ;;
    *)
        echo "无效选择，退出..."
        exit 1
        ;;
esac

echo ""
echo "演示程序已启动！"
echo ""
echo "功能特性："
echo "✅ 实时消息传递"
echo "✅ 一对一客服聊天"
echo "✅ 会话状态管理"
echo "✅ 消息已读状态"
echo "✅ 在线状态显示"
echo "✅ 自动消息检测"
echo ""
echo "数据库文件位置："
echo "~/.local/share/HospAI/hospai.db"
echo ""
echo "可以启动更多进程测试多用户场景！" 