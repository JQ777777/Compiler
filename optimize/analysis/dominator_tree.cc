#include "dominator_tree.h"
#include "../../include/ir.h"
#include <algorithm>
#include "../../include/cfg.h"
#include <bitset>

void DomAnalysis::Execute() {
    for (auto [defI, cfg] : llvmIR->llvm_cfg) {
        DomInfo[cfg].C = cfg;
        DomInfo[cfg].BuildDominatorTree();

        //反向支配树
        DomInfoReverse[cfg].C = cfg;
        DomInfoReverse[cfg].BuildDominatorTree(true);
    }
}

    // 执行 DFS 来构建 DFS 树
    void DFS(CFG * C,int u,std::vector<std::vector<LLVMBlock>>& G,std::vector<int>& S, std::vector<int>& visited) 
    {
        visited[u] = 1;
        for (auto next : G[u])
        {
            // 确保 next->block_id 是一个有效的键
            if (C->block_map->find(next->block_id) == C->block_map->end()) {
                std::cerr << "Warning: Invalid block ID in CFG: " << next->block_id << std::endl;
                continue;  // 跳过无效的 blockid
            }
            if (visited[next->block_id] == 0)
            {
                DFS(C,next->block_id,G,S,visited);
            }
        }
        S.push_back(u);
        //std::cout << "Added block " << u << " to post_order" << std::endl;
    }

int DominatorTree::CommonDominator(int a, int b, std::vector<std::vector<LLVMBlock>>& invG) {
    std::queue<int> q_a, q_b;
    std::unordered_set<int> visited_a, visited_b;

    // 打印初始节点
    //std::cout << "Finding common dominator for a: " << a << " and b: " << b << std::endl;

    // 将 a 的所有祖先加入队列
    q_a.push(a);
    visited_a.insert(a);
    //std::cout << "Traversing path from a to root:" << std::endl;
    while (!q_a.empty()) {
        int current = q_a.front();
        q_a.pop();
        //std::cout << "Visited node a: " << current << std::endl;

        // 如果当前节点没有前驱，假设它是根节点
        if (current >= invG.size() || invG[current].empty()) {
            //std::cout << "Reached root from a." << std::endl;
            continue;
        }

        // 遍历当前节点的所有前驱
        for (LLVMBlock pred : invG[current]) {
            int pred_id = pred->block_id;
            if (C->block_map->find(pred->block_id) == C->block_map->end()){
                continue;
            }
            if (visited_a.find(pred_id) == visited_a.end()) {
                visited_a.insert(pred_id);
                q_a.push(pred_id);
                //std::cout << "  Adding ancestor of a: " << pred_id << std::endl;
            }
        }
    }

    // 将 b 的所有祖先加入队列
    q_b.push(b);
    visited_b.insert(b);
    //std::cout << "Traversing path from b to root:" << std::endl;
    while (!q_b.empty()) {
        int current = q_b.front();
        q_b.pop();
        //std::cout << "Visited node b: " << current << std::endl;

        // 如果当前节点没有前驱，假设它是根节点
        if (current >= invG.size() || invG[current].empty()) {
            //std::cout << "Reached root from b." <<std::endl;
            break;
        }

        // 遍历当前节点的所有前驱
        for (LLVMBlock pred : invG[current]) {
            int pred_id = pred->block_id;
            if (C->block_map->find(pred->block_id) == C->block_map->end()){
                continue;
            }
            if (visited_b.find(pred_id) == visited_b.end()) {
                visited_b.insert(pred_id);
                q_b.push(pred_id);
                //std::cout << "  Adding ancestor of b: " << pred_id << std::endl;
            }
        }
    }

    // 找到最近公共祖先
    //std::cout << "Searching for common ancestor..." << std::endl;
    for (int i : visited_a) {
        if (visited_b.find(i) != visited_b.end()) {
            //std::cout << "Found common ancestor: " << i << std::endl;
            return i;
        }
    }

    // 检查根节点 0 是否是共同祖先
    if (visited_a.find(0) != visited_a.end() && visited_b.find(0) != visited_b.end()) {
        //std::cout << "Root node 0 is the common ancestor." << std::endl;
        return 0;
    }

    // 如果没有找到共同祖先，返回 -1
    //std::cout << "No common ancestor found. Returning -1." << std::endl;
    return -1;
}

