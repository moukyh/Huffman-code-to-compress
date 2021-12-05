#include <bits/stdc++.h>
#include "huffm.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Usage:\n\t huffmancoding inputfile outputfile\n");
        exit(1);
    }

    if (0 == strcmp("-c", argv[1]))
    {
        Huffman h(argv[2], argv[3]);
        h.compress();
    }
    else if (0 == strcmp("-d", argv[1]))
    {
        Huffman h(argv[2], argv[3]);
        h.decompress();
    }
    else
    {
        printf("Usage:\n\t unkonwn command\n");
    }
    return 0;
}