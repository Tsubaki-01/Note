# 🦜 Python LangChain & LangGraph 完整参考指南

> **适用版本：** LangChain >= 0.2.x · LangGraph >= 0.1.x  
> **语言：** Python 3.9+  
> **最后更新：** 2025

---

## 📚 目录

1. [概述与安装](#1-概述与安装)
2. [LangChain 核心概念](#2-langchain-核心概念)
   - [模型（Models）](#21-模型models)
   - [提示词（Prompts）](#22-提示词prompts)
   - [输出解析（Output Parsers）](#23-输出解析output-parsers)
   - [链（Chains / LCEL）](#24-链chainslcel)
   - [记忆（Memory）](#25-记忆memory)
   - [文档加载与处理](#26-文档加载与处理)
   - [向量存储与检索](#27-向量存储与检索)
   - [工具与代理（Tools & Agents）](#28-工具与代理tools--agents)
   - [回调（Callbacks）](#29-回调callbacks)
3. [LangGraph 核心概念](#3-langgraph-核心概念)
   - [基本图结构](#31-基本图结构)
   - [状态管理（State）](#32-状态管理state)
   - [节点（Nodes）](#33-节点nodes)
   - [边（Edges）](#34-边edges)
   - [条件路由](#35-条件路由)
   - [检查点（Checkpointing）](#36-检查点checkpointing)
   - [人机协作（Human-in-the-Loop）](#37-人机协作human-in-the-loop)
   - [子图（Subgraphs）](#38-子图subgraphs)
   - [并行执行](#39-并行执行)
   - [流式输出（Streaming）](#310-流式输出streaming)
4. [常见应用模式](#4-常见应用模式)
   - [RAG 检索增强生成](#41-rag-检索增强生成)
   - [ReAct 代理](#42-react-代理)
   - [多代理系统](#43-多代理系统)
   - [对话式 RAG](#44-对话式-rag)
5. [最佳实践与调试](#5-最佳实践与调试)
6. [常见错误与解决方案](#6-常见错误与解决方案)

---

## 1. 概述与安装

### 什么是 LangChain？

**LangChain** 是一个用于构建 LLM 应用的框架，提供：
- 统一的模型接口（OpenAI、Anthropic、Google 等）
- 提示词工程工具
- 链式调用（LCEL — LangChain Expression Language）
- 文档加载、分割、向量化、检索
- 内置工具与代理能力

### 什么是 LangGraph？

**LangGraph** 是 LangChain 团队开发的图计算框架，专为构建 **有状态的多步 AI 工作流** 而设计：
- 基于有向图（DAG/Cyclic Graph）定义复杂控制流
- 原生支持循环、条件分支、并行执行
- 内置 Checkpointing（断点续跑）
- 支持 Human-in-the-Loop 人机协作

### 安装

```bash
# 基础安装
pip install langchain langchain-core langchain-community

# LangGraph
pip install langgraph

# 按需安装模型提供商
pip install langchain-openai        # OpenAI / Azure
pip install langchain-anthropic     # Anthropic Claude
pip install langchain-google-genai  # Google Gemini

# 向量数据库（按需选择）
pip install faiss-cpu               # FAISS（本地）
pip install chromadb                # ChromaDB
pip install pinecone-client         # Pinecone

# 工具依赖
pip install tiktoken                # Token 计数
pip install unstructured            # 文档解析
pip install langchain-text-splitters # 文本分割
```

### 环境变量配置

```python
import os
from dotenv import load_dotenv

load_dotenv()  # 从 .env 文件加载

# OpenAI
os.environ["OPENAI_API_KEY"] = "sk-..."

# Anthropic
os.environ["ANTHROPIC_API_KEY"] = "sk-ant-..."

# LangSmith 追踪（可选但强烈推荐）
os.environ["LANGCHAIN_TRACING_V2"] = "true"
os.environ["LANGCHAIN_API_KEY"] = "ls__..."
os.environ["LANGCHAIN_PROJECT"] = "my-project"
```

---

## 2. LangChain 核心概念

### 2.1 模型（Models）

LangChain 将模型分为两大类：**LLM**（文本输入输出）和 **ChatModel**（消息列表输入输出）。

#### ChatModel 基础用法

```python
from langchain_openai import ChatOpenAI
from langchain_anthropic import ChatAnthropic
from langchain_google_genai import ChatGoogleGenerativeAI

# OpenAI
llm = ChatOpenAI(
    model="gpt-4o",
    temperature=0.7,       # 0.0 ~ 2.0，控制随机性
    max_tokens=1024,       # 最大输出 token 数
    timeout=30,            # 请求超时（秒）
    max_retries=2,         # 自动重试次数
)

# Anthropic Claude
llm = ChatAnthropic(
    model="claude-3-5-sonnet-20241022",
    temperature=0,
    max_tokens=4096,
)

# Google Gemini
llm = ChatGoogleGenerativeAI(
    model="gemini-1.5-pro",
    temperature=0,
)

# 基础调用
from langchain_core.messages import HumanMessage, SystemMessage

response = llm.invoke([
    SystemMessage(content="你是一个专业的 Python 开发者。"),
    HumanMessage(content="解释什么是装饰器？"),
])
print(response.content)        # 文本内容
print(response.usage_metadata) # token 使用量
```

#### 消息类型

```python
from langchain_core.messages import (
    SystemMessage,    # 系统提示
    HumanMessage,     # 用户消息
    AIMessage,        # 模型回复
    ToolMessage,      # 工具执行结果
    FunctionMessage,  # 函数调用结果（旧版）
)

# 消息历史示例
messages = [
    SystemMessage(content="你是一个助手。"),
    HumanMessage(content="你好！"),
    AIMessage(content="你好！有什么可以帮你的？"),
    HumanMessage(content="今天天气如何？"),
]
```

#### 流式输出

```python
# 同步流式
for chunk in llm.stream("给我讲一个故事"):
    print(chunk.content, end="", flush=True)

# 异步流式
import asyncio

async def stream_async():
    async for chunk in llm.astream("给我讲一个故事"):
        print(chunk.content, end="", flush=True)

asyncio.run(stream_async())
```

#### 批量调用

```python
# 批量处理多个请求（并发执行）
responses = llm.batch([
    "什么是机器学习？",
    "什么是深度学习？",
    "什么是强化学习？",
])

# 异步批量
responses = await llm.abatch([...])
```

#### 模型绑定工具

```python
from langchain_core.tools import tool

@tool
def get_weather(city: str) -> str:
    """获取指定城市的天气信息。"""
    return f"{city}今天晴天，25°C"

# 绑定工具到模型
llm_with_tools = llm.bind_tools([get_weather])

response = llm_with_tools.invoke("北京今天天气怎么样？")
print(response.tool_calls)  # 查看工具调用
```

---

### 2.2 提示词（Prompts）

#### ChatPromptTemplate

```python
from langchain_core.prompts import (
    ChatPromptTemplate,
    SystemMessagePromptTemplate,
    HumanMessagePromptTemplate,
    MessagesPlaceholder,
)

# 基础模板
prompt = ChatPromptTemplate.from_messages([
    ("system", "你是一个专业的{role}，请用{language}回答。"),
    ("human", "{question}"),
])

# 格式化
formatted = prompt.invoke({
    "role": "数据科学家",
    "language": "中文",
    "question": "解释线性回归",
})

# 包含历史消息的模板
prompt_with_history = ChatPromptTemplate.from_messages([
    ("system", "你是一个助手。"),
    MessagesPlaceholder("chat_history"),  # 动态插入历史
    ("human", "{input}"),
])

# 使用
chain = prompt_with_history | llm
response = chain.invoke({
    "chat_history": [
        HumanMessage(content="我叫张三"),
        AIMessage(content="你好，张三！"),
    ],
    "input": "我叫什么名字？",
})
```

#### PromptTemplate（纯文本）

```python
from langchain_core.prompts import PromptTemplate

template = PromptTemplate(
    input_variables=["topic", "style"],
    template="用{style}的方式解释{topic}，不超过100字。",
)

# 或使用 from_template 快捷方式
template = PromptTemplate.from_template(
    "用{style}的方式解释{topic}，不超过100字。"
)

formatted = template.format(topic="量子计算", style="通俗易懂")
```

#### FewShotPromptTemplate（少样本）

```python
from langchain_core.prompts import FewShotChatMessagePromptTemplate

examples = [
    {"input": "苹果", "output": "水果，红色或绿色，甜的"},
    {"input": "汽车", "output": "交通工具，有四个轮子，用于出行"},
]

example_prompt = ChatPromptTemplate.from_messages([
    ("human", "{input}"),
    ("ai", "{output}"),
])

few_shot_prompt = FewShotChatMessagePromptTemplate(
    example_prompt=example_prompt,
    examples=examples,
)

final_prompt = ChatPromptTemplate.from_messages([
    ("system", "请简单描述以下事物："),
    few_shot_prompt,
    ("human", "{input}"),
])
```

---

### 2.3 输出解析（Output Parsers）

#### StrOutputParser（字符串输出）

```python
from langchain_core.output_parsers import StrOutputParser

chain = prompt | llm | StrOutputParser()
result = chain.invoke({"question": "什么是 Python？"})
print(result)  # 纯字符串
```

#### JsonOutputParser（JSON 输出）

```python
from langchain_core.output_parsers import JsonOutputParser
from pydantic import BaseModel, Field
from typing import List

class BookReview(BaseModel):
    title: str = Field(description="书名")
    rating: float = Field(description="评分，1-10分")
    summary: str = Field(description="简短评价")
    tags: List[str] = Field(description="标签列表")

parser = JsonOutputParser(pydantic_object=BookReview)

prompt = ChatPromptTemplate.from_messages([
    ("system", "你是一个书评家。\n{format_instructions}"),
    ("human", "评价《{book}》"),
]).partial(format_instructions=parser.get_format_instructions())

chain = prompt | llm | parser
result = chain.invoke({"book": "三体"})
print(result)  # BookReview 对象或字典
```

#### PydanticOutputParser（结构化输出）

```python
from langchain_core.output_parsers import PydanticOutputParser
from pydantic import BaseModel, Field, validator

class Person(BaseModel):
    name: str = Field(description="姓名")
    age: int = Field(description="年龄", ge=0, le=150)
    skills: List[str] = Field(description="技能列表")

parser = PydanticOutputParser(pydantic_object=Person)

prompt = PromptTemplate(
    template="提取以下文本中的人物信息：\n{text}\n\n{format_instructions}",
    input_variables=["text"],
    partial_variables={"format_instructions": parser.get_format_instructions()},
)

chain = prompt | llm | parser
result = chain.invoke({"text": "张三，28岁，擅长 Python 和机器学习"})
print(result.name)    # "张三"
print(result.age)     # 28
print(result.skills)  # ["Python", "机器学习"]
```

#### 使用 `.with_structured_output()`（推荐方式）

```python
# 更简洁的结构化输出（直接绑定到模型）
structured_llm = llm.with_structured_output(Person)
result = structured_llm.invoke("张三，28岁，擅长 Python")
print(result)  # Person 对象

# 支持 dict schema
schema = {
    "title": "Person",
    "properties": {
        "name": {"type": "string"},
        "age": {"type": "integer"},
    },
    "required": ["name", "age"],
}
structured_llm = llm.with_structured_output(schema)
```

---

### 2.4 链（Chains / LCEL）

**LCEL（LangChain Expression Language）** 使用 `|` 操作符将组件串联起来。

#### 基础链

```python
from langchain_core.runnables import RunnablePassthrough, RunnableLambda

# 最简单的链
chain = prompt | llm | StrOutputParser()
result = chain.invoke({"topic": "Python", "style": "幽默"})

# RunnablePassthrough：透传输入
chain = (
    RunnablePassthrough()
    | prompt
    | llm
    | StrOutputParser()
)

# RunnableLambda：包装任意函数
def preprocess(text: str) -> dict:
    return {"question": text.strip().lower()}

chain = RunnableLambda(preprocess) | prompt | llm | StrOutputParser()
result = chain.invoke("  什么是 Docker？  ")
```

#### RunnableParallel（并行执行）

```python
from langchain_core.runnables import RunnableParallel

# 并行运行多个分支
parallel_chain = RunnableParallel(
    summary=prompt_summary | llm | StrOutputParser(),
    keywords=prompt_keywords | llm | StrOutputParser(),
    sentiment=prompt_sentiment | llm | StrOutputParser(),
)

result = parallel_chain.invoke({"text": "这篇文章非常精彩..."})
print(result["summary"])   # 摘要
print(result["keywords"])  # 关键词
print(result["sentiment"]) # 情感分析
```

#### RunnableBranch（条件分支）

```python
from langchain_core.runnables import RunnableBranch

branch = RunnableBranch(
    (lambda x: x["language"] == "python", python_chain),
    (lambda x: x["language"] == "javascript", js_chain),
    default_chain,  # 默认分支
)

result = branch.invoke({"language": "python", "code": "..."})
```

#### 链的配置与重试

```python
# 运行时配置
chain = prompt | llm | StrOutputParser()

result = chain.invoke(
    {"question": "你好"},
    config={
        "run_name": "my_chain",          # 追踪名称
        "tags": ["test", "dev"],          # 标签
        "metadata": {"user_id": "123"},   # 元数据
        "max_concurrency": 5,             # 最大并发
    }
)

# 自动重试
from langchain_core.runnables import with_retry

robust_llm = with_retry(
    llm,
    stop_after_attempt=3,
    wait_exponential_jitter=True,
)
```

#### 链的检查与调试

```python
# 查看链的输入/输出 schema
print(chain.input_schema.schema())
print(chain.output_schema.schema())

# 中间步骤追踪
result = chain.invoke({"question": "..."}, return_only_outputs=False)

# 使用 .get_graph() 可视化
graph = chain.get_graph()
graph.print_ascii()
```

---

### 2.5 记忆（Memory）

LangChain v0.2+ 推荐使用 **ChatMessageHistory** + **RunnableWithMessageHistory**。

#### ChatMessageHistory

```python
from langchain_core.chat_history import BaseChatMessageHistory
from langchain_community.chat_message_histories import (
    ChatMessageHistory,        # 内存存储
    RedisChatMessageHistory,   # Redis 存储
    SQLChatMessageHistory,     # SQL 存储
)

# 内存历史
history = ChatMessageHistory()
history.add_user_message("你好")
history.add_ai_message("你好！有什么可以帮你的？")
history.add_user_message("我叫张三")

print(history.messages)  # 所有消息
history.clear()          # 清空
```

#### RunnableWithMessageHistory

```python
from langchain_core.runnables.history import RunnableWithMessageHistory

# 定义获取历史记录的函数
store = {}  # 实际使用时换成数据库

def get_session_history(session_id: str) -> BaseChatMessageHistory:
    if session_id not in store:
        store[session_id] = ChatMessageHistory()
    return store[session_id]

# 构建带记忆的链
prompt = ChatPromptTemplate.from_messages([
    ("system", "你是一个助手。"),
    MessagesPlaceholder("history"),
    ("human", "{input}"),
])

chain = prompt | llm | StrOutputParser()

chain_with_history = RunnableWithMessageHistory(
    chain,
    get_session_history,
    input_messages_key="input",
    history_messages_key="history",
)

# 使用 session_id 区分不同对话
config = {"configurable": {"session_id": "user_123"}}

response1 = chain_with_history.invoke({"input": "我叫张三"}, config=config)
response2 = chain_with_history.invoke({"input": "我叫什么？"}, config=config)
print(response2)  # "你叫张三"
```

#### 限制历史长度

```python
from langchain_core.messages import trim_messages

# 裁剪消息历史（按 token 数限制）
trimmer = trim_messages(
    max_tokens=2000,
    strategy="last",           # 保留最新的
    token_counter=llm,         # 使用模型计算 token
    include_system=True,       # 保留 system 消息
    allow_partial=False,
    start_on="human",          # 从 human 消息开始
)

trimmed = trimmer.invoke(messages)
```

---

### 2.6 文档加载与处理

#### 文档加载器

```python
from langchain_community.document_loaders import (
    TextLoader,           # 纯文本
    PyPDFLoader,          # PDF
    Docx2txtLoader,       # Word 文档
    CSVLoader,            # CSV
    JSONLoader,           # JSON
    WebBaseLoader,        # 网页
    DirectoryLoader,      # 目录批量加载
    YoutubeLoader,        # YouTube 字幕
    GitLoader,            # Git 仓库
)

# 加载 PDF
loader = PyPDFLoader("document.pdf")
docs = loader.load()
print(docs[0].page_content)   # 文本内容
print(docs[0].metadata)       # {"source": "document.pdf", "page": 0}

# 加载网页
loader = WebBaseLoader(
    web_paths=["https://example.com"],
    bs_kwargs={"parse_only": SoupStrainer("article")},  # 选择器过滤
)
docs = loader.load()

# 批量加载目录
loader = DirectoryLoader(
    "./docs/",
    glob="**/*.md",        # 文件模式
    loader_cls=TextLoader,
    show_progress=True,
)
docs = loader.load()

# JSON 加载（指定 jq 路径）
loader = JSONLoader(
    file_path="data.json",
    jq_schema=".messages[].content",
    text_content=True,
)
```

#### 文本分割器

```python
from langchain_text_splitters import (
    RecursiveCharacterTextSplitter,  # 推荐，递归分割
    CharacterTextSplitter,           # 按单个字符分割
    TokenTextSplitter,               # 按 token 分割
    MarkdownHeaderTextSplitter,      # 按 Markdown 标题分割
    HTMLHeaderTextSplitter,          # 按 HTML 标题分割
    PythonCodeTextSplitter,          # Python 代码感知分割
)

# RecursiveCharacterTextSplitter（最常用）
splitter = RecursiveCharacterTextSplitter(
    chunk_size=1000,        # 每块最大字符数
    chunk_overlap=200,      # 块间重叠字符数
    length_function=len,    # 长度计算函数
    separators=["\n\n", "\n", "。", "！", "？", " ", ""],
)

chunks = splitter.split_documents(docs)
print(f"分割为 {len(chunks)} 块")

# 按 token 分割（更精确）
token_splitter = TokenTextSplitter(
    chunk_size=500,
    chunk_overlap=50,
    encoding_name="cl100k_base",  # GPT-4 的编码
)

# Markdown 感知分割（保留结构）
md_splitter = MarkdownHeaderTextSplitter(
    headers_to_split_on=[
        ("#", "H1"),
        ("##", "H2"),
        ("###", "H3"),
    ],
    strip_headers=False,
)
md_docs = md_splitter.split_text(markdown_text)
```

---

### 2.7 向量存储与检索

#### Embeddings（嵌入模型）

```python
from langchain_openai import OpenAIEmbeddings
from langchain_community.embeddings import HuggingFaceEmbeddings

# OpenAI Embeddings
embeddings = OpenAIEmbeddings(model="text-embedding-3-small")

# 本地 HuggingFace 模型（无需 API Key）
embeddings = HuggingFaceEmbeddings(
    model_name="BAAI/bge-m3",       # 支持中文
    model_kwargs={"device": "cpu"},
    encode_kwargs={"normalize_embeddings": True},
)

# 生成向量
vector = embeddings.embed_query("你好，世界")
vectors = embeddings.embed_documents(["文档1", "文档2"])
```

#### FAISS 向量存储

```python
from langchain_community.vectorstores import FAISS

# 从文档创建
vectorstore = FAISS.from_documents(
    documents=chunks,
    embedding=embeddings,
)

# 保存与加载
vectorstore.save_local("./faiss_index")
vectorstore = FAISS.load_local(
    "./faiss_index",
    embeddings,
    allow_dangerous_deserialization=True,
)

# 相似度搜索
results = vectorstore.similarity_search(
    query="Python 异步编程",
    k=4,                   # 返回 Top-K
)

# 带分数的搜索
results_with_scores = vectorstore.similarity_search_with_score(
    "Python 异步编程", k=4
)
for doc, score in results_with_scores:
    print(f"Score: {score:.4f} | {doc.page_content[:100]}")

# 添加新文档
vectorstore.add_documents(new_chunks)
```

#### ChromaDB 向量存储

```python
from langchain_community.vectorstores import Chroma

# 创建持久化存储
vectorstore = Chroma.from_documents(
    documents=chunks,
    embedding=embeddings,
    persist_directory="./chroma_db",
    collection_name="my_docs",
)

# 带过滤器的搜索
results = vectorstore.similarity_search(
    "Python 教程",
    k=5,
    filter={"source": "tutorial.pdf"},  # 元数据过滤
)
```

#### 检索器（Retrievers）

```python
# 基础检索器
retriever = vectorstore.as_retriever(
    search_type="similarity",        # similarity / mmr / similarity_score_threshold
    search_kwargs={
        "k": 5,
        "score_threshold": 0.7,      # 仅用于 similarity_score_threshold
        "fetch_k": 20,               # 仅用于 mmr
        "lambda_mult": 0.5,          # mmr 多样性参数
    },
)

# MMR（最大边际相关性）—— 减少重复
retriever = vectorstore.as_retriever(
    search_type="mmr",
    search_kwargs={"k": 5, "fetch_k": 20},
)

# MultiQueryRetriever（自动生成多个查询）
from langchain.retrievers import MultiQueryRetriever

multi_retriever = MultiQueryRetriever.from_llm(
    retriever=base_retriever,
    llm=llm,
    # 自动生成 3 个不同角度的查询
)

# ContextualCompressionRetriever（压缩相关内容）
from langchain.retrievers import ContextualCompressionRetriever
from langchain.retrievers.document_compressors import LLMChainExtractor

compressor = LLMChainExtractor.from_llm(llm)
compression_retriever = ContextualCompressionRetriever(
    base_compressor=compressor,
    base_retriever=base_retriever,
)

# EnsembleRetriever（混合检索）
from langchain.retrievers import EnsembleRetriever
from langchain_community.retrievers import BM25Retriever

bm25_retriever = BM25Retriever.from_documents(chunks)
bm25_retriever.k = 4

ensemble_retriever = EnsembleRetriever(
    retrievers=[bm25_retriever, vectorstore_retriever],
    weights=[0.4, 0.6],  # 权重
)
```

---

### 2.8 工具与代理（Tools & Agents）

#### 定义工具

```python
from langchain_core.tools import tool, StructuredTool
from pydantic import BaseModel, Field
from typing import Optional

# 使用 @tool 装饰器
@tool
def search_web(query: str) -> str:
    """在网络上搜索信息。当需要实时信息时使用此工具。"""
    # 实际实现...
    return f"搜索结果：{query}"

@tool
def calculate(expression: str) -> str:
    """计算数学表达式。例如：'2 + 2'，'sqrt(16)'"""
    import math
    try:
        result = eval(expression, {"__builtins__": {}}, vars(math))
        return str(result)
    except Exception as e:
        return f"计算错误：{e}"

# 带复杂参数的工具
class SearchInput(BaseModel):
    query: str = Field(description="搜索关键词")
    max_results: int = Field(default=5, description="返回结果数量")
    date_filter: Optional[str] = Field(default=None, description="日期过滤")

@tool(args_schema=SearchInput)
def advanced_search(query: str, max_results: int = 5, date_filter: Optional[str] = None) -> str:
    """执行高级网络搜索，支持过滤条件。"""
    return f"搜索 '{query}'，返回 {max_results} 条结果"

# StructuredTool（函数式创建）
def get_stock_price(ticker: str, currency: str = "USD") -> str:
    """获取股票价格"""
    return f"{ticker}: 150.00 {currency}"

stock_tool = StructuredTool.from_function(
    func=get_stock_price,
    name="get_stock_price",
    description="获取指定股票代码的当前价格",
)

# 内置工具
from langchain_community.tools import WikipediaQueryRun, DuckDuckGoSearchRun
from langchain_community.utilities import WikipediaAPIWrapper

wiki = WikipediaQueryRun(api_wrapper=WikipediaAPIWrapper())
search = DuckDuckGoSearchRun()
```

#### 代理（Agents）

```python
from langchain.agents import create_react_agent, AgentExecutor
from langchain import hub

tools = [search_web, calculate, wiki]

# 使用 LangChain Hub 中的提示词
prompt = hub.pull("hwchase17/react")

# 创建 ReAct 代理
agent = create_react_agent(llm, tools, prompt)

# 创建执行器
agent_executor = AgentExecutor(
    agent=agent,
    tools=tools,
    verbose=True,           # 打印推理过程
    max_iterations=10,      # 最大循环次数
    handle_parsing_errors=True,  # 自动处理解析错误
    return_intermediate_steps=True,  # 返回中间步骤
)

result = agent_executor.invoke({"input": "计算 sqrt(144) 然后告诉我这个数字的 Wikipedia 介绍"})
print(result["output"])
print(result["intermediate_steps"])  # 每步的动作和观察
```

#### Tool Calling 代理（推荐）

```python
from langchain.agents import create_tool_calling_agent

# 更现代的方式（使用模型原生 function calling）
prompt = ChatPromptTemplate.from_messages([
    ("system", "你是一个有用的助手，可以使用工具来回答问题。"),
    ("human", "{input}"),
    MessagesPlaceholder("agent_scratchpad"),
])

agent = create_tool_calling_agent(llm, tools, prompt)
agent_executor = AgentExecutor(agent=agent, tools=tools, verbose=True)

result = agent_executor.invoke({"input": "北京现在的天气如何？"})
```

---

### 2.9 回调（Callbacks）

```python
from langchain_core.callbacks import BaseCallbackHandler, StdOutCallbackHandler
from langchain_core.outputs import LLMResult

# 自定义回调处理器
class MyCallbackHandler(BaseCallbackHandler):
    
    def on_llm_start(self, serialized, prompts, **kwargs):
        print(f"🚀 LLM 开始调用，提示词数量：{len(prompts)}")
    
    def on_llm_end(self, response: LLMResult, **kwargs):
        print(f"✅ LLM 调用结束，生成 {len(response.generations)} 个结果")
    
    def on_llm_error(self, error, **kwargs):
        print(f"❌ LLM 出错：{error}")
    
    def on_chain_start(self, serialized, inputs, **kwargs):
        print(f"⛓️  链开始：{serialized.get('name', 'Unknown')}")
    
    def on_chain_end(self, outputs, **kwargs):
        print(f"⛓️  链结束：{outputs}")
    
    def on_tool_start(self, serialized, input_str, **kwargs):
        print(f"🔧 工具调用：{serialized['name']}({input_str})")
    
    def on_tool_end(self, output, **kwargs):
        print(f"🔧 工具结果：{output}")

# 使用回调
handler = MyCallbackHandler()

result = chain.invoke(
    {"question": "你好"},
    config={"callbacks": [handler]},
)

# 全局配置（影响所有调用）
from langchain_core.globals import set_verbose, set_debug

set_verbose(True)   # 打印链的输入输出
set_debug(True)     # 更详细的调试信息
```

---

## 3. LangGraph 核心概念

### 3.1 基本图结构

```python
from langgraph.graph import StateGraph, START, END
from typing import TypedDict, Annotated
import operator

# 1. 定义状态
class State(TypedDict):
    messages: list
    count: int

# 2. 定义节点函数
def node_a(state: State) -> State:
    return {"count": state["count"] + 1}

def node_b(state: State) -> State:
    return {"count": state["count"] * 2}

# 3. 构建图
graph_builder = StateGraph(State)

# 添加节点
graph_builder.add_node("node_a", node_a)
graph_builder.add_node("node_b", node_b)

# 添加边
graph_builder.add_edge(START, "node_a")
graph_builder.add_edge("node_a", "node_b")
graph_builder.add_edge("node_b", END)

# 4. 编译图
graph = graph_builder.compile()

# 5. 运行
result = graph.invoke({"messages": [], "count": 0})
print(result)  # {"messages": [], "count": 2}

# 可视化（需要安装 pygraphviz）
from IPython.display import Image
Image(graph.get_graph().draw_mermaid_png())
```

---

### 3.2 状态管理（State）

#### 使用 TypedDict

```python
from typing import TypedDict, Annotated, List
import operator

# 普通字段（节点返回值直接覆盖）
class SimpleState(TypedDict):
    name: str
    age: int
    result: str

# 使用 Annotated 自定义更新逻辑
class AdvancedState(TypedDict):
    # operator.add：新值追加到列表（用于消息历史）
    messages: Annotated[list, operator.add]
    
    # 自定义 reducer 函数
    scores: Annotated[list, lambda old, new: sorted(old + new)]
    
    # 普通覆盖字段
    current_step: str
    error: str
```

#### 使用 Pydantic（带验证）

```python
from pydantic import BaseModel, Field
from langgraph.graph import StateGraph

class AgentState(BaseModel):
    messages: Annotated[list, operator.add] = Field(default_factory=list)
    task: str = ""
    plan: List[str] = Field(default_factory=list)
    result: str = ""
    iteration: int = 0
    max_iterations: int = 10

graph = StateGraph(AgentState)
```

#### MessagesState（内置消息状态）

```python
from langgraph.graph.message import MessagesState, add_messages
from langchain_core.messages import AnyMessage

# 内置的消息状态（推荐用于聊天机器人）
# 等同于：messages: Annotated[list[AnyMessage], add_messages]
class MyState(MessagesState):
    # 可以扩展额外字段
    summary: str = ""
    user_id: str = ""

# add_messages 的特殊行为：
# 1. 自动去重（相同 id 的消息会被替换而非追加）
# 2. 支持删除（返回 RemoveMessage）
from langchain_core.messages import RemoveMessage
def delete_message(state):
    return {"messages": [RemoveMessage(id=state["messages"][0].id)]}
```

---

### 3.3 节点（Nodes）

```python
from langchain_core.messages import HumanMessage, AIMessage
from langgraph.graph import StateGraph, MessagesState, START, END

llm = ChatOpenAI(model="gpt-4o")

# 普通节点
def chatbot_node(state: MessagesState) -> MessagesState:
    response = llm.invoke(state["messages"])
    return {"messages": [response]}

# 工具节点（LangGraph 内置）
from langgraph.prebuilt import ToolNode

tools = [search_web, calculate]
tool_node = ToolNode(tools)  # 自动处理工具调用

# 带错误处理的节点
def safe_node(state: MessagesState):
    try:
        response = llm.invoke(state["messages"])
        return {"messages": [response]}
    except Exception as e:
        error_msg = AIMessage(content=f"处理出错：{str(e)}")
        return {"messages": [error_msg]}

# 异步节点
async def async_node(state: MessagesState):
    response = await llm.ainvoke(state["messages"])
    return {"messages": [response]}

# 构建图
builder = StateGraph(MessagesState)
builder.add_node("chatbot", chatbot_node)
builder.add_node("tools", tool_node)
builder.add_edge(START, "chatbot")
```

---

### 3.4 边（Edges）

```python
# 普通边（固定连接）
builder.add_edge("node_a", "node_b")
builder.add_edge("node_b", END)

# 从 START 开始
builder.add_edge(START, "first_node")

# 多个后继节点（同时触发，并行执行）
builder.add_edge("node_a", "node_b")
builder.add_edge("node_a", "node_c")  # a 完成后同时触发 b 和 c

# 等待多个前驱完成（自动处理）
builder.add_edge("node_b", "node_d")
builder.add_edge("node_c", "node_d")  # d 在 b 和 c 都完成后执行
```

---

### 3.5 条件路由

```python
# 条件边：根据状态决定下一步
def route_tools(state: MessagesState) -> str:
    """根据最新消息决定是否调用工具"""
    messages = state["messages"]
    last_message = messages[-1]
    
    # 如果有工具调用请求
    if hasattr(last_message, "tool_calls") and last_message.tool_calls:
        return "tools"
    
    # 否则结束
    return END

builder.add_conditional_edges(
    "chatbot",           # 源节点
    route_tools,         # 路由函数
    # 可选：路由映射（当函数返回值不是节点名时使用）
    {
        "tools": "tools",
        END: END,
    }
)

# 更复杂的路由
def complex_router(state: dict) -> str:
    if state["error"]:
        return "error_handler"
    elif state["iteration"] >= state["max_iterations"]:
        return "summarize"
    elif state["task_complete"]:
        return END
    else:
        return "continue"

builder.add_conditional_edges(
    "agent",
    complex_router,
    ["error_handler", "summarize", "continue", END],  # 可能的目标节点列表
)
```

---

### 3.6 检查点（Checkpointing）

检查点允许图的状态在每个节点执行后持久化，支持：
- 断点续跑
- 时间旅行（回溯到历史状态）
- Human-in-the-Loop

```python
from langgraph.checkpoint.memory import MemorySaver
from langgraph.checkpoint.sqlite import SqliteSaver
from langgraph.checkpoint.postgres import PostgresSaver

# 内存检查点（测试用）
memory = MemorySaver()

# SQLite 持久化检查点
with SqliteSaver.from_conn_string("./checkpoints.db") as checkpointer:
    graph = builder.compile(checkpointer=checkpointer)

# PostgreSQL 检查点（生产推荐）
with PostgresSaver.from_conn_string("postgresql://...") as checkpointer:
    graph = builder.compile(checkpointer=checkpointer)

# 编译时指定检查点
graph = builder.compile(checkpointer=memory)

# 使用 thread_id 区分不同会话
config = {"configurable": {"thread_id": "session_001"}}

# 第一次调用
result = graph.invoke({"messages": [HumanMessage("你好")]}, config=config)

# 后续调用（自动加载历史状态）
result = graph.invoke({"messages": [HumanMessage("我之前说了什么？")]}, config=config)

# 获取当前状态
state = graph.get_state(config)
print(state.values)   # 当前状态值
print(state.next)     # 下一步节点

# 获取历史状态（时间旅行）
history = list(graph.get_state_history(config))
for checkpoint in history:
    print(checkpoint.values, checkpoint.created_at)

# 从历史状态恢复
past_config = {"configurable": {"thread_id": "session_001", "checkpoint_id": "..."}}
result = graph.invoke(None, config=past_config)  # 从历史断点继续
```

---

### 3.7 人机协作（Human-in-the-Loop）

```python
from langgraph.types import interrupt, Command

# 方式一：使用 interrupt() 暂停执行
def review_node(state: dict):
    """需要人类审核的节点"""
    # 暂停并等待人类输入
    human_input = interrupt({
        "question": "请审核以下内容：",
        "content": state["draft"],
        "options": ["approve", "reject", "revise"],
    })
    
    if human_input == "approve":
        return {"status": "approved", "result": state["draft"]}
    elif human_input == "reject":
        return {"status": "rejected"}
    else:
        return {"status": "needs_revision", "feedback": human_input}

# 编译时启用中断
graph = builder.compile(
    checkpointer=memory,
    interrupt_before=["review_node"],  # 在节点执行前中断
    interrupt_after=["draft_node"],    # 在节点执行后中断
)

# 运行直到中断
config = {"configurable": {"thread_id": "review_001"}}
result = graph.invoke({"task": "写一篇文章"}, config=config)
# result 为 None 或部分结果，表示已中断

# 检查中断状态
state = graph.get_state(config)
print(state.next)     # 下一个待执行节点
print(state.tasks)    # 中断信息

# 人类提供反馈后继续
graph.update_state(config, {"feedback": "请更简洁"})

# 继续执行
result = graph.invoke(None, config=config)

# 方式二：使用 Command 控制流
def human_node(state):
    human_response = interrupt("需要你的输入：")
    return Command(
        update={"human_input": human_response},
        goto="next_node",  # 指定跳转目标
    )
```

---

### 3.8 子图（Subgraphs）

```python
# 创建子图
sub_builder = StateGraph(SubState)
sub_builder.add_node("sub_node_1", sub_fn_1)
sub_builder.add_node("sub_node_2", sub_fn_2)
sub_builder.add_edge(START, "sub_node_1")
sub_builder.add_edge("sub_node_1", "sub_node_2")
sub_builder.add_edge("sub_node_2", END)
subgraph = sub_builder.compile()

# 在父图中使用子图
class ParentState(TypedDict):
    input: str
    output: str

def run_subgraph(state: ParentState) -> ParentState:
    """将子图作为节点使用"""
    result = subgraph.invoke({"sub_input": state["input"]})
    return {"output": result["sub_output"]}

parent_builder = StateGraph(ParentState)
parent_builder.add_node("subgraph_node", run_subgraph)

# 直接将编译后的子图作为节点添加
# （状态需要有对应的键）
parent_builder.add_node("subgraph", subgraph)
```

---

### 3.9 并行执行

```python
from typing import TypedDict, Annotated
import operator

class ParallelState(TypedDict):
    input: str
    results: Annotated[list, operator.add]  # 并行结果合并

def search_arxiv(state):
    # 搜索学术论文
    return {"results": [f"arxiv: {state['input']}"]}

def search_web(state):
    # 搜索网络
    return {"results": [f"web: {state['input']}"]}

def search_database(state):
    # 搜索内部数据库
    return {"results": [f"db: {state['input']}"]}

def synthesize(state):
    # 汇总所有结果
    all_results = state["results"]
    return {"output": f"综合了 {len(all_results)} 个来源"}

builder = StateGraph(ParallelState)
builder.add_node("arxiv", search_arxiv)
builder.add_node("web", search_web)
builder.add_node("db", search_database)
builder.add_node("synthesize", synthesize)

# 并行扇出
builder.add_edge(START, "arxiv")
builder.add_edge(START, "web")
builder.add_edge(START, "db")

# 等待所有并行任务完成后汇总
builder.add_edge("arxiv", "synthesize")
builder.add_edge("web", "synthesize")
builder.add_edge("db", "synthesize")

builder.add_edge("synthesize", END)

graph = builder.compile()
result = graph.invoke({"input": "量子计算", "results": []})
```

#### Send API（动态并行）

```python
from langgraph.types import Send

class MapReduceState(TypedDict):
    topic: str
    subtopics: list[str]
    analyses: Annotated[list, operator.add]
    final_report: str

def generate_subtopics(state):
    """生成子主题列表"""
    subtopics = ["历史", "现状", "未来展望"]
    return {"subtopics": subtopics}

def analyze_subtopic(state):
    """分析单个子主题"""
    result = llm.invoke(f"分析 {state['subtopic']} 关于 {state['topic']}")
    return {"analyses": [result.content]}

def generate_report(state):
    """汇总生成报告"""
    combined = "\n".join(state["analyses"])
    return {"final_report": combined}

# 动态扇出：根据 subtopics 数量动态创建并行任务
def fan_out(state) -> list[Send]:
    return [
        Send("analyze", {"topic": state["topic"], "subtopic": sub})
        for sub in state["subtopics"]
    ]

builder = StateGraph(MapReduceState)
builder.add_node("generate_subtopics", generate_subtopics)
builder.add_node("analyze", analyze_subtopic)
builder.add_node("generate_report", generate_report)

builder.add_edge(START, "generate_subtopics")
builder.add_conditional_edges("generate_subtopics", fan_out, ["analyze"])
builder.add_edge("analyze", "generate_report")
builder.add_edge("generate_report", END)
```

---

### 3.10 流式输出（Streaming）

```python
# 流式节点事件
config = {"configurable": {"thread_id": "stream_001"}}

# 流式所有事件
for event in graph.stream(
    {"messages": [HumanMessage("你好")]},
    config=config,
    stream_mode="updates",  # updates / values / messages / debug
):
    print(event)

# stream_mode 说明：
# "values"  - 每步后的完整状态
# "updates" - 每步后的状态差异（推荐）
# "messages" - 消息流（适合聊天）
# "debug"   - 详细调试信息

# 只流式 LLM token（消息模式）
async def stream_tokens():
    async for msg, metadata in graph.astream(
        {"messages": [HumanMessage("讲个故事")]},
        config=config,
        stream_mode="messages",
    ):
        if hasattr(msg, "content") and msg.content:
            print(msg.content, end="", flush=True)

asyncio.run(stream_tokens())

# 多模式同时流式
async for chunk in graph.astream(
    input_data,
    config=config,
    stream_mode=["updates", "messages"],
):
    mode, data = chunk
    if mode == "messages":
        print(data[0].content, end="")
    elif mode == "updates":
        print(f"\n[更新]: {data}")
```

---

## 4. 常见应用模式

### 4.1 RAG 检索增强生成

```python
from langchain_core.runnables import RunnablePassthrough
from langchain_core.output_parsers import StrOutputParser

# === 基础 RAG ===

# 1. 构建向量库
docs = loader.load()
chunks = splitter.split_documents(docs)
vectorstore = FAISS.from_documents(chunks, embeddings)
retriever = vectorstore.as_retriever(search_kwargs={"k": 4})

# 2. 提示词模板
prompt = ChatPromptTemplate.from_template("""
根据以下上下文回答问题。如果上下文中没有相关信息，请说明不知道。

上下文：
{context}

问题：{question}

回答：
""")

# 3. 格式化文档
def format_docs(docs):
    return "\n\n".join(doc.page_content for doc in docs)

# 4. RAG 链
rag_chain = (
    {"context": retriever | format_docs, "question": RunnablePassthrough()}
    | prompt
    | llm
    | StrOutputParser()
)

answer = rag_chain.invoke("什么是向量数据库？")

# === LangGraph RAG（带来源引用）===
from langgraph.graph import StateGraph, MessagesState, START, END

class RAGState(TypedDict):
    question: str
    documents: list
    answer: str
    sources: list[str]

def retrieve(state: RAGState):
    docs = retriever.invoke(state["question"])
    return {
        "documents": docs,
        "sources": [doc.metadata.get("source", "未知") for doc in docs],
    }

def generate(state: RAGState):
    context = format_docs(state["documents"])
    prompt_text = f"上下文：{context}\n\n问题：{state['question']}"
    response = llm.invoke(prompt_text)
    return {"answer": response.content}

builder = StateGraph(RAGState)
builder.add_node("retrieve", retrieve)
builder.add_node("generate", generate)
builder.add_edge(START, "retrieve")
builder.add_edge("retrieve", "generate")
builder.add_edge("generate", END)

rag_graph = builder.compile()
result = rag_graph.invoke({"question": "什么是 RAG？", "documents": [], "answer": "", "sources": []})
print(result["answer"])
print(result["sources"])
```

---

### 4.2 ReAct 代理

```python
from langgraph.prebuilt import create_react_agent

# === 快速创建 ReAct 代理（推荐方式）===
tools = [search_web, calculate, get_weather]

agent = create_react_agent(
    model=llm,
    tools=tools,
    # 可选参数：
    state_modifier="你是一个有用的助手，善用工具解决问题。",
    checkpointer=MemorySaver(),   # 支持多轮对话记忆
)

config = {"configurable": {"thread_id": "agent_001"}}
result = agent.invoke(
    {"messages": [HumanMessage("北京今天天气怎么样？")]},
    config=config,
)
print(result["messages"][-1].content)

# === 手动构建 ReAct 代理（更灵活）===
class AgentState(MessagesState):
    pass

def call_model(state: AgentState):
    llm_with_tools = llm.bind_tools(tools)
    response = llm_with_tools.invoke(state["messages"])
    return {"messages": [response]}

def should_use_tools(state: AgentState) -> str:
    last_msg = state["messages"][-1]
    if hasattr(last_msg, "tool_calls") and last_msg.tool_calls:
        return "tools"
    return END

tool_node = ToolNode(tools)

builder = StateGraph(AgentState)
builder.add_node("agent", call_model)
builder.add_node("tools", tool_node)
builder.add_edge(START, "agent")
builder.add_conditional_edges("agent", should_use_tools)
builder.add_edge("tools", "agent")

react_graph = builder.compile(checkpointer=MemorySaver())

# 流式运行
for event in react_graph.stream(
    {"messages": [HumanMessage("计算 123 * 456，然后搜索这个数字")]},
    config={"configurable": {"thread_id": "t1"}},
    stream_mode="values",
):
    event["messages"][-1].pretty_print()
```

---

### 4.3 多代理系统

```python
from langgraph.graph import StateGraph, MessagesState, START, END

# === Supervisor 模式：一个主代理协调多个子代理 ===

class SupervisorState(MessagesState):
    next_agent: str = ""

# 创建专业子代理
research_agent = create_react_agent(llm, [search_web, wiki], state_modifier="你是研究专家。")
code_agent = create_react_agent(llm, [python_repl], state_modifier="你是代码专家。")
writing_agent = create_react_agent(llm, [], state_modifier="你是写作专家。")

# Supervisor 节点
def supervisor(state: SupervisorState):
    system_prompt = """你是一个任务调度员，根据当前任务分配给最合适的专家：
    - research_agent: 信息搜索和研究
    - code_agent: 代码编写和调试  
    - writing_agent: 文章写作和编辑
    - FINISH: 任务完成时返回

    当前对话历史：{messages}
    
    只返回下一步要调用的代理名称或 FINISH。"""
    
    response = llm.invoke([
        SystemMessage(content=system_prompt.format(messages=state["messages"])),
        HumanMessage(content="下一步应该调用哪个代理？"),
    ])
    return {"next_agent": response.content.strip()}

def route_to_agent(state: SupervisorState) -> str:
    return state["next_agent"]

# 子代理节点包装
def run_research(state):
    result = research_agent.invoke(state)
    return {"messages": result["messages"]}

def run_code(state):
    result = code_agent.invoke(state)
    return {"messages": result["messages"]}

def run_writing(state):
    result = writing_agent.invoke(state)
    return {"messages": result["messages"]}

# 构建多代理图
builder = StateGraph(SupervisorState)
builder.add_node("supervisor", supervisor)
builder.add_node("research_agent", run_research)
builder.add_node("code_agent", run_code)
builder.add_node("writing_agent", run_writing)

builder.add_edge(START, "supervisor")
builder.add_conditional_edges(
    "supervisor",
    route_to_agent,
    {
        "research_agent": "research_agent",
        "code_agent": "code_agent",
        "writing_agent": "writing_agent",
        "FINISH": END,
    },
)
# 每个子代理完成后回到 supervisor
builder.add_edge("research_agent", "supervisor")
builder.add_edge("code_agent", "supervisor")
builder.add_edge("writing_agent", "supervisor")

multi_agent = builder.compile()
```

---

### 4.4 对话式 RAG

```python
from langchain_core.prompts import ChatPromptTemplate, MessagesPlaceholder
from langchain.chains import create_history_aware_retriever, create_retrieval_chain
from langchain.chains.combine_documents import create_stuff_documents_chain

# === 带历史的对话式 RAG ===

# 步骤 1：上下文化查询（将历史融入检索查询）
contextualize_q_prompt = ChatPromptTemplate.from_messages([
    ("system", """给定聊天历史和最新的用户问题，
    请重写一个独立的、不需要聊天历史就能理解的问题。
    如果问题已经很清楚，直接返回原问题。"""),
    MessagesPlaceholder("chat_history"),
    ("human", "{input}"),
])

history_aware_retriever = create_history_aware_retriever(
    llm, retriever, contextualize_q_prompt
)

# 步骤 2：问答链（结合文档和历史）
qa_prompt = ChatPromptTemplate.from_messages([
    ("system", """你是一个问答助手。使用以下检索到的上下文回答问题。
    如果不知道答案，直接说不知道。
    
    上下文：{context}"""),
    MessagesPlaceholder("chat_history"),
    ("human", "{input}"),
])

question_answer_chain = create_stuff_documents_chain(llm, qa_prompt)

# 步骤 3：组合成 RAG 链
rag_chain = create_retrieval_chain(history_aware_retriever, question_answer_chain)

# 使用（手动管理历史）
chat_history = []

def chat(question: str) -> str:
    response = rag_chain.invoke({
        "input": question,
        "chat_history": chat_history,
    })
    chat_history.extend([
        HumanMessage(content=question),
        AIMessage(content=response["answer"]),
    ])
    return response["answer"]

print(chat("什么是 LangChain？"))
print(chat("它有哪些主要组件？"))  # 利用上下文理解 "它" 指 LangChain
```

---

## 5. 最佳实践与调试

### 使用 LangSmith 追踪

```python
# 设置追踪
import os
os.environ["LANGCHAIN_TRACING_V2"] = "true"
os.environ["LANGCHAIN_API_KEY"] = "your_key"
os.environ["LANGCHAIN_PROJECT"] = "production"

# 手动创建追踪
from langsmith import traceable

@traceable(name="my_rag_pipeline", run_type="chain")
def my_pipeline(question: str) -> str:
    docs = retriever.invoke(question)
    answer = rag_chain.invoke(question)
    return answer
```

### 缓存（减少 API 调用）

```python
from langchain.cache import InMemoryCache, SQLiteCache
from langchain.globals import set_llm_cache

# 内存缓存
set_llm_cache(InMemoryCache())

# SQLite 持久化缓存
set_llm_cache(SQLiteCache(database_path=".langchain.db"))

# Semantic Cache（语义相似缓存）
from langchain_community.cache import RedisSemanticCache

set_llm_cache(RedisSemanticCache(
    redis_url="redis://localhost:6379",
    embedding=embeddings,
    score_threshold=0.95,
))
```

### 速率限制与重试

```python
from langchain_core.rate_limiters import InMemoryRateLimiter

rate_limiter = InMemoryRateLimiter(
    requests_per_second=1,
    check_every_n_seconds=0.1,
    max_bucket_size=10,
)

llm = ChatOpenAI(
    model="gpt-4o",
    rate_limiter=rate_limiter,
)
```

### Token 使用量追踪

```python
from langchain_community.callbacks.manager import get_openai_callback

with get_openai_callback() as cb:
    result = chain.invoke({"question": "解释 LangChain"})
    print(f"总 Token: {cb.total_tokens}")
    print(f"提示 Token: {cb.prompt_tokens}")
    print(f"完成 Token: {cb.completion_tokens}")
    print(f"总费用: ${cb.total_cost:.6f}")
```

### LangGraph 状态更新技巧

```python
# 更新状态时只返回变化的键
def node_fn(state):
    # ✅ 正确：只返回需要更新的键
    return {"result": "some value"}
    
    # ❌ 错误：不要返回整个状态
    # return {**state, "result": "some value"}

# 使用 Command 进行复杂控制
from langgraph.types import Command

def conditional_node(state):
    if state["error"]:
        return Command(
            update={"status": "failed"},
            goto="error_handler",
        )
    return Command(
        update={"status": "success"},
        goto="next_step",
    )
```

### 图的调试

```python
# 打印图结构
graph.get_graph().print_ascii()

# 逐步执行并检查状态
for step in graph.stream(input_data, stream_mode="values"):
    print(f"当前状态: {step}")

# 检查检查点历史
for state in graph.get_state_history(config):
    print(f"时间: {state.created_at}")
    print(f"下一节点: {state.next}")
    print(f"状态: {state.values}")
    print("---")
```

---

## 6. 常见错误与解决方案

### 错误 1：RecursionError / 无限循环

```python
# ❌ 问题：条件边没有终止条件
# ✅ 解决：添加最大迭代限制

class State(TypedDict):
    messages: Annotated[list, operator.add]
    iteration: int

def check_limit(state: State) -> str:
    if state["iteration"] >= 10:
        return END  # 强制终止
    if should_stop(state):
        return END
    return "continue"

# 或在编译时设置
graph = builder.compile()
graph.invoke(input, config={"recursion_limit": 25})  # 默认 25
```

### 错误 2：状态键冲突

```python
# ❌ 问题：节点返回了不在状态定义中的键
def bad_node(state):
    return {"nonexistent_key": "value"}  # 会被忽略或报错

# ✅ 解决：确保键名与 State 定义一致
class State(TypedDict):
    result: str

def good_node(state):
    return {"result": "value"}  # 正确
```

### 错误 3：Pydantic 验证错误

```python
# ❌ 问题：结构化输出返回格式不符合 Schema
# ✅ 解决：使用更灵活的重试机制

from langchain_core.output_parsers import PydanticOutputParser
from langchain.output_parsers import OutputFixingParser

parser = PydanticOutputParser(pydantic_object=MyModel)
fixing_parser = OutputFixingParser.from_llm(parser=parser, llm=llm)

chain = prompt | llm | fixing_parser  # 自动修复格式错误
```

### 错误 4：检索质量差

```python
# 提升检索质量的方法：

# 1. 调整 chunk 大小（通常 500-1000 最优）
splitter = RecursiveCharacterTextSplitter(
    chunk_size=800,
    chunk_overlap=150,
)

# 2. 添加 metadata 过滤
retriever = vectorstore.as_retriever(
    search_kwargs={"filter": {"category": "technical"}}
)

# 3. 混合检索（向量 + 关键词）
ensemble = EnsembleRetriever(
    retrievers=[bm25_retriever, dense_retriever],
    weights=[0.3, 0.7],
)

# 4. 重排序（Reranking）
from langchain.retrievers.document_compressors import CrossEncoderReranker
from langchain_community.cross_encoders import HuggingFaceCrossEncoder

reranker = CrossEncoderReranker(
    model=HuggingFaceCrossEncoder(model_name="BAAI/bge-reranker-base"),
    top_n=3,
)
```

### 错误 5：Token 超限

```python
# ❌ 问题：文档太长超过模型上下文窗口
# ✅ 解决方案：

# 方案 1：Map-Reduce 文档链
from langchain.chains.summarize import load_summarize_chain

chain = load_summarize_chain(
    llm,
    chain_type="map_reduce",  # map / refine / map_reduce
)
summary = chain.invoke(long_docs)

# 方案 2：限制检索数量
retriever = vectorstore.as_retriever(search_kwargs={"k": 3})  # 减少 k

# 方案 3：压缩文档
from langchain.retrievers.document_compressors import LLMChainExtractor
compressor = LLMChainExtractor.from_llm(llm)
```

---

## 快速参考卡

| 功能 | LangChain 方式 | LangGraph 方式 |
|------|----------------|----------------|
| 顺序执行 | `A \| B \| C` | `add_edge(A, B); add_edge(B, C)` |
| 条件分支 | `RunnableBranch` | `add_conditional_edges` |
| 并行执行 | `RunnableParallel` | 多条 `add_edge` 到同一节点 |
| 循环 | ❌ 不支持 | `add_edge(B, A)`（条件终止）|
| 状态持久化 | `RunnableWithMessageHistory` | `checkpointer=MemorySaver()` |
| 流式输出 | `.stream()` / `.astream()` | `.stream(stream_mode=...)` |
| 工具调用 | `AgentExecutor` | `ToolNode` + 条件边 |
| 人机协作 | ❌ 不原生支持 | `interrupt()` + `interrupt_before` |

---

## 资源链接

- 📖 [LangChain 官方文档](https://python.langchain.com/docs/)
- 📖 [LangGraph 官方文档](https://langchain-ai.github.io/langgraph/)
- 🧪 [LangSmith（追踪调试）](https://smith.langchain.com/)
- 📦 [LangChain Hub（提示词库）](https://smith.langchain.com/hub)
- 💻 [LangGraph 示例](https://github.com/langchain-ai/langgraph/tree/main/examples)
- 🎓 [LangChain Academy（教程）](https://academy.langchain.com/)

---

*文档由 Claude 生成 · 欢迎反馈与贡献*
