# Git提交前缀规范完整指南

------

## 常用前缀

| 前缀         | 中文     | 作用说明                 | 是否影响版本号 | 常用程度 |
| ------------ | -------- | ------------------------ | -------------- | -------- |
| **feat**     | 功能     | 添加新功能或新特性       | ✓ 影响         | ⭐⭐⭐⭐⭐    |
| **fix**      | 修复     | 修复已知的bug或问题      | ✓ 影响         | ⭐⭐⭐⭐⭐    |
| **docs**     | 文档     | 更新或添加文档内容       | ✗ 不影响       | ⭐⭐⭐⭐     |
| **style**    | 样式     | 代码格式调整（不改功能） | ✗ 不影响       | ⭐⭐⭐      |
| **refactor** | 重构     | 代码重构优化（不改功能） | ✗ 不影响       | ⭐⭐⭐⭐     |
| **perf**     | 性能     | 性能优化改进             | ✗ 不影响       | ⭐⭐⭐      |
| **test**     | 测试     | 添加或修改测试代码       | ✗ 不影响       | ⭐⭐⭐⭐     |
| **chore**    | 杂务     | 构建工具或依赖更新       | ✗ 不影响       | ⭐⭐⭐⭐     |
| **ci**       | 持续集成 | CI/CD配置文件修改        | ✗ 不影响       | ⭐⭐⭐      |
| **revert**   | 回滚     | 撤销之前的提交           | ✓ 影响         | ⭐⭐       |

------

## 前缀详细说明

### feat（功能）

| 项目         | 详情                              |
| ------------ | --------------------------------- |
| **中文名**   | 功能、特性                        |
| **定义**     | 添加新的业务功能或产品特性        |
| **使用场景** | 实现新需求、新模块、新API接口     |
| **示例**     | `feat(auth): 添加JWT身份验证机制` |
| **版本影响** | 会触发minor版本号升级（x.y+1.z）  |
| **是否必要** | 是                                |

### fix（修复）

| 项目         | 详情                                  |
| ------------ | ------------------------------------- |
| **中文名**   | 修复、补丁                            |
| **定义**     | 修复发现的bug或缺陷                   |
| **使用场景** | 解决功能问题、逻辑错误、崩溃等        |
| **示例**     | `fix(login): 修复登录页面CSS布局错误` |
| **版本影响** | 会触发patch版本号升级（x.y.z+1）      |
| **是否必要** | 是                                    |

### docs（文档）

| 项目         | 详情                                |
| ------------ | ----------------------------------- |
| **中文名**   | 文档                                |
| **定义**     | 添加或更新文档文件                  |
| **使用场景** | 更新README、API文档、使用说明、注释 |
| **示例**     | `docs(api): 补充API参数说明文档`    |
| **版本影响** | 不影响版本号                        |
| **是否必要** | 是                                  |

### style（代码风格）

| 项目         | 详情                                       |
| ------------ | ------------------------------------------ |
| **中文名**   | 风格、格式                                 |
| **定义**     | 代码格式和风格的调整                       |
| **使用场景** | 空格、缩进、分号、换行等（不改变代码逻辑） |
| **示例**     | `style: 调整代码缩进为2个空格`             |
| **版本影响** | 不影响版本号                               |
| **是否必要** | 否                                         |

### refactor（重构）

| 项目         | 详情                                      |
| ------------ | ----------------------------------------- |
| **中文名**   | 重构                                      |
| **定义**     | 重构代码以提高质量                        |
| **使用场景** | 优化代码结构、提高可读性、提取公共方法    |
| **示例**     | `refactor(utils): 提取重复代码到工具函数` |
| **版本影响** | 不影响版本号                              |
| **是否必要** | 否                                        |

### perf（性能）

| 项目         | 详情                                     |
| ------------ | ---------------------------------------- |
| **中文名**   | 性能                                     |
| **定义**     | 改进应用性能                             |
| **使用场景** | 优化算法、减少打包体积、加快加载速度     |
| **示例**     | `perf(image): 压缩图片资源减少包体积50%` |
| **版本影响** | 不影响版本号                             |
| **是否必要** | 否                                       |

### test（测试）

| 项目         | 详情                                     |
| ------------ | ---------------------------------------- |
| **中文名**   | 测试                                     |
| **定义**     | 添加或修改测试用例                       |
| **使用场景** | 单元测试、集成测试、E2E测试              |
| **示例**     | `test(auth): 添加登录功能的单元测试用例` |
| **版本影响** | 不影响版本号                             |
| **是否必要** | 否                                       |

