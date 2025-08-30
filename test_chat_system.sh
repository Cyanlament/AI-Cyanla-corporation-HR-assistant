#!/bin/bash

# HospAI 聊天系统测试脚本
# 可以启动多个实例来测试患者与客服之间的聊天功能

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
EXEC_PATH="$BUILD_DIR/HospAI"

# 检查可执行文件是否存在
if [ ! -f "$EXEC_PATH" ]; then
    echo "❌ 错误：未找到可执行文件 $EXEC_PATH"
    echo "请先编译项目：cd build && make"
    exit 1
fi

# 显示使用说明
show_usage() {
    echo "🏥 HospAI 聊天系统测试脚本"
    echo ""
    echo "用法："
    echo "  $0 [选项]"
    echo ""
    echo "选项："
    echo "  patient    - 启动患者端实例"
    echo "  staff      - 启动客服端实例"
    echo "  admin      - 启动管理员端实例"
    echo "  multi      - 启动多个实例用于测试聊天"
    echo "  help       - 显示此帮助信息"
    echo ""
    echo "测试聊天系统："
    echo "  1. 运行 '$0 multi' 启动多个实例"
    echo "  2. 或者手动启动多个终端，分别运行 '$0 patient' 和 '$0 staff'"
    echo "  3. 在患者端使用AI分诊，然后点击'转人工客服'"
    echo "  4. 切换到'客服咨询'选项卡与客服聊天"
    echo ""
    echo "测试账号："
    echo "  管理员： admin / admin123"
    echo "  客服：   staff1 / staff123 或 staff2 / staff123"
    echo "  患者：   patient1 / patient123 或 patient2 / patient123"
}

# 启动单个实例
start_instance() {
    local instance_type="$1"
    echo "🚀 启动 $instance_type 实例..."
    cd "$BUILD_DIR"
    ./HospAI &
    local pid=$!
    echo "✅ $instance_type 实例已启动 (PID: $pid)"
    return $pid
}

# 启动多个实例用于测试
start_multi_test() {
    echo "🏥 启动多实例聊天测试..."
    echo ""
    echo "将启动3个HospAI实例："
    echo "  - 实例1：建议登录为患者 (patient1 / patient123)"
    echo "  - 实例2：建议登录为客服 (staff1 / staff123)"
    echo "  - 实例3：建议登录为另一个患者 (patient2 / patient123)"
    echo ""
    echo "测试步骤："
    echo "  1. 在患者实例中使用'智能分诊'功能"
    echo "  2. 点击'转人工客服'按钮"
    echo "  3. 切换到'客服咨询'选项卡"
    echo "  4. 在客服实例中查看和回复患者消息"
    echo ""
    read -p "按回车键继续启动实例..."
    
    # 启动3个实例
    start_instance "患者端1"
    sleep 2
    start_instance "客服端"
    sleep 2
    start_instance "患者端2"
    
    echo ""
    echo "✅ 所有实例已启动完成！"
    echo "💡 提示：可以在不同实例中登录不同角色的账号进行测试"
    echo ""
    echo "关闭测试：按 Ctrl+C 或关闭终端窗口"
    
    # 等待用户中断
    echo "等待测试完成..."
    while true; do
        sleep 1
    done
}

# 解析命令行参数
case "${1:-help}" in
    "patient")
        echo "🏥 启动患者端..."
        echo "建议使用账号：patient1 / patient123 或 patient2 / patient123"
        start_instance "患者端"
        wait
        ;;
    "staff")
        echo "👩‍⚕️ 启动客服端..."
        echo "建议使用账号：staff1 / staff123 或 staff2 / staff123"
        start_instance "客服端"
        wait
        ;;
    "admin")
        echo "👨‍💼 启动管理员端..."
        echo "建议使用账号：admin / admin123"
        start_instance "管理员端"
        wait
        ;;
    "multi")
        start_multi_test
        ;;
    "help"|"-h"|"--help")
        show_usage
        ;;
    *)
        echo "❌ 未知选项: $1"
        echo ""
        show_usage
        exit 1
        ;;
esac 