#!/bin/bash

echo "==============================================="
echo "HospAI 患者端状态显示修复测试"
echo "==============================================="

echo ""
echo "启动说明："
echo "1. 此脚本将启动3个进程："
echo "   - 患者1 (patient1/patient123)"
echo "   - 客服1 (staff1/staff123)" 
echo "   - 客服2 (staff2/staff123)"
echo ""
echo "2. 测试步骤："
echo "   - 在患者1界面：点击'转人工服务'按钮"
echo "   - 患者1状态应显示: '等待客服接入...'"
echo "   - 在客服1界面：接受该患者会话"
echo "   - 患者1状态应立即更新为: '与 客服名 对话中'"
echo "   - 开始正常聊天测试"
echo ""
echo "3. 验证要点："
echo "   - 客服接入后患者端标题立即更新"
echo "   - 显示正确的客服名称"
echo "   - 聊天状态保持正确"
echo ""

read -p "按任意键开始启动测试进程... " -n1 -s
echo ""

# 构建目录
BUILD_DIR="./build"

if [ ! -d "$BUILD_DIR" ]; then
    echo "错误: 构建目录 $BUILD_DIR 不存在"
    echo "请先运行 cmake 和 make 命令"
    exit 1
fi

if [ ! -f "$BUILD_DIR/HospAI" ]; then
    echo "错误: HospAI 可执行文件不存在"
    echo "请先编译项目"
    exit 1
fi

echo "启动 HospAI 实例..."
echo ""

# 启动患者1
echo "[患者1] 启动中..."
cd "$BUILD_DIR"
./HospAI --role=patient --user=patient1 --pass=patient123 &
PATIENT1_PID=$!
echo "患者1 PID: $PATIENT1_PID"

sleep 3

# 启动客服1
echo "[客服1] 启动中..."
./HospAI --role=staff --user=staff1 --pass=staff123 &
STAFF1_PID=$!
echo "客服1 PID: $STAFF1_PID"

sleep 3

# 启动客服2 (备用)
echo "[客服2] 启动中..."
./HospAI --role=staff --user=staff2 --pass=staff123 &
STAFF2_PID=$!
echo "客服2 PID: $STAFF2_PID"

echo ""
echo "所有进程已启动完成！"
echo ""
echo "测试指导："
echo "1. 在患者1窗口中点击 '转人工服务' 按钮"
echo "2. 观察状态是否显示 '等待客服接入...'"
echo "3. 在客服1窗口中接受该会话"
echo "4. 检查患者1状态是否立即更新为 '与客服对话中'"
echo "5. 尝试双向聊天测试"
echo ""
echo "进程信息："
echo "患者1: PID $PATIENT1_PID"
echo "客服1: PID $STAFF1_PID"  
echo "客服2: PID $STAFF2_PID"
echo ""
echo "测试完成后可以按 Ctrl+C 结束，或运行以下命令："
echo "kill $PATIENT1_PID $STAFF1_PID $STAFF2_PID"
echo ""

# 等待用户中断
trap "echo ''; echo '正在关闭所有进程...'; kill $PATIENT1_PID $STAFF1_PID $STAFF2_PID 2>/dev/null; exit 0" INT

echo "测试进行中... (按 Ctrl+C 退出)"
wait 