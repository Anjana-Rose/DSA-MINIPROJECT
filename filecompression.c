#include <stdio.h>
#include <stdlib.h>

#define Char_size 256

struct Node
{
    unsigned char character;
    long long int Freq;
    struct Node* left;
    struct Node* right;
};

void Mindownheap(struct Node** A, int i, int length)
{
    int least = i;
    if (2 * i + 1 <= length && A[2 * i + 1]->Freq < A[i]->Freq)
    {
        least = 2 * i + 1;
        if (2 * i + 2 <= length && A[2 * i + 2]->Freq < A[2 * i + 1]->Freq)
            least = 2 * i + 2;
    }
    else if (2 * i + 2 <= length && A[2 * i + 2]->Freq < A[i]->Freq)
        least = 2 * i + 2;
    if (least != i)
    {
        struct Node* temp = A[i];
        A[i] = A[least];
        A[least] = temp;
        Mindownheap(A, least, length);
    }
}

struct Node* Extract_min(struct Node** A, int* size)
{
    if (*size < 1)
        return NULL;
    struct Node* minimum = A[0];
    A[0] = A[*size - 1];
    (*size)--;
    Mindownheap(A, 0, *size - 1);
    return minimum;
}

void Insert_MinHeap(struct Node** A, struct Node* element, int* size)
{
    A[*size] = element;
    int i = *size;
    (*size)++;
    while (i > 0 && A[(i - 1) / 2]->Freq > A[i]->Freq)
    {
        struct Node* temp = A[i];
        A[i] = A[(i - 1) / 2];
        A[(i - 1) / 2] = temp;
        i = (i - 1) / 2;
    }
}

void Build_Minheap(struct Node** A, int length)
{
    for (int i = (length - 1) / 2; i >= 0; i--)
    {
        Mindownheap(A, i, length);
    }
}

void store_codes(struct Node* Root, char single_code[], int index, long long int Huffman_codemap[])
{
    if (Root->left)
    {
        single_code[index] = '0';
        store_codes(Root->left, single_code, index + 1, Huffman_codemap);
    }
    if (Root->right)
    {
        single_code[index] = '1';
        store_codes(Root->right, single_code, index + 1, Huffman_codemap);
    }
    if (!Root->left && !Root->right)
    {
        for (int i = index; i >= 0; i--)
        {
            if (i != index)
            {
                Huffman_codemap[Root->character] *= 10;
                Huffman_codemap[Root->character] += single_code[i] - '0';
            }
            else
                Huffman_codemap[Root->character] = 1;
        }
    }
}

void store_tree(FILE* input, struct Node* Root)
{
    if (!Root->left && !Root->right)
    {
        fprintf(input, "1%c", Root->character);
    }
    else
    {
        fprintf(input, "0");
        store_tree(input, Root->left);
        store_tree(input, Root->right);
    }
}

struct Node* Huffman(long long int Count[])
{
    struct Node** minheap = (struct Node*)malloc(Char_size * sizeof(struct Node));
    int minheap_size = 0;

    for (int i = 0; i < Char_size; i++)
    {
        if (Count[i] != 0)
        {
            minheap[minheap_size] = (struct Node*)malloc(sizeof(struct Node));
            minheap[minheap_size]->character = i;
            minheap[minheap_size]->Freq = Count[i];
            minheap[minheap_size]->left = NULL;
            minheap[minheap_size]->right = NULL;
            minheap_size++;
        }
    }

    Build_Minheap(minheap, minheap_size - 1);

    while (minheap_size != 1)
    {
        struct Node* Z = (struct Node*)malloc(sizeof(struct Node));
        Z->character = -1;
        Z->Freq = 0;
        Z->left = Extract_min(minheap, &minheap_size);
        Z->right = Extract_min(minheap, &minheap_size);
        Z->Freq = Z->left->Freq + Z->right->Freq;
        Insert_MinHeap(minheap, Z, &minheap_size);
    }

    struct Node* result = minheap[0];
    free(minheap);

    return result;
}

void Write_compressed(FILE* input, FILE* output, long long int Huffman_codemap[])
{
    char ch;
    unsigned char bits_8 = 0;
    long long int counter = 0;

    while (fread(&ch, sizeof(char), 1, input) == 1)
    {
        long long int temp = Huffman_codemap[(unsigned char)ch];
        while (temp != 1)
        {
            bits_8 <<= 1;
            if ((temp % 10) != 0)
                bits_8 |= 1;
            temp /= 10;
            counter++;
            if (counter == 8)
            {
                fwrite(&bits_8, sizeof(unsigned char), 1, output);
                counter = 0;
                bits_8 = 0;
            }
        }
    }

    while (counter != 8)
    {
        bits_8 <<= 1;
        counter++;
    }

    fwrite(&bits_8, sizeof(unsigned char), 1, output);
    fclose(output);
}

int main(int argc, char* argv[])
{
    long long int Huffman_codemap[Char_size] = {0};
    long long int Count[Char_size] = {0};
    char filename[256];
    FILE* input_file;
    FILE* output_file;

    printf("Enter Filename:\t");
    scanf("%s", filename);

    input_file = fopen(filename, "rb");
    if (input_file == NULL)
    {
        perror("Error:\t");
        exit(-1);
    }

    printf("\nCompressing the file.....");

    char ch;
    while (fread(&ch, sizeof(char), 1, input_file) == 1)
    {
        Count[(unsigned char)ch]++;
    }

    fseek(input_file, 0, SEEK_SET);

    struct Node* tree = Huffman(Count);

    char output_filename[256];
    sprintf(output_filename, "%s.huf", filename);
    output_file = fopen(output_filename, "wb");

    if (output_file == NULL)
    {
        perror("Error:\t");
        exit(-1);
    }

    fprintf(output_file, "%lld,", tree->Freq);
    store_tree(output_file, tree);
    fprintf(output_file, " ");

    char single_code[16];
    store_codes(tree, single_code, 0, Huffman_codemap);

    Write_compressed(input_file, output_file, Huffman_codemap);

    fclose(input_file);
    fclose(output_file);

    if (remove(filename) != 0)
        perror("Error deleting the compressed file:\t");

    printf("\n\nFile Compressed Successfully!");
    return 0;
}