### chore（杂务）

| 项目         | 详情                                   |
| ------------ | -------------------------------------- |
| **中文名**   | 杂务、构建                             |
| **定义**     | 更新构建工具和依赖                     |
| **使用场景** | 更新npm包版本、修改build脚本、升级工具 |
| **示例**     | `chore: 升级webpack到最新稳定版本`     |
| **版本影响** | 不影响版本号                           |
| **是否必要** | 否                                     |

### ci（持续集成）

| 项目         | 详情                                       |
| ------------ | ------------------------------------------ |
| **中文名**   | 持续集成                                   |
| **定义**     | 修改CI/CD配置                              |
| **使用场景** | GitHub Actions、Jenkins、GitLab CI配置修改 |
| **示例**     | `ci: 配置GitHub Actions自动部署流程`       |
| **版本影响** | 不影响版本号                               |
| **是否必要** | 否                                         |

### revert（回滚）

| 项目         | 详情                               |
| ------------ | ---------------------------------- |
| **中文名**   | 回滚、撤销                         |
| **定义**     | 撤销之前的某个提交                 |
| **使用场景** | 恢复已发布的功能、移除有问题的代码 |
| **示例**     | `revert: 撤销提交 abc1234`         |
| **版本影响** | 可能影响版本号                     |
| **是否必要** | 否                                 |

------

## 完整格式示例

### 基础格式

```
<type>[optional scope]: <description>
```

**示例：**

```
feat: 添加用户登录功能
fix: 修复登录页面样式错误
docs: 更新安装说明
```

### 带范围的格式（推荐）

```
<type>(<scope>): <description>
```

**示例：**

```
feat(auth): 添加用户登录功能
fix(ui): 修复登录页面样式错误
docs(readme): 更新安装说明
refactor(utils): 优化时间处理函数
test(api): 添加用户API测试
```

### 完整格式（含详细说明）

```
<type>(<scope>): <description>

<body>

<footer>
```

**示例：**

```
feat(auth): 实现JWT身份验证机制

实现用户登录和令牌生成的完整流程：
- 添加/api/login端点用于用户认证
- 集成jsonwebtoken库处理JWT生成
- 实现auth中间件验证请求令牌
- 添加token自动刷新机制
- 完成相关单元测试覆盖率达95%

Closes #123
Refs #456
Breaking change: 已废弃之前的session认证方式，所有客户端需升级
```

------

## 实际使用示例

### feat（功能）提交示例

```bash
git commit -m "feat(user): 添加用户个人资料页面"
git commit -m "feat(payment): 集成支付宝和微信支付接口"
git commit -m "feat(search): 实现全文搜索功能"
git commit -m "feat(export): 支持导出Excel报表"
```

### fix（修复）提交示例

```bash
git commit -m "fix(login): 修复登录验证bug"
git commit -m "fix(modal): 修复弹窗关闭时的动画卡顿"
git commit -m "fix(form): 修复表单验证逻辑错误"
git commit -m "fix(api): 修复API返回数据格式不一致"
```

### docs（文档）提交示例

```bash
git commit -m "docs: 更新API使用说明文档"
git commit -m "docs(setup): 补充开发环境配置步骤"
git commit -m "docs(changelog): 生成2024年1月版本变更日志"
git commit -m "docs(faq): 添加常见问题解答"
```

### style（代码风格）提交示例

```bash
git commit -m "style: 调整代码缩进为2个空格"
git commit -m "style(css): 统一样式表格式并运行prettier"
git commit -m "style: 移除未使用的import语句"
```

### refactor（重构）提交示例

```bash
git commit -m "refactor(api): 重构数据请求逻辑增强可读性"
git commit -m "refactor(utils): 提取公共方法到工具类"
git commit -m "refactor(component): 将大组件拆分为小组件"
```

### perf（性能）提交示例

```bash
git commit -m "perf: 优化列表渲染性能使用虚拟滚动"
git commit -m "perf(image): 压缩图片资源减少包体积30%"
git commit -m "perf(api): 添加请求缓存减少API调用"
```

### test（测试）提交示例

```bash
git commit -m "test: 添加登录功能的单元测试"
git commit -m "test(user): 增加用户API集成测试用例"
git commit -m "test(e2e): 添加购物流程端到端测试"
```

### chore（杂务）提交示例

```bash
git commit -m "chore: 升级依赖包到最新版本"
git commit -m "chore(deps): 更新webpack配置优化构建"
git commit -m "chore: 清理冗余代码和注释"
```

### ci（持续集成）提交示例