// 递归函数：将节点 u 的所有支配者（包括它自身）加入到 u 的支配集中
void updateDomSet(int u, const std::map<int, int>& dom, std::map<int, std::unordered_set<int>>& domset) {
    // 确保 u 本身在它的支配集中
    domset[u].insert(u);

    if (dom.find(u) == dom.end()) {
        std::cerr << "Error: Node " << u << " has no immediate dominator." << std::endl;
        return;
    }

    int idom = dom.at(u);  // 获取 u 的立即支配者

    // 如果 u 是根节点，停止递归
    if (idom == u) {
        return;
    }

    // 递归更新 idom 的支配集
    updateDomSet(idom, dom, domset);

    // 将 idom 的支配集与 u 的支配集取并集
    for (int v : domset[idom]) {
        domset[u].insert(v);
    }
}

// 辅助函数：从根节点开始遍历并更新所有节点的支配集
void updateAllDomSets(const std::map<int, int>& dom, std::map<int, std::unordered_set<int>>& domset, int startid) {
    // 从根节点开始递归更新支配集
    updateDomSet(startid, dom, domset);

    // 使用一个队列或栈来确保所有可达节点都被处理
    // 这里我们使用栈实现深度优先搜索（DFS）
    std::stack<int> stack;
    stack.push(startid);

    while (!stack.empty()) {
        int u = stack.top();
        stack.pop();

        // 遍历所有被 u 支配的节点（即 u 的子节点）
        for (const auto& [v, idom] : dom) {
            if (idom == u && v != startid) {  // 跳过根节点
                updateDomSet(v, dom, domset);
                stack.push(v);
            }
        }
    }
}


