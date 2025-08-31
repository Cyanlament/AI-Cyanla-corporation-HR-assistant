# 青蓝公司HR智能问答系统后端 - 使用jieba分词改进版
from http.server import HTTPServer, BaseHTTPRequestHandler
import json
import urllib.parse
import os
import re
from typing import Dict, List
import threading
import jieba  # 导入jieba分词库
import jieba.analyse  # 导入关键词提取功能

# 知识库路径 - 确保这个路径正确！
KNOWLEDGE_BASE_PATH = r"C:\Users\Lenovo\Desktop\hr-rag-server\青蓝公司知识库"

# 部门需求映射
department_requirements = {
    "控制部": {"勇气": 3, "谨慎": 5, "自律": 5, "正义": 3},
    "情报部": {"勇气": 2, "谨慎": 5, "自律": 4, "正义": 4},
    "培训部": {"勇气": 3, "谨慎": 4, "自律": 4, "正义": 5},
    "安保部": {"勇气": 5, "谨慎": 4, "自律": 4, "正义": 4},
    "中央本部一区": {"勇气": 3, "谨慎": 5, "自律": 5, "正义": 3},
    "中央本部二区": {"勇气": 3, "谨慎": 5, "自律": 5, "正义": 3},
    "福利部": {"勇气": 2, "谨慎": 4, "自律": 4, "正义": 5},
    "惩戒部": {"勇气": 5, "谨慎": 3, "自律": 5, "正义": 4},
    "记录部": {"勇气": 2, "谨慎": 5, "自律": 5, "正义": 3},
    "研发部": {"勇气": 4, "谨慎": 5, "自律": 4, "正义": 3},
    "构筑部": {"勇气": 5, "谨慎": 4, "自律": 5, "正义": 3}
}

# 特质关键词映射
quality_keywords = {
    "勇气": ["勇气", "勇敢", "强壮", "积极", "上进", "外向", "果断", "无畏", "胆量", "冒险", "大胆", "敢闯"],
    "谨慎": ["谨慎", "细心", "周密", "慎重", "稳妥", "内向", "善良", "温和", "耐心", "细致", "小心", "稳妥"],
    "自律": ["自律", "约束", "纪律", "规矩", "坚持", "克制", "守时", "负责", "可靠", "专注", "自制", "恪守"],
    "正义": ["正义", "责任", "热情", "梦想", "公平", "公正", "助人", "奉献", "理想", "信念", "道德", "仁爱"]
}

# 在内存中存储知识库内容
knowledge_base = {}

# 自定义词典 - 添加公司特定词汇
def setup_jieba_dict():
    """设置jieba自定义词典"""
    # 添加公司部门名称
    for dept in department_requirements.keys():
        jieba.add_word(dept, freq=1000, tag='n')
    
    # 添加其他公司特定词汇
    company_words = [
        "青蓝公司", "年假", "愿景", "使命", "价值观", "EGO装备", "脑啡肽", 
        "认知能量", "光之种", "异常收容", "能源提取", "控制部", "情报部",
        "培训部", "安保部", "中央本部", "福利部", "惩戒部", "记录部", "研发部", "构筑部"
    ]
    
    for word in company_words:
        jieba.add_word(word, freq=1000, tag='n')

def load_knowledge_base():
    """加载知识库内容到内存"""
    global knowledge_base
    knowledge_base = {}
    
    print("开始加载知识库...")
    print(f"知识库路径: {KNOWLEDGE_BASE_PATH}")
    
    # 检查路径是否存在
    if not os.path.exists(KNOWLEDGE_BASE_PATH):
        print(f"错误: 知识库路径不存在: {KNOWLEDGE_BASE_PATH}")
        return
    
    file_count = 0
    for root, dirs, files in os.walk(KNOWLEDGE_BASE_PATH):
        for file in files:
            if file.endswith(".txt"):
                file_path = os.path.join(root, file)
                try:
                    with open(file_path, 'r', encoding='utf-8') as f:
                        content = f.read()
                        # 使用相对路径作为键
                        relative_path = os.path.relpath(file_path, KNOWLEDGE_BASE_PATH)
                        knowledge_base[relative_path] = content
                        print(f"已加载: {relative_path}")
                        file_count += 1
                except Exception as e:
                    print(f"加载文件失败 {file_path}: {e}")
    
    print(f"知识库加载完成，共 {file_count} 个文件")

def extract_keywords(text, topK=10):
    """使用jieba提取关键词"""
    keywords = jieba.analyse.extract_tags(text, topK=topK, withWeight=True)
    return keywords

