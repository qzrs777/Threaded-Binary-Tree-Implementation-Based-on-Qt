# Threaded Binary Tree Visualizer

一个使用 C++17 与 Qt Widgets 实现的中序线索二叉树可视化程序。项目将数据结构算法与图形界面彻底分离：核心模型负责节点所有权、增删操作、线索重建和一致性校验，Qt 层只负责交互与渲染。

## 功能

- 按稳定节点 ID 选中节点，重复值不会造成误选。
- 在指定节点的空闲左/右孩子位置插入整数；已有孩子时明确拒绝操作，避免覆盖子树。
- 正确删除叶节点、单孩子节点、双孩子节点以及根节点。
- 每次结构变更后自动清除并重建中序前驱、后继线索。
- 同时展示真实孩子边与有向线索边，并实时显示中序线索遍历序列。
- 使用中序排名和结构深度布局节点，避免原先固定递减偏移造成的深层节点重叠。
- 提供不依赖 Qt 的核心测试，覆盖重复值和主要删除边界情况。

## 设计

`ThreadedBinaryTree` 是唯一的数据结构真源，以 `std::unique_ptr` 集中管理节点生命周期，并通过 `NodeId` 对外标识节点。`TreeNodeItem` 不再保存左右指针或标志位，`MainWindow` 根据模型快照重绘场景。因此，删除图元不会影响树结构，树算法也不依赖 Qt。

模型仍采用经典的 `left/right + ltag/rtag` 表示：

- `LinkTag::Child` 表示指针指向真实孩子；
- `LinkTag::Thread` 表示指针指向中序前驱或后继；
- 界面中的灰色实线表示孩子关系，红色虚线箭头表示线索方向。

删除双孩子节点时，模型将其中序前驱移植到目标位置，并实际移除用户选中的节点 ID。完成结构变更后统一重建线索，比在每个删除分支中手工修补前驱、后继更容易验证。

## 构建与运行

依赖 CMake 3.16+、支持 C++17 的编译器，以及 Qt 5 或 Qt 6 Widgets 开发包。

```bash
cmake -S . -B build -DBUILD_GUI=ON -DBUILD_TESTING=ON
cmake --build build --config Release
```

运行生成的 `untitled1` 可执行文件。若本机未安装 Qt，可只构建并验证核心算法：

```bash
cmake -S . -B build-core -DBUILD_GUI=OFF -DBUILD_TESTING=ON
cmake --build build-core --config Release
ctest --test-dir build-core -C Release --output-on-failure
```

## 操作说明

1. 单击图中的节点进行选择。
2. 输入整数并选择“左孩子”或“右孩子”，点击“添加节点”。
3. 点击“删除选中节点”可删除当前节点。
4. 树为空时，下一次添加操作会创建新的根节点。

`first.cpp` 是早期的控制台算法草稿，仅保留作历史参考，不属于当前构建目标。