void DominatorTree::BuildDominatorTree(bool reverse) {
    // 确保 C 不为空
    if (!C) {
        throw std::runtime_error("CFG pointer is null in DominatorTree");
    }

    // 检查 block_map 是否为空
    if (C->block_map->empty()) {
        throw std::runtime_error("block_map is empty or not initialized");
    }

    // 打印 block_map 的内容，确保它包含有效的基本块
    std::cout << "Block map size: " << C->block_map->size() << std::endl;
    // for (const auto& [blockid, block] : *C->block_map) {
    //     std::cout << "Block " << blockid << ": " << block->name << std::endl;
    // }

    // 确保 C->max_label 的值正确
    int max_block_id = 0;
    for (const auto& [blockid, block] : *C->block_map) {
        if (blockid > max_block_id) {
            max_block_id = blockid;
        }
    }
    std::cout << "C->max_label = " << C->max_label << ", Max block ID in block_map = " << max_block_id << std::endl;

    if (C->max_label < max_block_id) {
        throw std::runtime_error("C->max_label is smaller than the maximum block ID in block_map");
    }

    int startid = 0;  // 设置开始基本块

    if (reverse) {  // 如果构建后支配树
        //std::swap(G, invG);  // 交换 G 和 invG
        if (C->retBlocks.size() > 1) {
            BasicBlock* mergeBlock = C->CreateReturnMergeBlock(C->retBlocks);

            // 更新控制流图，将所有 RET 语句所在的块指向新的合并块
            for (auto* block : C->retBlocks) {
                C->G[block->block_id].push_back(mergeBlock);
                C->invG[mergeBlock->block_id].push_back(block);
            }

            // 将新的合并块作为返回块
            C->return_block = mergeBlock;
        }
    }

    auto *G = &(C->G);  // 正向控制流图边关系
    auto *invG = &(C->invG);  // 反向控制流图边关系
    if (reverse) {
        auto* temp = G;
        G = invG;
        invG = temp;
        startid = C->return_block->block_id;
        
        std::cout << "startid:" << startid << std::endl;
    }

    // 初始化所有基本块的直接支配者为自身
    dom.clear();
    for (auto [blockid, block] : *C->block_map) {
        dom[blockid] = blockid;  // 每个块的初始直接支配者是它自己
    }
    dom[startid] = startid;  // 入口块的直接支配者为自身

    // 创建一个后序遍历的基本块列表
    std::vector<int> post_order;
    std::vector<int> visited(C->max_label + 2, false);

    // 确保从所有入口点开始进行 DFS
    DFS(C, startid, *G, post_order, visited);

    // 检查 post_order 列表中是否有重复的 blockid
    std::set<int> unique_blocks;
    for (int b : post_order) {
        if (unique_blocks.find(b) != unique_blocks.end()) {
            throw std::runtime_error("Duplicate block ID in post_order: " + std::to_string(b));
        }
        unique_blocks.insert(b);
    }

    // 初始化缓存
    std::map<std::pair<int, int>, int> cache;

    // 使用 std::vector 替代动态分配的二维数组
    std::vector<std::vector<int>> Dom(C->max_label + 2, std::vector<int>(C->max_label + 2, 0));

    for (int i = 0 ; i < C->max_label+2; i++){
        if (i == startid)
        {
            for (int j = 0 ; j < C->max_label+2; j++){
                if (j == startid){
                    //初始化根节点支配关系
                    Dom[startid][startid] = 1;
                    continue;
                }
                Dom[i][j] = 0;
            }
        }
        else{
            for (int j = 0 ; j < C->max_label+2; j++){
                Dom[i][j] = 1;
            }
        }
    }
    
    
    bool flag = true;
    int iteration = 0;  // 记录迭代次数

    while (flag) {
        iteration++;
        //std::cout << "Starting iteration " << iteration << " with flag = " << flag << "\n";
        flag = false;

        for (auto it = post_order.rbegin(); it != post_order.rend(); ++it) {
            int b = *it;
            if (b == startid) continue;  // 跳过入口块

            // 创建一个临时的支配关系矩阵
            std::vector<int> temp(C->max_label + 2, 0);
            //temp[b] = 1; // 自己是自己的支配者

            // 如果有前驱节点，则进行支配关系的交集运算
            if (!(*invG)[b].empty()) {
                // 初始化为第一个前驱节点的支配关系
                for (int i = 0; i <= C->max_label + 1; ++i) {
                    temp[i] = Dom[(*invG)[b][0]->block_id][i];
                    //std::cout<<"temp[i]: "<<temp[i]<< std::endl;
                    //std::cout<<"Dom[(*invG)[b][0]->block_id][i]: "<<Dom[(*invG)[b][0]->block_id][i]<< std::endl;
                }

                // 对其他前驱节点进行交集运算
                for (size_t k = 1; k < (*invG)[b].size(); ++k) {
                    for (int i = 0; i <= C->max_label + 1; ++i) {
                        temp[i] &= Dom[(*invG)[b][k]->block_id][i];
                    }
                }
            }

            temp[b] = 1; // 自己是自己的支配者

            // 检查是否有新的支配关系被发现
            bool updated = false;
            for (int i = 0; i <= C->max_label + 1; ++i) {
                if (temp[i] != Dom[b][i]) {
                    //std::cout<<"temp[i]: "<<temp[i]<< std::endl;
                    //std::cout<<"Dom[b][i]: "<<Dom[b][i]<< std::endl;
                    Dom[b][i] = temp[i];
                    updated = true;
                }
            }

            if (updated) {

                flag = true;
                //std::cout << "Flag set to true due to update in node " << b << "\n";
            } else {
                //std::cout << "No update for node " << b << "\n";
            }
        }
    }

    // 根据最终支配关系确定支配集
    for (int i = 0; i <= C->max_label + 1; ++i) {
        if (C->block_map->find(i) == C->block_map->end()) {
            continue;
        }

        //std::cout << "Building domset for node " << i << ":\n";

        for (int j = 0; j <= C->max_label + 1; ++j) {
            if (C->block_map->find(j) == C->block_map->end()) {
                continue;
            }

            if (i == j || Dom[i][j]) {
                domset[i].insert(j);
                //std::cout << "  Node " << i << " is dominated by node " << j << "\n";
            }
        }

    }

    dom_tree.clear();
    dom_tree.resize(C->max_label + 2);
    idom.clear();  // 初始化 idom 数组
    idom.resize(C->max_label + 2, nullptr);

    //建立支配树和立即支配者
    for (int i = 0 ; i < C->max_label+2 ; i++){
        //std::cout << "  Node " << i << "\n";
        if (C->block_map->find(i) == C->block_map->end()) {
            continue;
        }
        if (i == startid){
            dom[startid] = startid;
            idom[startid] = (*C->block_map)[startid];
            dom_tree[startid].push_back((*C->block_map)[startid]);
            continue;
        }
        // 初始化为-1，表示尚未找到直接支配节点
        dom[i] = -1;
        for (int j = 0 ; j < C->max_label+2; j++){
            if (C->block_map->find(j) == C->block_map->end()) {
                continue;
            }
            if(i == j){
                continue;
            }
            if(Dom[i][j]== 1){
                //std::cout << "dominated by node " << j << "\n";
                // 创建一个临时的支配关系矩阵
                int temp[C->max_label+2];
                std::fill(temp, temp + (C->max_label+2), 0);

                for (int k = 0 ; k < C->max_label+2; k++){
                    if (C->block_map->find(k) == C->block_map->end()) {
                        continue;
                    }
                    temp[k] = (Dom[j][k] && Dom[i][k]) ^ Dom[i][k];
                    //std::cout << "k: " << k << " temp[k]: " << temp[k] << "\n";
                }

                // 检查temp中是否只有一个1，并且这个1是u本身
                int count = 0;
                bool is_u = false;
                for (int k = 0; k < C->max_label+2; ++k) {
                    if (C->block_map->find(k) == C->block_map->end()) {
                        continue;
                    }
                    if (temp[k] == 1) {
                        //std::cout << "k: " << k << "\n";
                        count++;
                        if (k == i) {
                            is_u = true;
                        }
                        else{
                            //std::cout << "k != i " << "\n";
                            is_u = false;
                            break;
                        }
                    }
                }
                if (count == 1 && is_u) {
                    //std::cout << "i: " << i << "is dominator by "<< j <<"\n";
                    dom[i] = j;
                    idom[i] = (*C->block_map)[j];
                    dom_tree[j].push_back((*C->block_map)[i]);
                    break; // 找到直接支配节点后退出循环
                }
            }
        }
        // 如果没有找到直接支配节点，说明i是根节点的直接支配节点
        if (dom[i] == -1) {
            dom[i] = startid;
            idom[i] = (*C->block_map)[startid];
            dom_tree[startid].push_back((*C->block_map)[i]);
        }
    }

    // 初始化支配边界数据结构
    df.clear();
    //std::cout << "Initializing Dominance Frontier data structure." << std::endl;

    for (auto [blockid, block] : *C->block_map) {
        for (auto succ : (*G)[blockid]) {
            if (C->block_map->find(succ->block_id) == C->block_map->end()){
                continue;
            }
            int a = blockid;
            int b = succ->block_id;

            // 打印当前处理的边
            //std::cout << "Processing edge: " << a << " -> " << b << std::endl;

            // 避免 x == b 的情况，直接跳过
            if (a == b) {
                //std::cout << "  Skipping edge because a == b" << std::endl;
                continue;
            }

            // 检查 a 是否支配 b
            bool dominates = IsDominate(a, b);
            //std::cout << "  Is " << a << " dominating " << b << "? " << (dominates ? "Yes" : "No") << std::endl;

            // 如果 a 不支配 b，将 b 添加到 a 的支配边界中
            if (!dominates) {
                df[a].insert(b);
                //std::cout << "  Adding " << b << " to DF[" << a << "] = ";
                
                //std::cout << std::endl;
            }

            // 沿着支配树向上遍历，直到找到一个不支配 b 的节点 x
            int x = (idom[a] != NULL) ? idom[a]->block_id : -1;  // 从 a 的直接支配者开始
            //std::cout << "  Starting upward traversal from x = " << x << std::endl;

            while (x != -1 && !IsDominate(x, b)) {  // -1 表示根节点
                df[x].insert(b);
                //std::cout << "  Adding " << b << " to DF[" << x << "] = ";
                
                //std::cout << std::endl;

                // 更新 x 为它的直接支配者
                if (idom[x] != NULL) {
                    x = idom[x]->block_id;
                    //std::cout << "  Updated x to " << x << std::endl;
                } else {
                    // 如果 x 没有直接支配者，退出循环
                    //std::cout << "  No more dominators, breaking loop" << std::endl;
                    break;
                }
            }

            if (x != -1 && IsDominate(x, b)) {
                //std::cout << "  x = " << x << " dominates " << b << ", stopping traversal" << std::endl;
            }
        }
    }

    // 打印最终的支配边界
    //std::cout << "Final Dominance Frontier:" << std::endl;
    for (auto [blockid, frontier] : df) {
        //std::cout << "  Block " << blockid << " has dominance frontier: ";
        for (int df_block : frontier) {
            std::cout << df_block << " ";
        }
        //std::cout << std::endl;
    }
}

