# GitHub 最小操作手册

本文档面向这个仓库的日常维护，目标是用最少概念覆盖最常见工作：

- 拉取最新代码
- 提交本地修改
- 推送到 GitHub
- 管理分支
- 打版本标签
- 创建 GitHub Release

## 1. 基本概念

### 1.1 Git 是什么

Git 是本地版本管理工具。它负责记录代码历史，支持回退、分支开发、合并等操作。

### 1.2 GitHub 是什么

GitHub 是远端托管平台。它存放仓库副本，提供网页浏览、协作、Issue、Pull Request、Release 等功能。

### 1.3 本地仓库和远端仓库

- 本地仓库：你电脑里的项目目录
- 远端仓库：GitHub 上的仓库

典型关系：

1. 在本地修改代码
2. 用 Git 提交为一个 commit
3. 推送到 GitHub

### 1.4 commit 是什么

commit 可以理解为一次可追踪的版本快照。每个 commit 都应该表达一件明确的事情，例如：

- 修复 OpenAI SSE 解析
- 修复 `mcp_server_manager_demo` 的流式显示
- 删除不再需要的分支

### 1.5 branch 是什么

branch 是分支，用来并行开发。

常见做法：

- `main`：主分支，保持相对稳定
- `feature/...` 或 `codex/...`：功能或修复分支

推荐流程：

1. 从 `main` 拉出新分支
2. 在分支上开发
3. 合并回 `main`
4. 删除已合并分支

### 1.6 tag 是什么

tag 是给某个 commit 打上的固定版本标记，常用于版本发布。

例如：

- `v0.1.0`
- `v0.1.1`
- `v1.0.0`

### 1.7 Release 是什么

GitHub Release 是 GitHub 基于 tag 做的一层发布页面展示。

一个 Release 通常包括：

- 一个 tag
- 一个标题
- 一份 release notes
- 可选的附件，例如压缩包、可执行文件、安装包

关系可以简单理解为：

- tag：Git 里的版本锚点
- Release：GitHub 页面上的版本发布说明

## 2. 这个仓库的当前状态

截至当前检查，这个仓库的发布环境已经可用：

- 远端仓库：`origin = https://github.com/joe-ieta/qt-llmlite.git`
- GitHub CLI：`gh version 2.88.1`
- GitHub CLI 已登录：账号 `joe-ieta`
- Git 协议：`https`
- Token scope：包含 `repo`
- 已发布版本：`v0.1.0`

这意味着后续可以直接使用 `gh` 做 release 管理，不需要每次手工打开网页。

## 3. 最常用的命令

下面这些命令已经足够覆盖大多数日常维护。

### 3.1 查看状态

```powershell
git status
```

作用：

- 查看当前分支
- 查看有哪些未提交改动
- 判断工作区是否干净

### 3.2 拉取最新代码

```powershell
git pull
```

作用：

- 从远端获取最新提交
- 同步本地 `main`

### 3.3 查看分支

```powershell
git branch
git branch -a
```

作用：

- `git branch`：看本地分支
- `git branch -a`：看本地和远端分支

### 3.4 新建并切换分支

```powershell
git switch -c codex/my-change
```

作用：

- 从当前分支创建新分支
- 立即切换过去

### 3.5 添加改动到暂存区

```powershell
git add .
```

或只加部分文件：

```powershell
git add path/to/file
```

作用：

- 把本次准备提交的文件放进暂存区

### 3.6 提交

```powershell
git commit -m "fix: repair Ollama streaming in mcp demo"
```

作用：

- 生成一个新的 commit

建议：

- commit message 要短且明确
- 一次 commit 只做一类事情

### 3.7 推送

```powershell
git push
```

如果是第一次推送新分支：

```powershell
git push -u origin codex/my-change
```

作用：

- 把本地提交推到 GitHub

### 3.8 删除本地分支

```powershell
git branch -d codex/my-change
```

作用：

- 删除已经合并完成的本地分支

### 3.9 删除远端分支

```powershell
git push origin --delete codex/my-change
```

作用：

- 删除远端上已不需要的分支

## 4. 推荐的最小日常流程

适合单人维护这个仓库的最小流程如下：

1. 先更新主分支

```powershell
git switch main
git pull
```

2. 拉一个新分支

```powershell
git switch -c codex/some-work
```

3. 修改代码并自测

4. 查看状态

```powershell
git status
```

5. 提交

```powershell
git add .
git commit -m "feat: add xxx"
```

6. 推送分支

