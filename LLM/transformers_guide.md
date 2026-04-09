# 🤗 Transformers 库完全指南

> Hugging Face `transformers` 是当前最流行的自然语言处理（NLP）、计算机视觉（CV）及多模态 AI 开源库。  
> 本文档系统性地梳理了日常开发中最常用的模块、类、方法与最佳实践。

---

## 目录

- [1. 安装与环境配置](#1-安装与环境配置)
- [2. 核心概念总览](#2-核心概念总览)
- [3. Pipeline —— 开箱即用的推理管道](#3-pipeline--开箱即用的推理管道)
- [4. Tokenizer —— 分词器](#4-tokenizer--分词器)
- [5. Model —— 模型加载与使用](#5-model--模型加载与使用)
- [6. AutoClass —— 自动类](#6-autoclass--自动类)
- [7. 配置（Config）](#7-配置config)
- [8. 数据处理与 Dataset 集成](#8-数据处理与-dataset-集成)
- [9. Trainer —— 训练器](#9-trainer--训练器)
- [10. 微调（Fine-Tuning）实战](#10-微调fine-tuning实战)
- [11. 生成（Generation）](#11-生成generation)
- [12. 模型量化与优化](#12-模型量化与优化)
- [13. PEFT 与 LoRA 微调](#13-peft-与-lora-微调)
- [14. 多模态模型](#14-多模态模型)
- [15. 模型保存、加载与分享](#15-模型保存加载与分享)
- [16. 常用工具函数](#16-常用工具函数)
- [17. 常见问题与排错](#17-常见问题与排错)
- [18. 速查表](#18-速查表)

---

## 1. 安装与环境配置

### 1.1 基础安装

```bash
# 基础安装（仅 PyTorch 后端）
pip install transformers

# 安装 transformers + PyTorch
pip install transformers[torch]

# 安装 transformers + TensorFlow
pip install transformers[tf-cpu]   # CPU 版
pip install transformers[tf-gpu]   # GPU 版

# 安装 transformers + Flax/JAX
pip install transformers[flax]

# 完整安装（所有可选依赖）
pip install transformers[all]
```

### 1.2 从源码安装（获取最新特性）

```bash
pip install git+https://github.com/huggingface/transformers
```

### 1.3 常见配套库

```bash
# 数据集处理
pip install datasets

# 模型评估
pip install evaluate

# 参数高效微调
pip install peft

# 加速训练（多卡/混合精度）
pip install accelerate

# 分词器快速后端
pip install tokenizers

# 模型量化
pip install bitsandbytes

# 可视化训练过程
pip install tensorboard wandb
```

### 1.4 验证安装

```python
import transformers
print(transformers.__version__)
# Output: 4.x.x

# 检查后端
import torch
print(f"PyTorch: {torch.__version__}")
print(f"CUDA available: {torch.cuda.is_available()}")
```

### 1.5 环境变量

| 变量名 | 说明 | 示例 |
|--------|------|------|
| `HF_HOME` | Hugging Face 缓存根目录 | `~/.cache/huggingface` |
| `TRANSFORMERS_CACHE` | 模型/分词器缓存目录 | `~/.cache/huggingface/hub` |
| `HF_TOKEN` | Hugging Face API Token | `hf_xxxxxxxxxxxx` |
| `TRANSFORMERS_OFFLINE` | 离线模式（`1` 启用） | `1` |
| `CUDA_VISIBLE_DEVICES` | 指定可见 GPU | `0,1` |

---

## 2. 核心概念总览

`transformers` 库的设计围绕三大核心组件：

```
┌──────────────────────────────────────────────────────┐
│                   Pipeline（管道）                     │
│        一行代码完成"输入 → 预处理 → 推理 → 后处理"       │
└──────────────┬──────────────────────┬────────────────┘
               │                      │
       ┌───────▼───────┐      ┌───────▼───────┐
       │   Tokenizer   │      │     Model     │
       │   分词 & 编码   │      │  模型前向传播   │
       └───────┬───────┘      └───────┬───────┘
               │                      │
       ┌───────▼──────────────────────▼───────┐
       │              Config（配置）             │
       │         超参数、模型结构定义             │
       └──────────────────────────────────────┘
```

| 组件 | 职责 | 代表类 |
|------|------|--------|
| **Tokenizer** | 文本 ↔ token ID 双向转换 | `AutoTokenizer` |
| **Model** | 接收 token ID，输出 logits/隐状态 | `AutoModel`, `AutoModelForXxx` |
| **Config** | 定义模型结构（层数、头数等） | `AutoConfig` |
| **Pipeline** | 将上述三者封装为端到端推理 | `pipeline()` |

---

## 3. Pipeline —— 开箱即用的推理管道

`pipeline()` 是最简单的推理入口，一行代码即可完成完整的推理流程。

### 3.1 基本用法

```python
from transformers import pipeline

# 自动选择模型
classifier = pipeline("sentiment-analysis")
result = classifier("I love this product!")
print(result)
# Output: [{'label': 'POSITIVE', 'score': 0.9998}]
```

### 3.2 指定模型

```python
# 指定特定模型和分词器
classifier = pipeline(
    "sentiment-analysis",
    model="nlptown/bert-base-multilingual-uncased-sentiment",
    tokenizer="nlptown/bert-base-multilingual-uncased-sentiment",
)

# 指定设备
classifier = pipeline("sentiment-analysis", device=0)        # GPU:0
classifier = pipeline("sentiment-analysis", device="cpu")     # CPU
classifier = pipeline("sentiment-analysis", device="mps")     # Apple Silicon
```

### 3.3 支持的任务类型一览

#### 自然语言处理（NLP）

| 任务名称 | `task` 参数 | 说明 |
|----------|-------------|------|
| 文本分类 | `"text-classification"` / `"sentiment-analysis"` | 情感分析、主题分类 |
| Token 分类 | `"token-classification"` / `"ner"` | 命名实体识别（NER） |
| 问答 | `"question-answering"` | 抽取式问答 |
| 文本摘要 | `"summarization"` | 自动摘要 |
| 翻译 | `"translation"` | 机器翻译 |
| 文本生成 | `"text-generation"` | 自由文本生成 |
| 填空 | `"fill-mask"` | 掩码语言模型预测 |
| 零样本分类 | `"zero-shot-classification"` | 无需训练的分类 |
| 对话 | `"conversational"` | 多轮对话 |
| 特征提取 | `"feature-extraction"` | 获取文本向量表示 |
| 表格问答 | `"table-question-answering"` | 对表格数据提问 |
| Text-to-Text | `"text2text-generation"` | T5 等编解码器模型 |

#### 计算机视觉（CV）

| 任务名称 | `task` 参数 | 说明 |
|----------|-------------|------|
| 图像分类 | `"image-classification"` | 对图片进行分类 |
| 目标检测 | `"object-detection"` | 检测图片中的物体 |
| 图像分割 | `"image-segmentation"` | 语义/实例分割 |
| 深度估计 | `"depth-estimation"` | 估计图像深度 |
| 图片生成 | `"image-to-image"` | 图像变换 |
| 零样本图像分类 | `"zero-shot-image-classification"` | 无需训练的图像分类 |

#### 音频（Audio）

| 任务名称 | `task` 参数 | 说明 |
|----------|-------------|------|
| 语音识别 | `"automatic-speech-recognition"` | ASR 语音转文字 |
| 音频分类 | `"audio-classification"` | 音频场景分类 |
| 文字转语音 | `"text-to-speech"` | TTS |

#### 多模态（Multimodal）

| 任务名称 | `task` 参数 | 说明 |
|----------|-------------|------|
| 视觉问答 | `"visual-question-answering"` | 对图片提问 |
| 文档问答 | `"document-question-answering"` | 对文档图片提问 |
| 图像描述 | `"image-to-text"` | 自动生成图片描述 |

### 3.4 批量处理

```python
classifier = pipeline("sentiment-analysis")

# 批量输入
results = classifier([
    "I love this movie!",
    "This is terrible.",
    "It's okay, nothing special.",
])
# Output: [{'label': 'POSITIVE', ...}, {'label': 'NEGATIVE', ...}, {'label': 'NEGATIVE', ...}]
```

### 3.5 Pipeline 高级参数

```python
generator = pipeline(
    "text-generation",
    model="gpt2",
    device=0,
    batch_size=8,            # 批处理大小
    torch_dtype="auto",      # 自动选择精度
    model_kwargs={            # 传给模型的额外参数
        "load_in_8bit": True,
    },
)
```

### 3.6 自定义 Pipeline 组件

```python
from transformers import AutoTokenizer, AutoModelForSequenceClassification, pipeline

# 先单独加载
tokenizer = AutoTokenizer.from_pretrained("bert-base-uncased")
model = AutoModelForSequenceClassification.from_pretrained("bert-base-uncased")

# 再组装进 pipeline
classifier = pipeline("sentiment-analysis", model=model, tokenizer=tokenizer)
```

---

## 4. Tokenizer —— 分词器

分词器负责将原始文本转换为模型可以理解的数字序列，同时也负责将模型输出解码回文本。

### 4.1 加载分词器

```python
from transformers import AutoTokenizer

# 从 Hub 加载
tokenizer = AutoTokenizer.from_pretrained("bert-base-uncased")

# 从本地加载
tokenizer = AutoTokenizer.from_pretrained("./my_model/")

# 加载需要认证的模型
tokenizer = AutoTokenizer.from_pretrained(
    "meta-llama/Llama-2-7b",
    token="hf_xxxxxxxxxxxx"
)
```

### 4.2 编码（Encode）

```python
# 基本编码
encoded = tokenizer("Hello, how are you?")
print(encoded)
# Output:
# {
#   'input_ids': [101, 7592, 1010, 2129, 2024, 2017, 1029, 102],
#   'token_type_ids': [0, 0, 0, 0, 0, 0, 0, 0],
#   'attention_mask': [1, 1, 1, 1, 1, 1, 1, 1]
# }

# 返回 PyTorch 张量
encoded = tokenizer("Hello", return_tensors="pt")     # PyTorch
encoded = tokenizer("Hello", return_tensors="tf")     # TensorFlow
encoded = tokenizer("Hello", return_tensors="np")     # NumPy

# 截断与填充
encoded = tokenizer(
    "A very long sentence...",
    max_length=128,
    padding="max_length",       # 填充到 max_length
    truncation=True,            # 超长截断
    return_tensors="pt",
)
```

### 4.3 批量编码

```python
texts = ["Hello world", "How are you?", "Fine, thanks!"]

# 批量编码 + 动态填充到最长句子
batch = tokenizer(
    texts,
    padding=True,               # 填充到 batch 内最长
    truncation=True,
    max_length=512,
    return_tensors="pt",
)

print(batch["input_ids"].shape)
# Output: torch.Size([3, max_len_in_batch])
```

### 4.4 解码（Decode）

```python
# 单条解码
ids = [101, 7592, 1010, 2129, 2024, 2017, 1029, 102]
text = tokenizer.decode(ids)
print(text)
# Output: "[CLS] hello, how are you? [SEP]"

# 跳过特殊 token
text = tokenizer.decode(ids, skip_special_tokens=True)
print(text)
# Output: "hello, how are you?"

# 批量解码
texts = tokenizer.batch_decode(
    [[101, 7592, 102], [101, 2129, 102]],
    skip_special_tokens=True
)
# Output: ["hello", "how"]
```

### 4.5 句对编码

```python
# 用于 NLI、问答等需要两段输入的任务
encoded = tokenizer(
    "What is the capital of France?",    # 第一个句子
    "Paris is the capital of France.",    # 第二个句子
    padding=True,
    truncation=True,
    return_tensors="pt",
)

# token_type_ids 会区分两段文本：0 = 句子A, 1 = 句子B
print(encoded["token_type_ids"])
# Output: tensor([[0, 0, ..., 0, 1, 1, ..., 1]])
```

### 4.6 特殊 Token

```python
# 查看特殊 token
print(tokenizer.special_tokens_map)
# Output:
# {
#   'unk_token': '[UNK]',
#   'sep_token': '[SEP]',
#   'pad_token': '[PAD]',
#   'cls_token': '[CLS]',
#   'mask_token': '[MASK]'
# }

# 特殊 token 的 ID
print(tokenizer.cls_token_id)   # 101
print(tokenizer.sep_token_id)   # 102
print(tokenizer.pad_token_id)   # 0
print(tokenizer.eos_token_id)   # 视模型而定

# 词汇表大小
print(tokenizer.vocab_size)     # 30522 (BERT)

# 添加特殊 token（用于自定义任务）
tokenizer.add_special_tokens({"additional_special_tokens": ["<CUSTOM>"]})
```

### 4.7 Padding 策略详解

| 参数值 | 行为 |
|--------|------|
| `padding=True` / `padding="longest"` | 填充到 batch 中最长序列的长度 |
| `padding="max_length"` | 填充到 `max_length` 参数指定的长度 |
| `padding=False` / `padding="do_not_pad"` | 不填充 |

### 4.8 Truncation 策略详解

| 参数值 | 行为 |
|--------|------|
| `truncation=True` / `truncation="longest_first"` | 截断最长的序列（句对时） |
| `truncation="only_first"` | 只截断第一个序列 |
| `truncation="only_second"` | 只截断第二个序列 |
| `truncation=False` / `truncation="do_not_truncate"` | 不截断 |

### 4.9 Fast Tokenizer vs Slow Tokenizer

```python
# Fast Tokenizer（Rust 实现，推荐）
tokenizer = AutoTokenizer.from_pretrained("bert-base-uncased", use_fast=True)

# Slow Tokenizer（纯 Python 实现）
tokenizer = AutoTokenizer.from_pretrained("bert-base-uncased", use_fast=False)

# Fast Tokenizer 额外功能：offset_mapping（字符级映射）
encoded = tokenizer("Hello world", return_offsets_mapping=True)
print(encoded["offset_mapping"])
# Output: [(0, 0), (0, 5), (6, 11), (0, 0)]
# 每个 token 对应原文的字符起止位置
```

### 4.10 聊天模板（Chat Template）

适用于对话式模型（如 ChatGLM、LLaMA-Chat 等）：

```python
from transformers import AutoTokenizer

tokenizer = AutoTokenizer.from_pretrained("meta-llama/Llama-2-7b-chat-hf")

messages = [
    {"role": "system", "content": "You are a helpful assistant."},
    {"role": "user", "content": "What is Python?"},
    {"role": "assistant", "content": "Python is a programming language."},
    {"role": "user", "content": "Tell me more."},
]

# 应用聊天模板
formatted = tokenizer.apply_chat_template(
    messages,
    tokenize=False,           # 仅格式化，不编码
    add_generation_prompt=True # 添加生成提示符
)
print(formatted)

# 直接编码
input_ids = tokenizer.apply_chat_template(
    messages,
    tokenize=True,
    return_tensors="pt",
    add_generation_prompt=True
)
```

---

## 5. Model —— 模型加载与使用

### 5.1 模型加载

```python
from transformers import AutoModel, AutoModelForSequenceClassification

# 通用基础模型（只输出隐状态，无任务头）
model = AutoModel.from_pretrained("bert-base-uncased")

# 带分类头的模型
model = AutoModelForSequenceClassification.from_pretrained(
    "bert-base-uncased",
    num_labels=3   # 3 类分类
)

# 指定精度
model = AutoModel.from_pretrained("bert-base-uncased", torch_dtype=torch.float16)

# 加载到指定设备
model = AutoModel.from_pretrained("bert-base-uncased").to("cuda:0")

# 自动分配设备（大模型多卡推理）
model = AutoModel.from_pretrained(
    "bigscience/bloom-7b1",
    device_map="auto",
    torch_dtype=torch.float16,
)
```

### 5.2 常用 AutoModelForXxx 任务类

| 类名 | 任务 | 输出 |
|------|------|------|
| `AutoModel` | 基础模型 | 隐状态 |
| `AutoModelForSequenceClassification` | 文本分类 | logits (num_labels,) |
| `AutoModelForTokenClassification` | Token 分类 / NER | logits (seq_len, num_labels) |
| `AutoModelForQuestionAnswering` | 抽取式问答 | start_logits, end_logits |
| `AutoModelForMaskedLM` | 掩码语言模型 | logits (seq_len, vocab_size) |
| `AutoModelForCausalLM` | 因果语言模型 / 文本生成 | logits (seq_len, vocab_size) |
| `AutoModelForSeq2SeqLM` | 编解码模型（翻译/摘要） | logits (seq_len, vocab_size) |
| `AutoModelForMultipleChoice` | 多选题 | logits (num_choices,) |
| `AutoModelForNextSentencePrediction` | 下一句预测 | logits (2,) |
| `AutoModelForImageClassification` | 图像分类 | logits (num_labels,) |
| `AutoModelForObjectDetection` | 目标检测 | boxes, scores, labels |
| `AutoModelForAudioClassification` | 音频分类 | logits (num_labels,) |
| `AutoModelForSpeechSeq2Seq` | 语音识别（Whisper） | logits |
| `AutoModelForVision2Seq` | 图像描述 | logits |

### 5.3 模型推理（手动方式）

```python
import torch
from transformers import AutoTokenizer, AutoModelForSequenceClassification

tokenizer = AutoTokenizer.from_pretrained("bert-base-uncased")
model = AutoModelForSequenceClassification.from_pretrained("bert-base-uncased", num_labels=2)

# 编码
inputs = tokenizer("I love transformers!", return_tensors="pt")

# 推理（关闭梯度计算）
with torch.no_grad():
    outputs = model(**inputs)

# 获取结果
logits = outputs.logits
predicted_class = torch.argmax(logits, dim=-1).item()
probabilities = torch.softmax(logits, dim=-1)
print(f"Predicted class: {predicted_class}")
print(f"Probabilities: {probabilities}")
```

### 5.4 模型输出结构

所有模型输出都是 `dataclass` 风格的对象，可按属性名或索引访问：

```python
outputs = model(**inputs)

# 按属性访问
logits = outputs.logits
hidden_states = outputs.hidden_states      # 需要 output_hidden_states=True
attentions = outputs.attentions            # 需要 output_attentions=True

# 按索引访问（不推荐）
logits = outputs[0]

# 获取所有隐藏层输出
outputs = model(**inputs, output_hidden_states=True)
all_hidden = outputs.hidden_states         # tuple: (embedding, layer1, layer2, ...)
last_hidden = outputs.last_hidden_state    # 最后一层隐状态
```

### 5.5 模型参数查看

```python
# 查看模型结构
print(model)

# 参数数量
total_params = sum(p.numel() for p in model.parameters())
trainable_params = sum(p.numel() for p in model.parameters() if p.requires_grad)
print(f"Total: {total_params:,}")
print(f"Trainable: {trainable_params:,}")

# 查看参数名和形状
for name, param in model.named_parameters():
    print(f"{name}: {param.shape}")
```

### 5.6 冻结/解冻参数

```python
# 冻结所有参数
for param in model.parameters():
    param.requires_grad = False

# 只解冻分类头
for param in model.classifier.parameters():
    param.requires_grad = True

# 冻结特定层（如 BERT 的前 6 层）
for name, param in model.named_parameters():
    if "encoder.layer" in name:
        layer_num = int(name.split("encoder.layer.")[1].split(".")[0])
        if layer_num < 6:
            param.requires_grad = False
```

---

## 6. AutoClass —— 自动类

`Auto` 系列类是 `transformers` 的核心设计模式。你只需提供模型名称，`AutoClass` 会自动查找并实例化正确的架构类。

### 6.1 自动类一览

```python
from transformers import (
    AutoConfig,                              # 自动配置
    AutoTokenizer,                           # 自动分词器
    AutoModel,                               # 自动基础模型
    AutoModelForSequenceClassification,      # 自动文本分类模型
    AutoModelForCausalLM,                    # 自动因果语言模型
    AutoModelForSeq2SeqLM,                   # 自动编解码模型
    AutoModelForTokenClassification,         # 自动 Token 分类模型
    AutoModelForQuestionAnswering,           # 自动问答模型
    AutoFeatureExtractor,                    # 自动特征提取器（视觉）
    AutoProcessor,                           # 自动处理器（多模态）
    AutoImageProcessor,                      # 自动图像处理器
)
```

### 6.2 工作原理

```python
# 当你调用：
model = AutoModelForCausalLM.from_pretrained("gpt2")

# transformers 会：
# 1. 下载 config.json
# 2. 读取 config.json 中的 "model_type": "gpt2"
# 3. 查找映射表 → GPT2LMHeadModel
# 4. 用 GPT2LMHeadModel.from_pretrained("gpt2") 加载权重

# 等价于：
from transformers import GPT2LMHeadModel
model = GPT2LMHeadModel.from_pretrained("gpt2")
```

### 6.3 AutoProcessor（多模态模型专用）

```python
from transformers import AutoProcessor

# 加载多模态处理器（自动包含 tokenizer + image_processor）
processor = AutoProcessor.from_pretrained("openai/clip-vit-base-patch32")

# 同时处理文本和图像
from PIL import Image
image = Image.open("photo.jpg")
inputs = processor(text=["a photo of a cat"], images=image, return_tensors="pt")
```

---

## 7. 配置（Config）

Config 对象定义了模型的结构参数（如层数、头数、隐藏维度等），独立于权重存储。

### 7.1 加载与查看配置

```python
from transformers import AutoConfig

config = AutoConfig.from_pretrained("bert-base-uncased")
print(config)
# BertConfig {
#   "hidden_size": 768,
#   "num_attention_heads": 12,
#   "num_hidden_layers": 12,
#   "vocab_size": 30522,
#   ...
# }

# 访问具体参数
print(config.hidden_size)        # 768
print(config.num_hidden_layers)  # 12
```

### 7.2 自定义配置

```python
from transformers import BertConfig, BertModel

# 从零创建自定义配置
config = BertConfig(
    vocab_size=30000,
    hidden_size=512,
    num_hidden_layers=6,
    num_attention_heads=8,
    intermediate_size=2048,
)

# 用自定义配置创建全新模型（随机初始化）
model = BertModel(config)
```

### 7.3 修改已有配置

```python
config = AutoConfig.from_pretrained("bert-base-uncased")
config.num_labels = 5                  # 修改分类数
config.hidden_dropout_prob = 0.2       # 修改 dropout

model = AutoModelForSequenceClassification.from_pretrained(
    "bert-base-uncased",
    config=config
)
```

### 7.4 保存与加载配置

```python
# 保存
config.save_pretrained("./my_config/")
# 生成 ./my_config/config.json

# 加载
config = AutoConfig.from_pretrained("./my_config/")
```

---

## 8. 数据处理与 Dataset 集成

### 8.1 加载 Hugging Face Datasets

```python
from datasets import load_dataset

# 从 Hub 加载
dataset = load_dataset("imdb")
print(dataset)
# DatasetDict({
#     train: Dataset({features: ['text', 'label'], num_rows: 25000})
#     test:  Dataset({features: ['text', 'label'], num_rows: 25000})
# })

# 加载特定 split
train_data = load_dataset("imdb", split="train")
test_data = load_dataset("imdb", split="test[:1000]")  # 前 1000 条

# 加载本地文件
dataset = load_dataset("csv", data_files="data.csv")
dataset = load_dataset("json", data_files="data.json")
dataset = load_dataset("text", data_files="data.txt")
```

### 8.2 数据集预处理（Tokenize）

```python
from transformers import AutoTokenizer
from datasets import load_dataset

tokenizer = AutoTokenizer.from_pretrained("bert-base-uncased")
dataset = load_dataset("imdb")

# 定义预处理函数
def preprocess_function(examples):
    return tokenizer(
        examples["text"],
        truncation=True,
        padding="max_length",
        max_length=256,
    )

# 应用到整个数据集（批处理模式）
tokenized_dataset = dataset.map(
    preprocess_function,
    batched=True,             # 批量处理，更快
    num_proc=4,               # 多进程
    remove_columns=["text"],  # 移除不需要的列
)

# 设置格式
tokenized_dataset.set_format("torch")
```

### 8.3 自定义 PyTorch Dataset

```python
import torch
from torch.utils.data import Dataset, DataLoader

class TextDataset(Dataset):
    def __init__(self, texts, labels, tokenizer, max_length=128):
        self.encodings = tokenizer(
            texts, truncation=True, padding="max_length",
            max_length=max_length, return_tensors="pt"
        )
        self.labels = torch.tensor(labels, dtype=torch.long)

    def __len__(self):
        return len(self.labels)

    def __getitem__(self, idx):
        item = {key: val[idx] for key, val in self.encodings.items()}
        item["labels"] = self.labels[idx]
        return item

# 使用
dataset = TextDataset(
    texts=["Great!", "Terrible."],
    labels=[1, 0],
    tokenizer=tokenizer
)
dataloader = DataLoader(dataset, batch_size=16, shuffle=True)
```

### 8.4 DataCollator（动态填充）

```python
from transformers import DataCollatorWithPadding

# 动态填充（在 collate 时才填充到 batch 内最长，比预先 padding 更高效）
data_collator = DataCollatorWithPadding(tokenizer=tokenizer)

# 语言模型专用
from transformers import DataCollatorForLanguageModeling

# MLM 数据整理器（自动随机 mask 15% token）
data_collator = DataCollatorForLanguageModeling(
    tokenizer=tokenizer,
    mlm=True,
    mlm_probability=0.15,
)

# CLM 数据整理器（自动构造 labels = input_ids 右移一位）
data_collator = DataCollatorForLanguageModeling(
    tokenizer=tokenizer,
    mlm=False,
)

# Seq2Seq 数据整理器
from transformers import DataCollatorForSeq2Seq
data_collator = DataCollatorForSeq2Seq(tokenizer=tokenizer, model=model)
```

### 8.5 常用 DataCollator 一览

| DataCollator | 用途 |
|--------------|------|
| `DataCollatorWithPadding` | 动态填充，用于分类任务 |
| `DataCollatorForLanguageModeling` | MLM 或 CLM 的数据整理 |
| `DataCollatorForSeq2Seq` | Seq2Seq 翻译/摘要任务 |
| `DataCollatorForTokenClassification` | NER 等 Token 分类任务 |
| `DataCollatorForWholeWordMask` | 全词掩码（中文 BERT 等） |
| `DataCollatorForPermutationLanguageModeling` | XLNet 排列语言模型 |

---

## 9. Trainer —— 训练器

`Trainer` 是 `transformers` 提供的高级训练 API，封装了训练循环、评估、日志、保存等全部逻辑。

### 9.1 TrainingArguments（训练参数）

```python
from transformers import TrainingArguments

training_args = TrainingArguments(
    # === 基本设置 ===
    output_dir="./results",              # 输出目录
    overwrite_output_dir=True,           # 覆盖已有输出

    # === 训练轮数与批次 ===
    num_train_epochs=3,                  # 训练轮数
    per_device_train_batch_size=16,      # 每卡训练 batch size
    per_device_eval_batch_size=32,       # 每卡评估 batch size
    gradient_accumulation_steps=2,       # 梯度累积步数（等效 batch_size × 2）

    # === 学习率与优化器 ===
    learning_rate=5e-5,                  # 学习率
    weight_decay=0.01,                   # 权重衰减
    warmup_steps=500,                    # 预热步数
    warmup_ratio=0.1,                    # 或用预热比例
    lr_scheduler_type="cosine",          # 学习率调度器
    optim="adamw_torch",                # 优化器

    # === 评估与保存 ===
    eval_strategy="steps",               # 评估策略：steps / epoch / no
    eval_steps=500,                      # 每 N 步评估一次
    save_strategy="steps",               # 保存策略
    save_steps=500,                      # 每 N 步保存一次
    save_total_limit=3,                  # 最多保留 N 个 checkpoint
    load_best_model_at_end=True,         # 训练结束加载最优模型
    metric_for_best_model="f1",          # 最优模型的评判指标

    # === 精度与性能 ===
    fp16=True,                           # 混合精度（NVIDIA GPU）
    bf16=False,                          # BF16 精度（A100+）
    dataloader_num_workers=4,            # DataLoader 工作进程数

    # === 日志 ===
    logging_dir="./logs",                # 日志目录
    logging_steps=100,                   # 每 N 步记录日志
    report_to="tensorboard",             # 日志平台：tensorboard / wandb / none

    # === 其他 ===
    seed=42,                             # 随机种子
    remove_unused_columns=True,          # 自动移除模型不需要的列
    push_to_hub=False,                   # 训练后推送到 Hub
)
```

### 9.2 常用学习率调度器

| `lr_scheduler_type` | 说明 |
|----------------------|------|
| `"linear"` | 线性衰减（默认） |
| `"cosine"` | 余弦退火 |
| `"cosine_with_restarts"` | 带重启的余弦退火 |
| `"polynomial"` | 多项式衰减 |
| `"constant"` | 常数学习率 |
| `"constant_with_warmup"` | 预热后常数 |
| `"inverse_sqrt"` | 逆平方根衰减 |

### 9.3 创建 Trainer

```python
from transformers import Trainer, TrainingArguments
import evaluate
import numpy as np

# 加载评估指标
metric = evaluate.load("accuracy")

def compute_metrics(eval_pred):
    logits, labels = eval_pred
    predictions = np.argmax(logits, axis=-1)
    return metric.compute(predictions=predictions, references=labels)

trainer = Trainer(
    model=model,
    args=training_args,
    train_dataset=train_dataset,
    eval_dataset=eval_dataset,
    tokenizer=tokenizer,
    data_collator=data_collator,
    compute_metrics=compute_metrics,
)
```

### 9.4 训练与评估

```python
# 开始训练
trainer.train()

# 从 checkpoint 恢复训练
trainer.train(resume_from_checkpoint="./results/checkpoint-500")

# 评估
results = trainer.evaluate()
print(results)
# {'eval_loss': 0.32, 'eval_accuracy': 0.91, ...}

# 预测
predictions = trainer.predict(test_dataset)
print(predictions.predictions.shape)  # (num_samples, num_labels)
print(predictions.label_ids.shape)    # (num_samples,)
print(predictions.metrics)            # {'test_loss': ..., 'test_accuracy': ...}
```

### 9.5 自定义 Trainer

```python
from transformers import Trainer

class CustomTrainer(Trainer):
    def compute_loss(self, model, inputs, return_outputs=False, **kwargs):
        """自定义损失函数"""
        labels = inputs.pop("labels")
        outputs = model(**inputs)
        logits = outputs.logits

        # 例：加权交叉熵
        loss_fn = torch.nn.CrossEntropyLoss(weight=torch.tensor([1.0, 2.0]).to(logits.device))
        loss = loss_fn(logits, labels)

        return (loss, outputs) if return_outputs else loss
```

### 9.6 Callback（回调）

```python
from transformers import TrainerCallback

class PrintCallback(TrainerCallback):
    def on_epoch_end(self, args, state, control, **kwargs):
        print(f"Epoch {state.epoch} finished!")

    def on_log(self, args, state, control, logs=None, **kwargs):
        if logs:
            print(f"Step {state.global_step}: {logs}")

# 使用回调
trainer = Trainer(
    model=model,
    args=training_args,
    callbacks=[PrintCallback()],
    # ...
)

# 内置回调
from transformers import EarlyStoppingCallback

trainer = Trainer(
    # ...
    callbacks=[
        EarlyStoppingCallback(early_stopping_patience=3),
    ],
)
```

### 9.7 常用内置 Callback

| Callback | 功能 |
|----------|------|
| `EarlyStoppingCallback` | 早停（需 `load_best_model_at_end=True`） |
| `TensorBoardCallback` | TensorBoard 日志 |
| `WandbCallback` | Weights & Biases 日志 |
| `PrinterCallback` | 打印训练进度到控制台 |
| `ProgressCallback` | 进度条（tqdm） |

---

## 10. 微调（Fine-Tuning）实战

### 10.1 文本分类完整示例

```python
import torch
import numpy as np
import evaluate
from datasets import load_dataset
from transformers import (
    AutoTokenizer,
    AutoModelForSequenceClassification,
    TrainingArguments,
    Trainer,
    DataCollatorWithPadding,
)

# 1. 加载数据
dataset = load_dataset("imdb")

# 2. 加载分词器
tokenizer = AutoTokenizer.from_pretrained("bert-base-uncased")

# 3. 数据预处理
def tokenize_fn(examples):
    return tokenizer(examples["text"], truncation=True, max_length=256)

tokenized = dataset.map(tokenize_fn, batched=True, remove_columns=["text"])

# 4. 加载模型
model = AutoModelForSequenceClassification.from_pretrained(
    "bert-base-uncased",
    num_labels=2,
)

# 5. 定义评估指标
accuracy = evaluate.load("accuracy")

def compute_metrics(eval_pred):
    logits, labels = eval_pred
    preds = np.argmax(logits, axis=-1)
    return accuracy.compute(predictions=preds, references=labels)

# 6. 训练参数
training_args = TrainingArguments(
    output_dir="./imdb-bert",
    num_train_epochs=3,
    per_device_train_batch_size=16,
    per_device_eval_batch_size=64,
    eval_strategy="epoch",
    save_strategy="epoch",
    learning_rate=2e-5,
    weight_decay=0.01,
    load_best_model_at_end=True,
    metric_for_best_model="accuracy",
    fp16=torch.cuda.is_available(),
)

# 7. 创建 Trainer 并训练
trainer = Trainer(
    model=model,
    args=training_args,
    train_dataset=tokenized["train"],
    eval_dataset=tokenized["test"],
    tokenizer=tokenizer,
    data_collator=DataCollatorWithPadding(tokenizer),
    compute_metrics=compute_metrics,
)

trainer.train()

# 8. 保存
trainer.save_model("./imdb-bert-final")
tokenizer.save_pretrained("./imdb-bert-final")
```

### 10.2 命名实体识别（NER）示例

```python
from transformers import (
    AutoTokenizer,
    AutoModelForTokenClassification,
    TrainingArguments,
    Trainer,
    DataCollatorForTokenClassification,
)
from datasets import load_dataset
import evaluate
import numpy as np

# 加载数据
dataset = load_dataset("conll2003")
label_names = dataset["train"].features["ner_tags"].feature.names

# 加载模型
tokenizer = AutoTokenizer.from_pretrained("bert-base-uncased")
model = AutoModelForTokenClassification.from_pretrained(
    "bert-base-uncased",
    num_labels=len(label_names),
)

# 对齐 token 标签（subword tokenization 需要对齐）
def tokenize_and_align(examples):
    tokenized = tokenizer(
        examples["tokens"],
        truncation=True,
        is_split_into_words=True,
    )
    all_labels = []
    for i, labels in enumerate(examples["ner_tags"]):
        word_ids = tokenized.word_ids(batch_index=i)
        label_ids = []
        prev_word_id = None
        for word_id in word_ids:
            if word_id is None:
                label_ids.append(-100)           # 特殊 token → 忽略
            elif word_id != prev_word_id:
                label_ids.append(labels[word_id]) # 首个 subword → 保留标签
            else:
                label_ids.append(-100)           # 后续 subword → 忽略
            prev_word_id = word_id
        all_labels.append(label_ids)
    tokenized["labels"] = all_labels
    return tokenized

tokenized = dataset.map(tokenize_and_align, batched=True)

# 评估
seqeval = evaluate.load("seqeval")

def compute_metrics(eval_pred):
    logits, labels = eval_pred
    preds = np.argmax(logits, axis=-1)
    true_labels = [
        [label_names[l] for l, p in zip(label, pred) if l != -100]
        for label, pred in zip(labels, preds)
    ]
    true_preds = [
        [label_names[p] for l, p in zip(label, pred) if l != -100]
        for label, pred in zip(labels, preds)
    ]
    results = seqeval.compute(predictions=true_preds, references=true_labels)
    return {"precision": results["overall_precision"],
            "recall": results["overall_recall"],
            "f1": results["overall_f1"]}

# 训练
trainer = Trainer(
    model=model,
    args=TrainingArguments(
        output_dir="./ner-bert",
        num_train_epochs=3,
        per_device_train_batch_size=16,
        eval_strategy="epoch",
        learning_rate=2e-5,
    ),
    train_dataset=tokenized["train"],
    eval_dataset=tokenized["validation"],
    tokenizer=tokenizer,
    data_collator=DataCollatorForTokenClassification(tokenizer),
    compute_metrics=compute_metrics,
)
trainer.train()
```

### 10.3 不使用 Trainer 的原生 PyTorch 训练

```python
import torch
from torch.utils.data import DataLoader
from torch.optim import AdamW
from transformers import (
    AutoTokenizer,
    AutoModelForSequenceClassification,
    get_linear_schedule_with_warmup,
)

# 准备
tokenizer = AutoTokenizer.from_pretrained("bert-base-uncased")
model = AutoModelForSequenceClassification.from_pretrained("bert-base-uncased", num_labels=2)
model.to("cuda")

# DataLoader
train_loader = DataLoader(train_dataset, batch_size=16, shuffle=True)

# 优化器 & 调度器
optimizer = AdamW(model.parameters(), lr=2e-5, weight_decay=0.01)
num_training_steps = len(train_loader) * 3  # 3 epochs
scheduler = get_linear_schedule_with_warmup(
    optimizer,
    num_warmup_steps=int(0.1 * num_training_steps),
    num_training_steps=num_training_steps,
)

# 训练循环
model.train()
for epoch in range(3):
    total_loss = 0
    for batch in train_loader:
        batch = {k: v.to("cuda") for k, v in batch.items()}
        outputs = model(**batch)
        loss = outputs.loss

        loss.backward()
        torch.nn.utils.clip_grad_norm_(model.parameters(), max_norm=1.0)
        optimizer.step()
        scheduler.step()
        optimizer.zero_grad()

        total_loss += loss.item()

    avg_loss = total_loss / len(train_loader)
    print(f"Epoch {epoch+1}, Loss: {avg_loss:.4f}")
```

---

## 11. 生成（Generation）

### 11.1 基本生成

```python
from transformers import AutoTokenizer, AutoModelForCausalLM

tokenizer = AutoTokenizer.from_pretrained("gpt2")
model = AutoModelForCausalLM.from_pretrained("gpt2")

inputs = tokenizer("The future of AI is", return_tensors="pt")

# 基本生成
output_ids = model.generate(**inputs, max_new_tokens=50)
print(tokenizer.decode(output_ids[0], skip_special_tokens=True))
```

### 11.2 生成策略详解

```python
# === 贪心搜索（Greedy） ===
output = model.generate(
    **inputs,
    max_new_tokens=50,
    do_sample=False,              # 默认关闭采样 → 贪心
)

# === Beam Search ===
output = model.generate(
    **inputs,
    max_new_tokens=50,
    num_beams=5,                  # beam 数量
    early_stopping=True,          # 所有 beam 结束时停止
    no_repeat_ngram_size=2,       # 禁止 2-gram 重复
)

# === 采样（Sampling） ===
output = model.generate(
    **inputs,
    max_new_tokens=50,
    do_sample=True,
    temperature=0.7,              # 温度（越低越确定）
    top_k=50,                     # Top-K 采样
    top_p=0.9,                    # Top-P / Nucleus 采样
)

# === 对比搜索（Contrastive Search） ===
output = model.generate(
    **inputs,
    max_new_tokens=50,
    penalty_alpha=0.6,
    top_k=4,
)
```

### 11.3 `generate()` 常用参数

| 参数 | 类型 | 说明 |
|------|------|------|
| `max_new_tokens` | int | 最大生成 token 数 |
| `max_length` | int | 总长度上限（含输入） |
| `min_length` | int | 最小生成长度 |
| `do_sample` | bool | 是否使用采样 |
| `temperature` | float | 采样温度（0-2） |
| `top_k` | int | Top-K 采样 |
| `top_p` | float | Top-P 采样 |
| `num_beams` | int | Beam Search 宽度 |
| `repetition_penalty` | float | 重复惩罚（>1.0） |
| `no_repeat_ngram_size` | int | 禁止 N-gram 重复 |
| `early_stopping` | bool | Beam Search 早停 |
| `num_return_sequences` | int | 返回的序列数 |
| `pad_token_id` | int | 填充 token ID |
| `eos_token_id` | int | 结束 token ID |
| `bad_words_ids` | list | 禁止生成的 token 列表 |
| `forced_eos_token_id` | int | 强制结束 token |
| `diversity_penalty` | float | beam 多样性惩罚 |
| `length_penalty` | float | 长度惩罚（>1 偏长，<1 偏短） |

### 11.4 流式生成（Streaming）

```python
from transformers import AutoTokenizer, AutoModelForCausalLM, TextStreamer, TextIteratorStreamer
from threading import Thread

tokenizer = AutoTokenizer.from_pretrained("gpt2")
model = AutoModelForCausalLM.from_pretrained("gpt2")

inputs = tokenizer("Once upon a time", return_tensors="pt")

# 方式 1：TextStreamer（自动打印到控制台）
streamer = TextStreamer(tokenizer, skip_special_tokens=True)
model.generate(**inputs, max_new_tokens=100, streamer=streamer)

# 方式 2：TextIteratorStreamer（可迭代，适合 Web 服务）
streamer = TextIteratorStreamer(tokenizer, skip_special_tokens=True)

thread = Thread(target=model.generate, kwargs={**inputs, "max_new_tokens": 100, "streamer": streamer})
thread.start()

for text in streamer:
    print(text, end="", flush=True)

thread.join()
```

### 11.5 Seq2Seq 生成（翻译/摘要）

```python
from transformers import AutoTokenizer, AutoModelForSeq2SeqLM

# 翻译示例
tokenizer = AutoTokenizer.from_pretrained("Helsinki-NLP/opus-mt-en-zh")
model = AutoModelForSeq2SeqLM.from_pretrained("Helsinki-NLP/opus-mt-en-zh")

text = "Transformers is an amazing library."
inputs = tokenizer(text, return_tensors="pt")
output = model.generate(**inputs, max_new_tokens=128)
print(tokenizer.decode(output[0], skip_special_tokens=True))

# 摘要示例
tokenizer = AutoTokenizer.from_pretrained("facebook/bart-large-cnn")
model = AutoModelForSeq2SeqLM.from_pretrained("facebook/bart-large-cnn")

article = "Long article text here..."
inputs = tokenizer(article, return_tensors="pt", max_length=1024, truncation=True)
summary_ids = model.generate(
    **inputs,
    max_new_tokens=150,
    min_length=40,
    length_penalty=2.0,
    num_beams=4,
)
print(tokenizer.decode(summary_ids[0], skip_special_tokens=True))
```

---

## 12. 模型量化与优化

### 12.1 使用 `bitsandbytes` 量化

```python
from transformers import AutoModelForCausalLM, AutoTokenizer, BitsAndBytesConfig

# 8-bit 量化
model_8bit = AutoModelForCausalLM.from_pretrained(
    "meta-llama/Llama-2-7b-hf",
    load_in_8bit=True,
    device_map="auto",
)

# 4-bit 量化（更激进的压缩）
bnb_config = BitsAndBytesConfig(
    load_in_4bit=True,
    bnb_4bit_quant_type="nf4",              # NormalFloat4 量化
    bnb_4bit_compute_dtype=torch.float16,    # 计算精度
    bnb_4bit_use_double_quant=True,          # 双重量化（进一步压缩）
)

model_4bit = AutoModelForCausalLM.from_pretrained(
    "meta-llama/Llama-2-7b-hf",
    quantization_config=bnb_config,
    device_map="auto",
)
```

### 12.2 GPTQ 量化

```python
from transformers import AutoModelForCausalLM, GPTQConfig

# 加载 GPTQ 预量化模型
model = AutoModelForCausalLM.from_pretrained(
    "TheBloke/Llama-2-7B-GPTQ",
    device_map="auto",
)

# 手动 GPTQ 量化
gptq_config = GPTQConfig(
    bits=4,
    dataset="c4",
    tokenizer=tokenizer,
)
model = AutoModelForCausalLM.from_pretrained(
    "meta-llama/Llama-2-7b-hf",
    quantization_config=gptq_config,
    device_map="auto",
)
```

### 12.3 AWQ 量化

```python
from transformers import AutoModelForCausalLM, AwqConfig

awq_config = AwqConfig(
    bits=4,
    fuse_max_seq_len=512,
    do_fuse=True,
)

model = AutoModelForCausalLM.from_pretrained(
    "TheBloke/Llama-2-7B-AWQ",
    quantization_config=awq_config,
    device_map="auto",
)
```

### 12.4 `device_map` 使用

```python
# 自动分配到所有可用 GPU
model = AutoModelForCausalLM.from_pretrained("bigmodel", device_map="auto")

# 全部放在 GPU 0
model = AutoModelForCausalLM.from_pretrained("bigmodel", device_map={"": 0})

# 自定义层到设备映射
device_map = {
    "model.embed_tokens": 0,
    "model.layers.0": 0,
    "model.layers.1": 0,
    "model.layers.2": 1,
    "model.layers.3": 1,
    "model.norm": 1,
    "lm_head": 1,
}
model = AutoModelForCausalLM.from_pretrained("bigmodel", device_map=device_map)

# 查看当前分配
print(model.hf_device_map)
```

### 12.5 混合精度推理

```python
import torch

# float16（NVIDIA GPU）
model = AutoModelForCausalLM.from_pretrained("gpt2", torch_dtype=torch.float16)

# bfloat16（A100, H100 等）
model = AutoModelForCausalLM.from_pretrained("gpt2", torch_dtype=torch.bfloat16)

# 自动选择最佳精度
model = AutoModelForCausalLM.from_pretrained("gpt2", torch_dtype="auto")
```

---

## 13. PEFT 与 LoRA 微调

PEFT（Parameter-Efficient Fine-Tuning）通过只训练少量额外参数来实现高效微调。

### 13.1 安装

```bash
pip install peft
```

### 13.2 LoRA 配置与应用

```python
from peft import LoraConfig, get_peft_model, TaskType

# 配置 LoRA
lora_config = LoraConfig(
    task_type=TaskType.CAUSAL_LM,        # 任务类型
    r=8,                                  # LoRA 秩
    lora_alpha=32,                        # LoRA 缩放因子
    lora_dropout=0.1,                     # LoRA Dropout
    target_modules=["q_proj", "v_proj"],  # 注入 LoRA 的模块
    bias="none",                          # 偏置处理方式
)

# 将 LoRA 应用到模型
model = AutoModelForCausalLM.from_pretrained("meta-llama/Llama-2-7b-hf", torch_dtype=torch.float16)
peft_model = get_peft_model(model, lora_config)

# 查看可训练参数
peft_model.print_trainable_parameters()
# Output: trainable params: 4,194,304 || all params: 6,742,609,920 || trainable%: 0.0622
```

### 13.3 常用 TaskType

| TaskType | 用途 |
|----------|------|
| `TaskType.CAUSAL_LM` | 因果语言模型（GPT, LLaMA） |
| `TaskType.SEQ_2_SEQ_LM` | 编解码模型（T5, BART） |
| `TaskType.SEQ_CLS` | 文本分类 |
| `TaskType.TOKEN_CLS` | Token 分类（NER） |
| `TaskType.QUESTION_ANS` | 问答 |
| `TaskType.FEATURE_EXTRACTION` | 特征提取 |

### 13.4 LoRA 常用 `target_modules`

| 模型 | 常用 target_modules |
|------|---------------------|
| LLaMA / Mistral | `["q_proj", "k_proj", "v_proj", "o_proj"]` |
| BERT | `["query", "value"]` |
| GPT-2 | `["c_attn"]` |
| T5 | `["q", "v"]` |
| Bloom | `["query_key_value"]` |

### 13.5 QLoRA（量化 + LoRA）

```python
import torch
from transformers import AutoModelForCausalLM, BitsAndBytesConfig
from peft import LoraConfig, get_peft_model, prepare_model_for_kbit_training

# 4-bit 量化加载
bnb_config = BitsAndBytesConfig(
    load_in_4bit=True,
    bnb_4bit_quant_type="nf4",
    bnb_4bit_compute_dtype=torch.float16,
    bnb_4bit_use_double_quant=True,
)

model = AutoModelForCausalLM.from_pretrained(
    "meta-llama/Llama-2-7b-hf",
    quantization_config=bnb_config,
    device_map="auto",
)

# 准备量化模型用于训练
model = prepare_model_for_kbit_training(model)

# 应用 LoRA
lora_config = LoraConfig(
    r=16,
    lora_alpha=32,
    lora_dropout=0.05,
    target_modules=["q_proj", "v_proj", "k_proj", "o_proj"],
    task_type=TaskType.CAUSAL_LM,
)
model = get_peft_model(model, lora_config)
model.print_trainable_parameters()
```

### 13.6 保存与加载 LoRA 权重

```python
# 保存（只保存 LoRA 增量权重，非常小）
peft_model.save_pretrained("./lora-weights")

# 加载
from peft import PeftModel

base_model = AutoModelForCausalLM.from_pretrained("meta-llama/Llama-2-7b-hf")
model = PeftModel.from_pretrained(base_model, "./lora-weights")

# 合并 LoRA 权重到基础模型（部署用）
merged_model = model.merge_and_unload()
merged_model.save_pretrained("./merged-model")
```

---

## 14. 多模态模型

### 14.1 CLIP（文图匹配）

```python
from transformers import CLIPProcessor, CLIPModel
from PIL import Image

model = CLIPModel.from_pretrained("openai/clip-vit-base-patch32")
processor = CLIPProcessor.from_pretrained("openai/clip-vit-base-patch32")

image = Image.open("cat.jpg")
texts = ["a photo of a cat", "a photo of a dog", "a photo of a bird"]

inputs = processor(text=texts, images=image, return_tensors="pt", padding=True)
outputs = model(**inputs)

# 计算相似度
logits_per_image = outputs.logits_per_image   # shape: (1, 3)
probs = logits_per_image.softmax(dim=-1)
print(probs)
# Output: tensor([[0.95, 0.03, 0.02]])
```

### 14.2 视觉问答（VQA）

```python
from transformers import pipeline
from PIL import Image

vqa = pipeline("visual-question-answering", model="dandelin/vilt-b32-finetuned-vqa")

image = Image.open("photo.jpg")
result = vqa(image=image, question="What color is the car?")
print(result)
# Output: [{'score': 0.95, 'answer': 'red'}]
```

### 14.3 图像描述

```python
from transformers import pipeline

captioner = pipeline("image-to-text", model="Salesforce/blip-image-captioning-base")
result = captioner("photo.jpg")
print(result)
# Output: [{'generated_text': 'a cat sitting on a couch'}]
```

### 14.4 Whisper（语音识别）

```python
from transformers import pipeline

# 基本使用
asr = pipeline(
    "automatic-speech-recognition",
    model="openai/whisper-base",
    device=0,
)

result = asr("audio.mp3")
print(result["text"])

# 带时间戳
result = asr("audio.mp3", return_timestamps=True)
for chunk in result["chunks"]:
    print(f"[{chunk['timestamp'][0]:.1f}s - {chunk['timestamp'][1]:.1f}s] {chunk['text']}")

# 指定语言
result = asr("chinese_audio.mp3", generate_kwargs={"language": "zh"})
```

### 14.5 LLaVA / 多模态 LLM

```python
from transformers import AutoProcessor, LlavaForConditionalGeneration
from PIL import Image

model = LlavaForConditionalGeneration.from_pretrained(
    "llava-hf/llava-1.5-7b-hf",
    torch_dtype=torch.float16,
    device_map="auto",
)
processor = AutoProcessor.from_pretrained("llava-hf/llava-1.5-7b-hf")

image = Image.open("photo.jpg")
prompt = "USER: <image>\nDescribe this image in detail.\nASSISTANT:"

inputs = processor(text=prompt, images=image, return_tensors="pt").to("cuda")
output = model.generate(**inputs, max_new_tokens=200)
print(processor.decode(output[0], skip_special_tokens=True))
```

---

## 15. 模型保存、加载与分享

### 15.1 保存模型

```python
# 保存模型 + 分词器 + 配置
model.save_pretrained("./my_model/")
tokenizer.save_pretrained("./my_model/")

# 保存后的文件结构：
# ./my_model/
# ├── config.json              # 模型配置
# ├── model.safetensors        # 权重（safetensors 格式，推荐）
# ├── tokenizer.json           # 分词器
# ├── tokenizer_config.json    # 分词器配置
# ├── special_tokens_map.json  # 特殊 token 映射
# └── vocab.txt                # 词汇表（BERT 系列）
```

### 15.2 从本地加载

```python
model = AutoModelForSequenceClassification.from_pretrained("./my_model/")
tokenizer = AutoTokenizer.from_pretrained("./my_model/")
```

### 15.3 推送到 Hugging Face Hub

```python
# 方式 1：通过 CLI 登录
# 在终端执行：huggingface-cli login

# 方式 2：代码中登录
from huggingface_hub import login
login(token="hf_xxxxxxxxxxxx")

# 推送模型
model.push_to_hub("my-username/my-model")
tokenizer.push_to_hub("my-username/my-model")

# 推送时设为私有
model.push_to_hub("my-username/my-model", private=True)

# 使用 Trainer 训练后直接推送
training_args = TrainingArguments(
    output_dir="./results",
    push_to_hub=True,
    hub_model_id="my-username/my-model",
)
trainer.push_to_hub()
```

### 15.4 SafeTensors 格式

```python
# 保存为 safetensors（默认格式，更安全更快）
model.save_pretrained("./my_model/", safe_serialization=True)

# 保存为传统 PyTorch .bin 格式
model.save_pretrained("./my_model/", safe_serialization=False)
```

---

## 16. 常用工具函数

### 16.1 学习率调度器

```python
from transformers import get_scheduler

scheduler = get_scheduler(
    name="cosine",
    optimizer=optimizer,
    num_warmup_steps=100,
    num_training_steps=1000,
)

# 支持的调度器名称：
# "linear", "cosine", "cosine_with_restarts",
# "polynomial", "constant", "constant_with_warmup",
# "inverse_sqrt", "reduce_on_plateau"
```

### 16.2 模型信息工具

```python
from transformers import AutoModel

model = AutoModel.from_pretrained("bert-base-uncased")

# 查看模型参数量
print(model.num_parameters())                    # 总参数
print(model.num_parameters(only_trainable=True))  # 可训练参数
```

### 16.3 日志设置

```python
import transformers

# 设置日志级别
transformers.logging.set_verbosity_info()      # INFO 级别
transformers.logging.set_verbosity_warning()   # WARNING 级别
transformers.logging.set_verbosity_error()     # ERROR 级别
transformers.logging.set_verbosity_debug()     # DEBUG 级别

# 禁用进度条
transformers.logging.disable_progress_bar()

# 启用进度条
transformers.logging.enable_progress_bar()
```

### 16.4 设置随机种子

```python
from transformers import set_seed

set_seed(42)  # 同时设置 Python、NumPy、PyTorch 的随机种子
```

### 16.5 Accelerate 多卡训练

```python
# 使用 accelerate 启动脚本
# accelerate launch train.py

from accelerate import Accelerator

accelerator = Accelerator()

model, optimizer, train_loader, scheduler = accelerator.prepare(
    model, optimizer, train_loader, scheduler
)

for batch in train_loader:
    outputs = model(**batch)
    loss = outputs.loss
    accelerator.backward(loss)
    optimizer.step()
    scheduler.step()
    optimizer.zero_grad()
```

### 16.6 FlashAttention 2

```python
# 需要安装：pip install flash-attn

model = AutoModelForCausalLM.from_pretrained(
    "meta-llama/Llama-2-7b-hf",
    torch_dtype=torch.float16,
    attn_implementation="flash_attention_2",  # 启用 FlashAttention 2
    device_map="auto",
)
```

### 16.7 模型并行（Pipeline Parallelism）

```python
# Accelerate 自动并行
from accelerate import infer_auto_device_map, dispatch_model

device_map = infer_auto_device_map(
    model,
    max_memory={0: "10GiB", 1: "10GiB", "cpu": "30GiB"},
)

model = dispatch_model(model, device_map=device_map)
```

---

## 17. 常见问题与排错

### 17.1 CUDA Out of Memory

**原因：** GPU 显存不足。

**解决方案：**

```python
# 方案 1：减小 batch size
per_device_train_batch_size=4

# 方案 2：使用梯度累积
gradient_accumulation_steps=8

# 方案 3：使用混合精度
fp16=True  # 或 bf16=True

# 方案 4：使用 8-bit / 4-bit 量化
model = AutoModelForCausalLM.from_pretrained("model", load_in_8bit=True)

# 方案 5：使用梯度检查点
model.gradient_checkpointing_enable()

# 方案 6：清除 GPU 缓存
torch.cuda.empty_cache()
```

### 17.2 Tokenizer 缺少 pad_token

**错误信息：** `ValueError: Asking to pad but the tokenizer does not have a padding token.`

**解决方案：**

```python
# 方案 1：设置 pad_token 为 eos_token
tokenizer.pad_token = tokenizer.eos_token
model.config.pad_token_id = tokenizer.eos_token_id

# 方案 2：添加新的 pad_token
tokenizer.add_special_tokens({"pad_token": "[PAD]"})
model.resize_token_embeddings(len(tokenizer))
```

### 17.3 模型下载失败

**解决方案：**

```python
# 方案 1：使用镜像
import os
os.environ["HF_ENDPOINT"] = "https://hf-mirror.com"

# 方案 2：离线模式（需要先下载好模型）
os.environ["TRANSFORMERS_OFFLINE"] = "1"
model = AutoModel.from_pretrained("./local_model/")

# 方案 3：手动下载
from huggingface_hub import snapshot_download
snapshot_download(repo_id="bert-base-uncased", local_dir="./bert/")
```

### 17.4 Token 数量不匹配

**错误信息：** `RuntimeError: The size of tensor a (30524) must match the size of tensor b (30522)`

**解决方案：**

```python
# 添加新 token 后需要调整嵌入层大小
tokenizer.add_special_tokens({"additional_special_tokens": ["<NEW1>", "<NEW2>"]})
model.resize_token_embeddings(len(tokenizer))
```

### 17.5 label 命名问题

**错误信息：** `model forward 未收到 labels`

**解决方案：**

```python
# Trainer 默认从 dataset 中提取名为 "labels" 的列
# 确保你的数据集使用 "labels" 而非 "label"
tokenized_dataset = tokenized_dataset.rename_column("label", "labels")
```

### 17.6 多 GPU 不均衡

```python
# 使用 device_map="balanced" 而非 "auto"
model = AutoModelForCausalLM.from_pretrained(
    "bigmodel",
    device_map="balanced",
    torch_dtype=torch.float16,
)
```

---

## 18. 速查表

### 18.1 Pipeline 一行式速查

```python
from transformers import pipeline

# 情感分析
pipeline("sentiment-analysis")("Great product!")

# 命名实体识别
pipeline("ner", grouped_entities=True)("My name is John and I live in New York.")

# 文本生成
pipeline("text-generation", model="gpt2")("AI will", max_new_tokens=30)

# 问答
pipeline("question-answering")(question="What is AI?", context="AI is artificial intelligence.")

# 翻译
pipeline("translation_en_to_fr")("Hello, how are you?")

# 摘要
pipeline("summarization")("Long article text here..." * 10)

# 零样本分类
pipeline("zero-shot-classification")("I love cooking", candidate_labels=["food", "sports", "tech"])

# 填空
pipeline("fill-mask")("The capital of France is [MASK].")

# 图像分类
pipeline("image-classification")("cat.jpg")

# 语音识别
pipeline("automatic-speech-recognition", model="openai/whisper-base")("audio.mp3")
```

### 18.2 模型加载速查

```python
from transformers import AutoTokenizer, AutoModelForCausalLM
import torch

# 标准加载
model = AutoModelForCausalLM.from_pretrained("model_name")

# FP16 加载
model = AutoModelForCausalLM.from_pretrained("model_name", torch_dtype=torch.float16)

# 多卡加载
model = AutoModelForCausalLM.from_pretrained("model_name", device_map="auto")

# 8-bit 量化
model = AutoModelForCausalLM.from_pretrained("model_name", load_in_8bit=True, device_map="auto")

# 4-bit 量化
model = AutoModelForCausalLM.from_pretrained("model_name", load_in_4bit=True, device_map="auto")

# FlashAttention 2
model = AutoModelForCausalLM.from_pretrained("model_name", attn_implementation="flash_attention_2")
```

### 18.3 常见模型名称速查

| 模型 | Hub 名称 | 类型 | 参数量 |
|------|---------|------|--------|
| BERT base | `bert-base-uncased` | Encoder | 110M |
| BERT large | `bert-large-uncased` | Encoder | 340M |
| RoBERTa | `roberta-base` | Encoder | 125M |
| GPT-2 | `gpt2` | Decoder | 117M |
| GPT-2 Large | `gpt2-large` | Decoder | 774M |
| T5 Small | `t5-small` | Enc-Dec | 60M |
| T5 Base | `t5-base` | Enc-Dec | 220M |
| BART | `facebook/bart-large` | Enc-Dec | 400M |
| LLaMA 2 7B | `meta-llama/Llama-2-7b-hf` | Decoder | 7B |
| Mistral 7B | `mistralai/Mistral-7B-v0.1` | Decoder | 7B |
| Whisper | `openai/whisper-base` | Enc-Dec | 74M |
| CLIP | `openai/clip-vit-base-patch32` | Multimodal | 151M |
| ViT | `google/vit-base-patch16-224` | Vision | 86M |

### 18.4 常用 `import` 清单

```python
# ——— 核心 ———
from transformers import (
    AutoConfig,
    AutoTokenizer,
    AutoModel,
    AutoModelForSequenceClassification,
    AutoModelForTokenClassification,
    AutoModelForQuestionAnswering,
    AutoModelForCausalLM,
    AutoModelForSeq2SeqLM,
    AutoModelForMaskedLM,
    AutoProcessor,
    AutoImageProcessor,
)

# ——— Pipeline ———
from transformers import pipeline

# ——— 训练 ———
from transformers import (
    Trainer,
    TrainingArguments,
    TrainerCallback,
    EarlyStoppingCallback,
)

# ——— 数据 ———
from transformers import (
    DataCollatorWithPadding,
    DataCollatorForLanguageModeling,
    DataCollatorForSeq2Seq,
    DataCollatorForTokenClassification,
)

# ——— 生成 ———
from transformers import (
    GenerationConfig,
    TextStreamer,
    TextIteratorStreamer,
    StoppingCriteria,
    StoppingCriteriaList,
)

# ——— 量化 ———
from transformers import (
    BitsAndBytesConfig,
    GPTQConfig,
    AwqConfig,
)

# ——— 工具 ———
from transformers import (
    set_seed,
    get_scheduler,
    get_linear_schedule_with_warmup,
    get_cosine_schedule_with_warmup,
)

# ——— PEFT ———
from peft import (
    LoraConfig,
    get_peft_model,
    PeftModel,
    TaskType,
    prepare_model_for_kbit_training,
)

# ——— 数据集与评估 ———
from datasets import load_dataset, Dataset, DatasetDict
import evaluate
```

---

> **文档版本：** v1.0  
> **适用 `transformers` 版本：** 4.30+  
> **更新说明：** 涵盖 Pipeline、Tokenizer、Model、Trainer、生成、量化、PEFT/LoRA、多模态等常用模块的完整用法。