```bash
git commit -m "ci: 配置GitHub Actions自动部署流程"
git commit -m "ci(travis): 修复CI构建失败问题"
git commit -m "ci: 添加代码覆盖率检查步骤"
```

### revert（回滚）提交示例

```bash
git commit -m "revert: 撤销提交 abc1234"
git commit -m "revert(auth): 移除有问题的OAuth2实现"
```

------

## 进阶前缀

| 前缀        | 中文     | 作用                 | 使用场景                          | 示例                          |
| ----------- | -------- | -------------------- | --------------------------------- | ----------------------------- |
| **build**   | 构建     | 编译构建系统修改     | 修改webpack、rollup等构建工具配置 | `build: 升级webpack到v5版本`  |
| **deps**    | 依赖     | 依赖包版本更新       | 更新npm、yarn依赖版本             | `deps: 更新React到18.0`       |
| **sec**     | 安全     | 安全漏洞修复         | 修复XSS、CSRF等安全问题           | `sec: 修复登录页面XSS漏洞`    |
| **wip**     | 进行中   | 工作进行中（未完成） | 功能开发中，不应merge到main       | `wip: 开发支付功能（进行中）` |
| **hotfix**  | 紧急修复 | 生产环境紧急修复     | 修复严重的生产bug                 | `hotfix: 修复订单系统崩溃`    |
| **release** | 发布     | 发布新版本           | 标记版本发布点                    | `release: 发布v1.2.0版本`     |

------

## 最佳实践

### ✅ 推荐做法

#### 1. 使用明确的前缀和范围

```bash
# ✓ 好
git commit -m "feat(payment): 添加微信支付功能"
git commit -m "fix(ui): 修复弹窗样式错误"

# ✗ 不好
git commit -m "feat: 修改文件"
git commit -m "update something"
```

#### 2. 使用祈使句形式（动词原形）

```bash
# ✓ 好
git commit -m "feat: add user authentication"
git commit -m "fix: correct typo in config"
git commit -m "refactor: optimize database queries"

# ✗ 不好
git commit -m "feat: added user authentication"
git commit -m "fix: corrected typo in config"
git commit -m "refactor: optimized database queries"
```

#### 3. 小写开头且不加句号

```bash
# ✓ 好
git commit -m "feat: implement oauth2 login"

# ✗ 不好
git commit -m "Feat: implement oauth2 login."
git commit -m "FEAT: Implement oauth2 login"
```

#### 4. 简洁清晰的描述（50字以内）

```bash
# ✓ 好
git commit -m "feat(api): add user profile endpoint"

# ✗ 不好
git commit -m "feat: 添加了用户资料API端点接口，支持获取和更新用户的个人信息，包括头像、昵称、个人签名等字段"
```

#### 5. 详细提交（用于重要功能）

```bash
git commit -m "feat(auth): 实现OAuth2认证流程

实现OAuth2标准的用户认证机制：
- 支持Google、GitHub、微信登录
- 自动刷新access token
- 添加安全令牌存储机制
- 实现授权验证中间件

Closes #456
Refs #123, #789
Breaking change: 移除旧的密码认证API，客户端需升级"
```

### ❌ 避免做法

#### 1. 过于简洁，无法理解

```bash
# ✗ 不好
git commit -m "fix: bug"
git commit -m "feat: some changes"
git commit -m "update: fix"
```

#### 2. 不使用前缀或前缀不清楚

```bash
# ✗ 不好
git commit -m "修复bug"
git commit -m "update: some changes"
git commit -m "fix/update/modify the code"
```

#### 3. 多个无关功能在一个提交里

```bash
# ✗ 不好
git commit -m "feat: fixed login, added new button, updated docs, refactored utils"

# ✓ 好 - 分成多次提交
git commit -m "fix(login): correct authentication logic"
git commit -m "feat(ui): add new action button"
git commit -m "docs: update readme"
git commit -m "refactor(utils): optimize helper functions"
```

#### 4. 没有范围，无法快速定位

```bash
# ✗ 不好
git commit -m "fix: 修复问题"

# ✓ 好
git commit -m "fix(form): 修复表单验证错误"
git commit -m "fix(api): 修复API超时问题"
```

------

## 工具推荐

### 1. Commitizen - 交互式提交工具

#### 安装

```bash
npm install -g commitizen
```

#### 项目初始化

```bash
commitizen init cz-conventional-changelog --save-dev --save-exact
```

#### 使用方式

```bash
# 方式1：全局使用
git cz

# 方式2：项目使用
npx cz

# 方式3：npm脚本
npm run commit
```

#### package.json配置