```powershell
git push -u origin codex/some-work
```

7. 合并回 `main`

如果你习惯走 GitHub 网页，可以在网页发起 Pull Request。

如果直接在本地维护，也可以合并后推送 `main`。

8. 合并完成后删除分支

```powershell
git branch -d codex/some-work
git push origin --delete codex/some-work
```

## 5. 发版最小流程

### 5.1 什么时候该发版

适合发版的场景：

- 一组功能已经稳定
- 一组修复已经完成
- 想给用户提供一个明确可回溯的版本点

### 5.2 发版前检查

最少检查以下几项：

1. 工作区干净

```powershell
git status
```

2. 当前在 `main`

```powershell
git branch --show-current
```

3. `main` 已推到远端

```powershell
git push origin main
```

4. 本地测试或构建已通过

### 5.3 创建 tag

```powershell
git tag -a v0.1.1 -m "Release v0.1.1"
git push origin v0.1.1
```

说明：

- `-a` 表示 annotated tag
- `v0.1.1` 是版本号
- 第二条命令把 tag 推送到 GitHub

### 5.4 创建 GitHub Release

如果已经安装并登录 `gh`，推荐直接用：

```powershell
gh release create v0.1.1 --title "v0.1.1" --notes "Release notes"
```

如果已经有 tag 和 release notes 文件：

```powershell
gh release create v0.1.1 --title "v0.1.1" --notes-file .\release-notes-v0.1.1.md
```

### 5.5 查看 Release

```powershell
gh release view v0.1.1
```

或查看 JSON：

```powershell
gh release view v0.1.1 --json name,tagName,isDraft,isPrerelease,publishedAt,url
```

## 6. 版本号怎么取

当前建议使用语义化版本的简化形式：

- `v0.1.0`：第一个对外可用版本
- `v0.1.1`：小修复版本
- `v0.2.0`：新增一组功能，但还没到 `1.0`
- `v1.0.0`：稳定正式版

一个简单判断标准：

- 只修 bug：补丁号加一，例如 `0.1.0 -> 0.1.1`
- 加功能但兼容旧用法：次版本号加一，例如 `0.1.1 -> 0.2.0`
- 有明显不兼容变化：主版本号加一，例如 `0.x -> 1.0.0` 或 `1.2.0 -> 2.0.0`

## 7. 常见问题

### 7.1 为什么 `git push` 能成功，但创建 release 失败

因为它们不是同一类认证：

- `git push` 走 Git 推送凭据
- `gh release create` 走 GitHub API 认证

如果没有安装或登录 `gh`，就可能出现：

- 能 push
- 但不能创建 Release

### 7.2 怎么检查 `gh` 是否可用

```powershell
gh --version
gh auth status
```

### 7.3 怎么重新登录 GitHub CLI

```powershell
gh auth login
```

推荐选项：

- `GitHub.com`
- `HTTPS`
- `Login with a web browser`

### 7.4 怎么查看当前远端地址

```powershell
git remote -v
```

### 7.5 怎么看历史提交

```powershell
git log --oneline --decorate -n 20
```

## 8. 面向这个仓库的建议

这个仓库当前适合采用下面这套最小规则：

1. 开发默认从 `main` 拉 `codex/...` 分支
2. 功能完成后合并回 `main`
3. 合并后尽快删除已完成分支，避免分支堆积
4. 重要功能集成完成后打 tag
5. 每个 tag 都创建对应 GitHub Release
6. release notes 至少写清楚：
   - 本版本新增了什么
   - 修复了什么
   - 是否有已知限制

## 9. 一组可直接复用的命令模板

### 新功能开发

```powershell
git switch main
git pull
git switch -c codex/new-feature
git add .
git commit -m "feat: add new feature"
git push -u origin codex/new-feature
```

### 合并后清理分支

```powershell
git switch main
git pull
git branch -d codex/new-feature
git push origin --delete codex/new-feature
```

### 发布新版本

```powershell
git switch main
git pull
git status
git tag -a v0.1.1 -m "Release v0.1.1"
git push origin v0.1.1
gh release create v0.1.1 --title "v0.1.1" --notes-file .\release-notes-v0.1.1.md
```

## 10. 当前已验证通过的命令

在这台机器上，下面这些能力已经验证可用：

- `git push origin main`
- `git push origin v0.1.0`
- `gh auth status`
- `gh release view v0.1.0`

如果后续需要自动化发布，优先基于 `gh` 继续扩展，不必再回退到手工网页发布。
