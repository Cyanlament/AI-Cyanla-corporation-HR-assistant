#!/bin/bash

echo "==============================================="
echo "HospAI 智能分诊助手 AI API 测试"
echo "==============================================="

echo ""
echo "🤖 AI API 配置信息："
echo "   API Key: cb103329-5b77-418e-89f2-fea182318c91"
echo "   模型: doubao-lite-32k-character-250228"
echo "   服务商: 豆包 (字节跳动)"
echo ""

echo "📋 测试功能："
echo "   ✅ 真实AI智能分诊"
echo "   ✅ 症状分析与科室推荐"
echo "   ✅ 紧急程度评估"
echo "   ✅ 自然语言理解"
echo "   ✅ 人工服务转接"
echo "   ✅ 网络错误处理"
echo ""

echo "🧪 建议测试场景："
echo "   1. 发热症状: '我发烧38.5度，头痛乏力'"
echo "   2. 胸痛症状: '胸部疼痛，呼吸困难'"
echo "   3. 咳嗽症状: '咳嗽有痰，持续一周了'"
echo "   4. 外伤情况: '摔倒了，手臂疼痛'"
echo "   5. 儿科问题: '小孩拉肚子，没精神'"
echo "   6. 转人工: '我要转人工客服'"
echo ""

echo "⚠️  注意事项："
echo "   • 需要网络连接以访问AI服务"
echo "   • 首次响应可能需要几秒钟"
echo "   • 网络错误时会自动切换到本地逻辑"
echo "   • AI响应会根据症状智能推荐科室"
echo ""

read -p "按任意键启动智能分诊测试... " -n1 -s
echo ""

# 构建目录
BUILD_DIR="./build"

if [ ! -d "$BUILD_DIR" ]; then
    echo "❌ 错误: 构建目录 $BUILD_DIR 不存在"
    echo "请先运行 cmake 和 make 命令"
    exit 1
fi

if [ ! -f "$BUILD_DIR/HospAI" ]; then
    echo "❌ 错误: HospAI 可执行文件不存在"
    echo "请先编译项目"
    exit 1
fi

echo "🚀 启动 HospAI 智能分诊测试..."

# 启动患者端进行AI分诊测试
cd "$BUILD_DIR"
./HospAI --role=patient --user=patient1 --pass=patient123 &
PATIENT_PID=$!

echo ""
echo "✅ 患者端已启动 (PID: $PATIENT_PID)"
echo ""
echo "📝 测试步骤："
echo "1. 在聊天界面中输入症状描述"
echo "2. 观察AI分析过程和响应时间"
echo "3. 检查科室推荐是否准确"
echo "4. 测试紧急情况识别"
echo "5. 验证转人工服务功能"
echo ""
echo "💡 测试提示："
echo "   • 状态栏会显示'正在分析中...'表示AI处理中"
echo "   • AI响应后会显示相应的操作按钮"
echo "   • 可以测试多轮对话的上下文理解"
echo "   • 网络异常时会显示错误信息"
echo ""

# 等待用户中断
trap "echo ''; echo '正在关闭测试进程...'; kill $PATIENT_PID 2>/dev/null; exit 0" INT

echo "🔄 AI API测试进行中... (按 Ctrl+C 退出)"
echo ""
echo "📊 实时监控："
echo "   进程ID: $PATIENT_PID"
echo "   日志输出: 查看终端调试信息"
echo "   API状态: 首次请求时会显示连接状态"
echo ""

wait 