```json
{
  "scripts": {
    "commit": "cz"
  },
  "devDependencies": {
    "commitizen": "^4.3.0",
    "cz-conventional-changelog": "^3.3.0"
  }
}
```

### 2. Husky + Commitlint - 自动化验证

#### 安装

```bash
npm install husky @commitlint/config-conventional @commitlint/cli --save-dev

npx husky install

npx husky add .husky/commit-msg 'npx --no -- commitlint --edit "$1"'
```

#### commitlint.config.js配置

```javascript
module.exports = {
  extends: ['@commitlint/config-conventional'],
  rules: {
    'type-enum': [
      2,
      'always',
      [
        'feat',
        'fix',
        'docs',
        'style',
        'refactor',
        'perf',
        'test',
        'chore',
        'ci',
        'revert',
        'build',
        'sec'
      ]
    ],
    'type-case': [2, 'always', 'lower-case'],
    'subject-case': [2, 'never', ['start-case', 'pascal-case', 'upper-case']],
    'subject-empty': [2, 'never'],
    'subject-full-stop': [2, 'never', '.'],
    'body-leading-blank': [1, 'always'],
    'footer-leading-blank': [1, 'always']
  }
};
```

### 3. 完整的.husky配置示例

#### 初始化husky

```bash
npx husky install
```

#### 添加pre-commit钩子

```bash
npx husky add .husky/pre-commit "npm run lint"
```

#### 添加commit-msg钩子

```bash
npx husky add .husky/commit-msg "npx --no -- commitlint --edit $1"
```

#### .husky/commit-msg文件内容

```bash
#!/bin/sh
. "$(dirname "$0")/_/husky.sh"

npx --no -- commitlint --edit $1
```

### 4. VSCode扩展推荐

| 扩展名                          | 功能                 | 下载                     |
| ------------------------------- | -------------------- | ------------------------ |
| **Conventional Commits**        | 提供提交前缀智能提示 | 在VSCode扩展市场搜索安装 |
| **Git Commit Message Template** | 提交信息模板         | 在VSCode扩展市场搜索安装 |
| **Git Graph**                   | 可视化git历史        | 在VSCode扩展市场搜索安装 |

------

## 提交历史查看

### 按类型查看提交

```bash
# 查看所有feat提交
git log --oneline --grep="^feat"

# 查看所有fix提交
git log --oneline --grep="^fix"

# 查看所有docs提交
git log --oneline --grep="^docs"

# 查看特定范围内的提交
git log --oneline v1.0.0..v2.0.0 | grep "^[a-f0-9]* feat"
```

### 统计提交数量

```bash
# 统计各类型提交数
git log --oneline | grep -oE "^[a-f0-9]* [a-z]+" | sort | uniq -c

# 统计作者提交数
git log --oneline --author="user@example.com" | wc -l

# 统计最近7天的提交
git log --oneline --since="7 days ago" | wc -l
```

### 自定义日志格式

```bash
# 显示提交类型和作者
git log --format="%h %s (%an)" --oneline

# 显示详细信息
git log --format="%H %an %ae %ad %s" --date=short

# 按提交类型分组
git log --oneline | awk -F':' '{print $1}' | sort | uniq -c
```

------

## 常见问题

### Q1: 为什么要使用提交前缀?

**A:**

- 快速理解提交的目的和类型
- 便于自动化工具生成变更日志
- 改进代码审查效率
- 便于查询和统计提交历史

### Q2: feat和feature的区别?

**A:** 在Conventional Commits规范中，只有`feat`，没有`feature`。请统一使用`feat`。

### Q3: 一个提交中有多种改动如何命名?

**A:** 应该分开多次提交，每次提交只做一种改动，这样便于回滚和理解。

### Q4: 如何修改已经push的提交信息?

**A:**

```bash
# 修改最后一次提交
git commit --amend -m "feat: new message"
git push --force-with-lease

# 仅修改提交信息不修改内容
git commit --amend --no-edit
```

### Q5: Breaking changes如何标记?

**A:**

```bash
git commit -m "feat!: 移除用户登录API

Breaking change: 所有客户端需升级至新API版本"
```

------

## 参考资源

- [Conventional Commits 官方文档](https://www.conventionalcommits.org/zh-hans/)
- [Angular 提交规范](https://github.com/angular/angular/blob/main/CONTRIBUTING.md)
- [Commitizen 官方文档](http://commitizen.github.io/cz-cli/)
- [Commitlint 官方文档](https://commitlint.js.org/)

------

*最后更新: 2024年 | 版本: 1.0*