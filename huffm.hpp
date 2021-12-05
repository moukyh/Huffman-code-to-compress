/**
 * @file huffm.hpp
 * @author moukyh (moukyh@qq.com)
 * @brief
 * @version 0.1
 * @date 2021-12-05
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef _huffm_h
#define _huffm_h

#include <bits/stdc++.h>
#include <cstdlib>
#include <string>
#include <string.h>
#include <queue>
#include <map>
#include <fstream>

using namespace std;

#define MAX_SIZE 270
#define WRITE_BUFF_SIZE 10
#define PSEUDO_EOF 256

string uchar_to_binary(string &input, int &length, int &flag)
{
    string out_string = "";
    unsigned char temp_;
    for (int i = 0; i + 7 < length; i += 8, flag = i)
    {
        unsigned char out_ = 0;
        for (int j = 0; j < 8; j++)
        {
            if (input[i + j] == '0')
                temp_ = 0;
            else
                temp_ = 1;
            out_ += temp_ << (7 - j);
        }
        out_string += out_;
    }
    return out_string;
}

struct Huffman_node
{
    int id; // 使用int类型，因为要插入值为256的pseudo-EOF
    unsigned int freq;
    Huffman_node *left,
        *right;
    Huffman_node() : id(), freq(), left(NULL), right(NULL) {}
    Huffman_node(int id, unsigned int freq)
        : id(id), freq(freq), left(NULL), right(NULL) {}

    //是否是叶节点
    bool isleaf()
    {
        return left == NULL && right == NULL;
    }
};

typedef Huffman_node *Node_ptr;

class Huffman
{
private:
    Node_ptr node_array[MAX_SIZE]; // 叶子节点数组
    Node_ptr root;                 // 根节点
    int size;                      // 叶子节点数
    fstream in_file, out_file;     // 输入、输出文件流
    map<int, string> table;        // 字符->huffman编码映射表

    class Compare
    {
    public:
        bool operator()(const Node_ptr &c1, const Node_ptr &c2) const
        {
            return (*c1).freq > (*c2).freq;
        }
    };

    // 用于比较优先队列中元素间的顺序
    priority_queue<Node_ptr, vector<Node_ptr>, Compare> pq;

    // 根据输入文件构造包含字符及其频率的数组
    void create_node_array();

    // 根据构造好的Huffman树建立Huffman映射表
    void create_map_table(const Node_ptr node, string s);

    // 构造优先队列
    void create_pq();

    // 构造Huffman树
    void create_huffman_tree();

    // 计算Huffman编码
    void calculate_huffman_codes();

    // 开始压缩过程
    void do_compress();

    // 从huffman编码文件中重建huffman树
    void rebuild_huffman_tree();

    // 根据重建好的huffman树解码文件
    void decode_huffman();

public:
    // 根据输入和输出流初始化对象
    Huffman(string in_file_name, string out_file_name);

    // 析构函数
    ~Huffman();

    // 压缩文件
    void compress();

    // 解压文件
    void decompress();
};
void Huffman::create_node_array()
{
    int i, count;
    int freq[MAX_SIZE] = {0}; // 频数统计数组
    char in_char;

    // 依次读入字符，统计数据
    while (!in_file.eof())
    {
        in_file.get(in_char);
        // 消除最后一行的影响
        if (in_file.eof())
            break;
        // char是有符号的，数组下标是unsigned 所以要换成unsigned char
        freq[(unsigned char)in_char]++;
    }

    count = 0;
    for (i = 0; i < MAX_SIZE; ++i)
    {
        if (freq[i] <= 0)
            continue;
        Node_ptr node = new Huffman_node(i, freq[i]);
        // cout << "node->id = " << node->id << "  node->freq = " << node->freq << endl;
        node_array[count++] = node;
    }
    // 插入频率为1的pseudo-EOF

    Node_ptr node = new Huffman_node(PSEUDO_EOF, 1);
    node_array[count++] = node;

    size = count;
}

//创建优先队列pq，队列中以fre为序，首先小顶堆排序
void Huffman::create_pq()
{
    int i;
    create_node_array();
    for (i = 0; i < size; ++i)
        pq.push(node_array[i]);
}

//根据pq构建Huffman tree
void Huffman::create_huffman_tree()
{
    root = NULL;
    while (!pq.empty())
    {
        Node_ptr first = pq.top();
        // cout << "node->id = " << first->id << "  node->freq = " << first->freq << endl;
        pq.pop();
        if (pq.empty())
        {
            root = first;
            break;
        }
        Node_ptr second = pq.top();
        pq.pop();
        Node_ptr parent = new Huffman_node();
        parent->freq = first->freq + second->freq;
        parent->left = first;
        parent->right = second;
        pq.push(parent);
    }
}

void Huffman::create_map_table(const Node_ptr node, string s)
{
    if (node->isleaf())
    {
        table[node->id] = s;
        // cout << "table[" << node->id << "] = " << s << endl;
        return;
    }
    create_map_table(node->left, s + "0");
    create_map_table(node->right, s + "1");
}

//根据huffman tree计算其huffman编码
void Huffman::calculate_huffman_codes()
{
    if (root == NULL)
    {
        printf("Build the huffman tree failed or no characters are counted\n");
        exit(1);
    }
    create_map_table(root, "");
}

void Huffman::do_compress()
{
    int length;
    char in_char;
    string code, out_string;
    map<int, string>::iterator table_it;

    // 写入查找树
    string temp = "";
    // write_tree(*root, temp);
    // cout << "output = " << temp << endl;
    // 按节点数(包括pseudo-EOF) + 哈夫曼树 + 哈夫曼编码来写入文件

    // 第1行写入节点数（int）
    out_file << size << endl;

    // 第2~(size+1)行写入huffman树，即每行写入字符+huffman编码，如"43 00100"
    for (table_it = table.begin(); table_it != table.end(); ++table_it)
    {
        out_file << table_it->first << " " << table_it->second << endl;
    }

    // 第size+2行写入huffman编码
    in_file.clear();
    in_file.seekg(ios::beg);
    code.clear();
    while (!in_file.eof())
    {
        in_file.get(in_char);
        // 消除最后一行回车的影响
        if (in_file.eof())
            break;
        // 找到每一个字符所对应的huffman编码
        table_it = table.find((unsigned char)in_char);
        if (table_it != table.end())
            code += table_it->second;
        else
        {
            printf("Can't find the huffman code of character %X\n", in_char);
            exit(1);
        }
        // 当总编码的长度大于预设的WRITE_BUFF_SIZE时再写入文件
        length = code.length();
        // cout << code << endl;
        if (length > WRITE_BUFF_SIZE)
        {
            out_string.clear();
            int flag = 0;
            out_string = uchar_to_binary(code, length, flag);
            out_file << out_string;
            code = code.substr(flag, length - flag);
        }
    }

    // 已读完所有文件，先插入pseudo-EOF
    table_it = table.find(PSEUDO_EOF);
    if (table_it != table.end())
        code += table_it->second;
    else
    {
        printf("Can't find the huffman code of pseudo-EOF\n");
        exit(1);
    }
    // 再处理尾部剩余的huffman编码
    length = code.length();
    unsigned char out_c = 0;
    for (int i = 0; i < length; i++)
    {
        unsigned char tmp_c = 0;
        if ('0' == code[i])
            tmp_c = 0;
        else
            tmp_c = 1;
        out_c += tmp_c << (7 - (i % 8));
        if (0 == (i + 1) % 8 || i == length - 1)
        {
            // 每8位写入一次文件
            out_file << out_c;
            out_c = 0;
        }
    }
}

void Huffman::rebuild_huffman_tree()
{
    int i, j, id, length;
    string code;
    Node_ptr node, tmp, new_node;

    //读huffman tree
    root = new Huffman_node();
    root->left = NULL;
    root->right = NULL;

    in_file >> size;
    if (size > MAX_SIZE)
    {
        printf("The number of nodes is not valid, maybe the compressed file has been broken.\n");
        exit(1);
    }

    for (i = 0; i < size; ++i)
    {
        in_file >> id;
        in_file >> code;
        length = code.length();
        node = root;
        for (j = 0; j < length; ++j)
        {
            if (code[j] == '0')
                tmp = node->left;
            else if (code[j] == '1')
                tmp = node->right;
            else
            {
                printf("Decode error, huffman code is not made up with 0 or 1\n");
                exit(1);
            }

            // 如果到了空，则新建一个节点
            if (tmp == NULL)
            {
                new_node = new Huffman_node();
                new_node->left = NULL;
                new_node->right = NULL;
                // new_node->parent = node;

                // 如果是最后一个0或1,说明到了叶子节点，给叶子节点赋相关的值
                if (j == length - 1)
                {
                    new_node->id = id;
                    table[new_node->id] = code;
                    // new_node->code = code;
                }

                if ('0' == code[j])
                    node->left = new_node;
                else
                    node->right = new_node;

                tmp = new_node;
            }
            // 如果不为空，且到了该huffman编码的最后一位，这里却已经存在了一个节点，就说明
            // 原来的huffmaninman是有问题的
            else if (j == length - 1)
            {
                printf("Huffman code is not valid, maybe the compressed file has been broken.\n");
                exit(1);
            }
            // 如果不为空，但该节点却已经是叶子节点，说明寻路到了其他字符的编码处，huffman编码也不对
            else if (tmp->left == NULL && tmp->right == NULL)
            {
                printf("Huffman code is not valid, maybe the compressed file has been broken.\n");
                exit(1);
            }
            node = tmp;
        }
    }
}

void Huffman::decode_huffman()
{
    bool pseudo_eof;
    int i, id;
    char in_char;
    string out_string;
    unsigned char u_char, flag;
    Node_ptr node;

    out_string.clear();
    node = root;
    pseudo_eof = false;
    in_file.get(in_char); // 跳过最后一个回车
    while (!in_file.eof())
    {
        in_file.get(in_char);
        u_char = (unsigned char)in_char;
        flag = 0x80;
        for (i = 0; i < 8; ++i)
        {

            if (u_char & flag)
                node = node->right;
            else
                node = node->left;

            if (node->left == NULL && node->right == NULL)
            {
                id = node->id;
                if (id == PSEUDO_EOF)
                {
                    pseudo_eof = true;
                    break;
                }
                else
                {
                    // int to char是安全的，高位会被截断
                    out_string += (char)node->id;
                    node = root;
                }
            }
            flag = flag >> 1;
        }
        if (pseudo_eof)
            break;

        if (WRITE_BUFF_SIZE < out_string.length())
        {
            out_file << out_string;
            out_string.clear();
        }
    }

    if (!out_string.empty())
        out_file << out_string;
}

Huffman::Huffman(string in_file_name, string out_file_name)
{
    in_file.open((".\\" + in_file_name).c_str(), ios::in | ios::binary);
    if (!in_file)
    {
        printf("Open file error, path is: %s\n", in_file_name.c_str());
        exit(1);
    }

    out_file.open((".\\" + out_file_name).c_str(), ios::out | ios::binary);
    if (!out_file)
    {
        printf("Open file error, path is: %s\n", out_file_name.c_str());
        exit(1);
    }
}

Huffman::~Huffman()
{
    in_file.close();
    out_file.close();
}

void Huffman::compress()
{
    create_pq();
    create_huffman_tree();
    calculate_huffman_codes();
    do_compress();
}

void Huffman::decompress()
{
    rebuild_huffman_tree();
    decode_huffman();
}

#endif