def search_in_knowledge_base(question: str) -> str:
    """在知识库中搜索相关问题答案 - 使用jieba分词改进版"""
    if not knowledge_base:
        return "知识库未加载，请先调用/load接口加载知识库"
    
    # 确保问题是字符串类型
    if not isinstance(question, str):
        try:
            question = str(question, 'utf-8')
        except:
            try:
                question = str(question, 'gbk')
            except:
                return "问题格式不正确"
    
    print(f"搜索问题: {question}")
    
    # 使用jieba提取问题关键词
    try:
        question_keywords = extract_keywords(question, topK=5)
        print(f"提取的关键词: {[kw[0] for kw in question_keywords]}")
    except Exception as e:
        print(f"提取关键词时出错: {e}")
        # 如果分词失败，使用简单方法
        question_keywords = [(word, 1.0) for word in re.findall(r'[\u4e00-\u9fa5]{2,}', question)]
        print(f"使用简单方法提取的关键词: {[kw[0] for kw in question_keywords]}")
    
    # 首先尝试文件名匹配
    for file_path, content in knowledge_base.items():
        file_name = os.path.basename(file_path)
        for keyword, weight in question_keywords:
            if keyword in file_name:
                print(f"文件名匹配: {file_path} - {keyword}")
                # 返回相关内容
                return f"根据「{file_path}」中的信息：{content[:300]}..."
    
    # 然后尝试内容匹配
    results = []
    for file_path, content in knowledge_base.items():
        content_lower = content.lower()
        score = 0
        
        # 计算关键词匹配得分
        for keyword, weight in question_keywords:
            keyword_lower = keyword.lower()
            if keyword_lower in content_lower:
                count = content_lower.count(keyword_lower)
                score += count * weight * len(keyword)  # 考虑权重和关键词长度
        
        if score > 0:
            print(f"文件 '{file_path}' 得分: {score}")
            # 找到包含关键词的段落
            for keyword, weight in question_keywords:
                keyword_lower = keyword.lower()
                if keyword_lower in content_lower:
                    pos = content_lower.find(keyword_lower)
                    if pos != -1:
                        start_pos = max(0, pos - 50)
                        end_pos = min(len(content), pos + 150)
                        snippet = content[start_pos:end_pos]
                        results.append({
                            'file': file_path,
                            'score': score,
                            'snippet': snippet.replace('\n', ' ').replace('\r', ' ')
                        })
                        break
    
    # 按得分排序
    results.sort(key=lambda x: x['score'], reverse=True)
    
    if not results:
        return "未在知识库中找到相关信息。您可以尝试询问关于公司愿景、各部门职责、招聘政策、薪酬福利、假期制度等方面的问题。"
    
    # 返回最相关的结果
    best_result = results[0]
    return f"根据「{best_result['file']}」中的信息：{best_result['snippet']}..."

def analyze_applicant(introduction: str) -> Dict:
    """分析申请人特质并推荐部门"""
    # 计算各特质的得分
    trait_scores = {trait: 0 for trait in quality_keywords.keys()}
    intro_lower = introduction.lower()
    
    for trait, keywords in quality_keywords.items():
        for keyword in keywords:
            # 计算关键词在自我介绍中出现的次数
            count = intro_lower.count(keyword.lower())
            trait_scores[trait] += count
    
    # 计算与各部门的匹配度
    department_scores = {}
    for dept, requirements in department_requirements.items():
        score = 0
        for trait, weight in requirements.items():
            score += trait_scores[trait] * weight
        department_scores[dept] = score
    
    # 找出最匹配的部门
    if not department_scores:
        best_dept = "控制部"
        max_score = 0
    else:
        best_dept = max(department_scores.items(), key=lambda x: x[1])[0]
        max_score = department_scores[best_dept]
    
    # 确定合适程度
    if max_score < 5:
        fitness_level = "low"
    elif max_score < 15:
        fitness_level = "medium"
    elif max_score < 25:
        fitness_level = "high"
    else:
        fitness_level = "critical"
    
    # 生成品质分析
    if trait_scores:
        sorted_traits = sorted(trait_scores.items(), key=lambda x: x[1], reverse=True)
        dominant_traits = [trait for trait, score in sorted_traits if score > 0]
        
        if len(dominant_traits) >= 2:
            quality_analysis = f"您的核心特质是{dominant_traits[0]}和{dominant_traits[1]}"
        elif dominant_traits:
            quality_analysis = f"您的核心特质是{dominant_traits[0]}"
        else:
            quality_analysis = "您的特质表现不够明显"
    else:
        quality_analysis = "无法分析您的特质"
    
    # 生成可能原因和建议
    possible_causes = []
    suggestions = []
    
    for trait, score in trait_scores.items():
        if score > 2:
            possible_causes.append(f"您在自我介绍中体现了较强的{trait}特质")
    
    # 添加建议
    suggestions.append(f"推荐您进一步了解{best_dept}的职责要求")
    suggestions.append("建议准备相关面试材料，突出您的优势特质")
    
    if fitness_level == "critical":
        suggestions.append("您与推荐部门匹配度极高，请务必申请面试！")
    elif fitness_level == "low":
        suggestions.append("建议您重新考虑职业方向，或咨询HR获取更多指导")
    
    # 判断是否需要人工咨询
    needs_human_consult = fitness_level in ["low", "critical"]
    
    return {
        "qualityAnalysis": quality_analysis,
        "fitnessLevel": fitness_level,
        "recommendedDepartment": best_dept,
        "possibleCauses": possible_causes,
        "suggestions": suggestions,
        "needsHumanConsult": needs_human_consult
    }

class RequestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/':
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            response = {"message": "青蓝公司HR智能问答系统API已就绪", "status": "success"}
            self.wfile.write(json.dumps(response, ensure_ascii=False).encode('utf-8'))
        
        elif self.path == '/load':
            load_knowledge_base()
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            response = {"message": f"知识库加载成功，共{len(knowledge_base)}个文件", "status": "success"}
            self.wfile.write(json.dumps(response).encode('utf-8'))
        
        else:
            self.send_response(404)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            response = {"error": "接口不存在", "status": "error"}
            self.wfile.write(json.dumps(response, ensure_ascii=False).encode('utf-8'))
    
    def do_POST(self):
        try:
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            
            # 尝试多种编码方式解码
            try:
                # 首先尝试UTF-8
                data_str = post_data.decode('utf-8')
            except UnicodeDecodeError:
                try:
                    # 如果UTF-8失败，尝试GBK（中文Windows默认编码）
                    data_str = post_data.decode('gbk')
                except UnicodeDecodeError:
                    # 如果都失败，返回错误
                    self.send_response(400)
                    self.send_header('Content-type', 'application/json')
                    self.send_header('Access-Control-Allow-Origin', '*')
                    self.end_headers()
                    response = {"error": "无法解码请求数据", "status": "error"}
                    self.wfile.write(json.dumps(response, ensure_ascii=False).encode('utf-8'))
                    return
            
            try:
                data = json.loads(data_str)
            except json.JSONDecodeError:
                self.send_response(400)
                self.send_header('Content-type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                response = {"error": "无效的JSON格式", "status": "error"}
                self.wfile.write(json.dumps(response, ensure_ascii=False).encode('utf-8'))
                return
            
            self.send_response(200)
            self.send_header('Content-type', 'application/json; charset=utf-8')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            
            if self.path == '/ask':
                question = data.get('question', '')
                if not question:
                    response = {"error": "问题不能为空", "status": "error"}
                else:
                    answer = search_in_knowledge_base(question)
                    response = {"answer": answer, "status": "success"}
            
            elif self.path == '/analyze_applicant':
                introduction = data.get('introduction', '')
                if not introduction:
                    response = {"error": "自我介绍不能为空", "status": "error"}
                else:
                    analysis = analyze_applicant(introduction)
                    response = {**analysis, "status": "success"}
            
            else:
                response = {"error": "接口不存在", "status": "error"}
            
            self.wfile.write(json.dumps(response, ensure_ascii=False).encode('utf-8'))
        
        except Exception as e:
            print(f"处理POST请求时出错: {e}")
            self.send_response(500)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            response = {"error": "服务器内部错误", "status": "error"}
            self.wfile.write(json.dumps(response, ensure_ascii=False).encode('utf-8'))
    
    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()

def run_server():
    """启动HTTP服务器"""
    # 设置jieba自定义词典
    setup_jieba_dict()
    
    server_address = ('0.0.0.0', 8000)
    httpd = HTTPServer(server_address, RequestHandler)
    print(f'服务器已启动，访问 http://localhost:8000')
    print('可用接口:')
    print('  GET  /        - 服务器状态')
    print('  GET  /load    - 加载知识库')
    print('  POST /ask     - 提问 {"question": "问题内容"}')
    print('  POST /analyze_applicant - 分析申请人 {"introduction": "自我介绍"}')
    httpd.serve_forever()

if __name__ == '__main__':
    # 启动时自动加载知识库
    load_knowledge_base()
    run_server()