std::set<int> DominatorTree::GetDF(std::set<int> S) { 
    //TODO("GetDF"); 
    std::set<int> result;

    // 对于每个给定的基本块，将它的支配边界加入结果集
    for (int blockid : S) {
        if (df.find(blockid) != df.end()) { // 确保该基本块存在对应的支配边界
            result.insert(df[blockid].begin(), df[blockid].end());
        }
    }

    return result;
}

std::set<int> DominatorTree::GetDF(int id) { 
    //TODO("GetDF"); 
    // 检查给定的id是否存在于支配边界数据结构中
    std::set<int> result;
    if (df.find(id) != df.end()) {
        result.insert(df[id].begin(), df[id].end());
        return result; // 返回对应的支配边界集合
    } else {
        // 如果不存在，则返回一个空的集合
        return std::set<int>();
    }
}


bool DominatorTree::IsDominate(int id1, int id2) { 
    //TODO("IsDominate"); 

    // 查找id2的支配集
    auto it = domset.find(id2);
    if (it == domset.end()) {
        // 如果id2没有支配集，返回false
        return false;
    }

    // 获取id2的支配者集合
    const std::unordered_set<int>& dominators = it->second;

    // 检查id1是否在id2的支配集中
    return dominators.find(id1) != dominators.end();
}
